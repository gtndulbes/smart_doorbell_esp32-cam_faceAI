# 📷 ESP32-CAM Smart Door Bell with Telegram Integration

Sistem **Smart Door Bell berbasis ESP32-CAM** yang terintegrasi dengan **Telegram Bot** untuk monitoring jarak jauh, pengambilan foto, serta kontrol kunci pintu secara real-time. Sistem ini mendukung deteksi gerakan menggunakan sensor PIR dan dapat dikembangkan untuk pengenalan wajah.

---

## 🚀 Fitur Utama

* 📸 Pengambilan foto ESP32-CAM via Telegram
* 🚨 Notifikasi gerakan dari sensor PIR
* 🔒 Kontrol kunci pintu (lock/unlock) jarak jauh
* 📡 Live streaming kamera melalui browser
* 🔔 Buzzer alarm untuk kondisi tertentu
* 🤖 Kontrol penuh menggunakan Telegram Bot
* ⚙️ Siap dikembangkan ke face recognition

---

## 🧰 Perangkat Keras (Hardware)

### Komponen:

* ESP32-CAM (AI Thinker / WROVER KIT)
* Sensor PIR (HC-SR501)
* Relay Module 1 Channel
* Buzzer
* LED Flash (onboard ESP32-CAM)
* Power supply 5V (disarankan ≥ 2A)

### Konfigurasi Pin:

| Komponen      | GPIO    |
| ------------- | ------- |
| PIR OUT       | GPIO 14 |
| Relay Control | GPIO 13 |
| Buzzer        | GPIO 12 |
| Flash LED     | GPIO 2  |

> ⚠️ Pastikan seluruh GND terhubung ke ground yang sama.

---

## 💻 Perangkat Lunak (Software)

### Tools:

* Arduino IDE (versi terbaru)
* Board ESP32 by Espressif

### Library yang Digunakan:

* `esp_camera`
* `WiFi`
* `WiFiClientSecure`
* `HTTPClient`
* `UniversalTelegramBot`
* `ArduinoJson`

---

## ⚙️ Instalasi & Konfigurasi

### 1️⃣ Konfigurasi Board ESP32

Arduino IDE → **Tools**:

| Parameter        | Value                            |
| ---------------- | -------------------------------- |
| Board            | ESP32 Wrover Module / AI Thinker |
| Upload Speed     | 115200                           |
| Flash Frequency  | 40 MHz                           |
| Partition Scheme | Huge APP                         |
| PSRAM            | Enabled                          |

---

### 2️⃣ Pilih Model Kamera

Sesuaikan dengan board yang digunakan:

```cpp
//#define CAMERA_MODEL_AI_THINKER
#define CAMERA_MODEL_WROVER_KIT
```

---

### 3️⃣ Konfigurasi WiFi

```cpp
const char* ssid = "USERNAME_WIFI";
const char* password = "PASSWORD_WIFI";
```

---

### 4️⃣ Konfigurasi Telegram Bot

1. Buat bot melalui **@BotFather**
2. Salin **Bot Token**
3. Ambil **Chat ID** menggunakan **@userinfobot**

```cpp
const char* botToken = "BOT_TOKEN_TELEGRAM";
const char* chatID  = "CHAT_ID_TELEGRAM";
```

---

## 🔌 Upload Program

1. Sambungkan ESP32-CAM ke USB to TTL
2. Tekan & tahan tombol **BOOT**
3. Upload sketch
4. Lepaskan tombol BOOT setelah upload selesai
5. Buka Serial Monitor (baudrate 115200)

Jika berhasil:

```
WiFi Connected
Camera Ready! Use 'http://IP_ADDRESS'
```

---

## 🌐 Live Streaming Kamera

Akses melalui browser:

```
http://IP_ADDRESS_ESP32
```

Atau kirim perintah Telegram:

```
/ipcam
```

---

## 🤖 Perintah Telegram Bot

| Command   | Fungsi                 |
| --------- | ---------------------- |
| `/start`  | Menampilkan menu       |
| `/photo`  | Mengambil foto         |
| `/lock`   | Mengunci pintu         |
| `/unlock` | Membuka pintu          |
| `/status` | Status sistem          |
| `/enroll` | Mode pendaftaran wajah |
| `/ipcam`  | Link live stream       |

---

## 🔄 Alur Kerja Sistem

1. **Sensor PIR mendeteksi gerakan**
2. Sistem mengirim notifikasi ke Telegram
3. Pengguna dapat:

   * Mengambil foto
   * Mengunci/membuka pintu
   * Monitoring live camera
4. Kondisi tertentu memicu buzzer & notifikasi

---

## ⚠️ Catatan Teknis

* Sistem menggunakan `WiFiClientSecure.setInsecure()` (tanpa sertifikat SSL)
* Delay bersifat blocking (tidak real-time hard system)
* Power supply harus stabil, **hindari USB laptop**
* Variabel `faceDetected` dan `faceRecognized` siap diintegrasikan dengan modul AI

---

## 🧠 Pengembangan Lanjutan

* Face Recognition ESP32
* Penyimpanan log ke database (Firebase / Supabase)
* Integrasi IoT dashboard
* Mode deep sleep untuk efisiensi daya
* Notifikasi multi-user Telegram

---

## 📄 Lisensi

Proyek ini bersifat **open-source** dan dapat digunakan untuk keperluan:

* Edukasi
* Penelitian
* Pengembangan sistem IoT

---
