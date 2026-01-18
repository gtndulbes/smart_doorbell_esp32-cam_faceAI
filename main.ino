#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
// ================== CAMERA MODEL ==================
//#define CAMERA_MODEL_AI_THINKER
#define CAMERA_MODEL_WROVER_KIT
#include "camera_pins.h"

// ================== WIFI ==================
const char* ssid = "USERNAME_WIFI";
const char* password = "PASSWORD_WIFI";

// ================== TELEGRAM ==================
const char* botToken = "BOT_TOKEN_TELEGRAM";
const char* chatID  = "CHAT_ID_TELEGRAM";

// ================== PIN ==================
#define PIR_PIN     14
#define RELAY_PIN   13
#define BUZZER_PIN  12
#define FLASH_PIN   4
// ================== FLAG STATUS ==================
volatile bool pirDetected = false;
volatile bool faceDetected = false;
volatile bool faceRecognized = false;
volatile bool actuatorBusy = false;
volatile bool is_enrolling = false;

WiFiClientSecure telegramClient;
UniversalTelegramBot bot(botToken, telegramClient);
unsigned long pirTimer = 0;
unsigned long lastTelegramCheck = 0;
const unsigned long telegramInterval = 3000; // cek tiap 3 detik (HEMAT)
// ================== TELEGRAM CONTROL ==================
bool sendPhotoCmd = false;
bool relayState = LOW;
// ================== FUNCTION ==================
void startCameraServer();
void sendTelegramMessage(String text);
void sendTelegramPhoto(camera_fb_t * fb);


// ================== SETUP ==================
void setup() {
  
  telegramClient.setInsecure();
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FLASH_PIN, OUTPUT);
  
  digitalWrite(FLASH_PIN, LOW); // Flash mati awal
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);

  // ===== CAMERA CONFIG =====
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera Init Failed");
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  startCameraServer();
  
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}
void sendTelegramMessage(String text) {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  String url = "https://api.telegram.org/bot" + String(botToken) +
               "/sendMessage?chat_id=" + String(chatID) +
               "&text=" + text;

  if (https.begin(client, url)) {
    https.GET();
    https.end();
  }
}

void sendTelegramPhoto(camera_fb_t * fb) {
  if (!fb || WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();

  String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";

  String head =
    "--" + boundary + "\r\n"
    "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n" +
    String(chatID) + "\r\n" +
    "--" + boundary + "\r\n"
    "Content-Disposition: form-data; name=\"photo\"; filename=\"face.jpg\"\r\n"
    "Content-Type: image/jpeg\r\n\r\n";

  String tail = "\r\n--" + boundary + "--\r\n";

  if (!client.connect("api.telegram.org", 443)) return;

  client.println("POST /bot" + String(botToken) + "/sendPhoto HTTP/1.1");
  client.println("Host: api.telegram.org");
  client.println("Content-Type: multipart/form-data; boundary=" + boundary);
  client.println("Content-Length: " + String(head.length() + fb->len + tail.length()));
  client.println();

  client.print(head);
  client.write(fb->buf, fb->len);
  client.print(tail);
}

void handleNewMessages(int numNewMessages) {

  for (int i = 0; i < numNewMessages; i++) {

    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != chatID) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String text = bot.messages[i].text;

    // ===== /start =====
    if (text == "/start") {
      String msg  = "📷 ESP32-CAM Smart Door Bell\n\n";
      msg += "/photo  - Ambil foto\n";
      msg += "/lock   - Kunci pintu\n";
      msg += "/unlock - Buka pintu\n";
      msg += "/status - Status sistem\n";
      msg += "/enroll - tambah pengguna\n";
      bot.sendMessage(chatID, msg, "");
    }

    // ===== /photo =====
    else if (text == "/photo") {
      sendPhotoCmd = true;   // hanya set FLAG
      bot.sendMessage(chatID, "📸 Mengambil foto...", "");
    }

    // ===== /lock =====
    else if (text == "/lock") {
      relayState = HIGH;
      digitalWrite(RELAY_PIN, relayState);
      bot.sendMessage(chatID, "🔒 Pintu dikunci", "");
    }

    // ===== /unlock =====
    else if (text == "/unlock") {
      relayState = LOW;
      digitalWrite(RELAY_PIN, relayState);
      bot.sendMessage(chatID, "🔓 Pintu dibuka", "");
    }

    // ===== /status =====
    else if (text == "/status") {
      String statusMsg = "📊 Status Sistem:\n";
      statusMsg += "PIR        : " + String(pirDetected ? "DETEKSI" : "AMAN") + "\n";
      statusMsg += "Face Detect: " + String(faceDetected ? "YA" : "TIDAK") + "\n";
      statusMsg += "Relay      : " + String(relayState ? "ON" : "OFF");
      bot.sendMessage(chatID, statusMsg, "");
    }
    else if (text == "/enroll") {
    if (!is_enrolling) {
        is_enrolling = true;      // <-- DI SINI
        bot.sendMessage(chat_id, "📸 Enroll dimulai, hadapkan wajah ke kamera", "");
      }
    }
  }
}


// ================== LOOP ==================
void loop() {
// ===== TELEGRAM POLLING (HEMAT) =====
if (millis() - lastTelegramCheck > telegramInterval) {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  if (numNewMessages) {
    handleNewMessages(numNewMessages);
  }
  lastTelegramCheck = millis();
}
// ===== FOTO DARI TELEGRAM =====
if (sendPhotoCmd) {
  sendPhotoCmd = false;
    digitalWrite(FLASH_PIN, HIGH);   // 🔦 FLASH ON
    delay(150);
  camera_fb_t * fb = esp_camera_fb_get();
  if (fb) {
    sendTelegramPhoto(fb);
    esp_camera_fb_return(fb);
  }
  digitalWrite(FLASH_PIN, LOW);    // 🔦 FLASH OFF
}

  // ===== 1. PIR DETECTION =====
  // Check PIR sensor for motion with cooldown
  int pirState = digitalRead(PIR_PIN);
    if (pirState == HIGH && !pirDetected) {
      pirDetected = true;
      pirTimer = millis();              // catat waktu deteksi
      sendTelegramMessage("🚨 Gerakan terdeteksi!");
    }

  // PIR SUDAH TENANG 10 DETIK
    if (pirState == LOW && pirDetected && millis() - pirTimer > 10000) {
      pirDetected = false;
    }

// ===== 2. FACE DETECTED =====
if (faceDetected) {
  delay(500);        // delay 5 detik antar foto
  faceDetected = false;

  digitalWrite(BUZZER_PIN, HIGH);
  delay(3000);
  digitalWrite(BUZZER_PIN, LOW);
  // FLASH & FOTO
  digitalWrite(FLASH_PIN, HIGH);
  delay(120);   // stabilkan cahaya kamera, bisa diganti millis non-blocking

  camera_fb_t * fb = esp_camera_fb_get();
  if (fb) {
    sendTelegramPhoto(fb);
    esp_camera_fb_return(fb);
  }

  digitalWrite(FLASH_PIN, LOW);
}


// ===== 3. FACE RECOGNIZED =====
if (faceRecognized) {
  delay(500);        // delay 0,5 detik (blocking)
  faceRecognized = false;

  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);

  digitalWrite(RELAY_PIN, LOW); // RELAY ON (BUKA PINTU)
  delay(6000);
  digitalWrite(RELAY_PIN, HIGH); // RELAY OFF

  sendTelegramMessage("✅ Wajah dikenali, pintu terbuka");

}

  delay(200);
}
