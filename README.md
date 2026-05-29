<div align="center">

<img src="https://img.shields.io/badge/version-1.0.0-blue?style=for-the-badge" />
<img src="https://img.shields.io/badge/status-active-success?style=for-the-badge" />
<img src="https://img.shields.io/badge/license-MIT-green?style=for-the-badge" />
<img src="https://img.shields.io/badge/platform-ESP32-orange?style=for-the-badge" />

# 🫀 SmartCare

### *Non-Invasive Cholesterol Monitoring System*

> Sistem pemantauan kadar kolesterol secara **real-time** tanpa prosedur invasif,  
> menggunakan sensor optik dan kecerdasan *Fuzzy Logic Mamdani*.

<br/>

![SmartCare Device](https://img.shields.io/badge/Device-SmartCare_v1.0-blueviolet?style=flat-square&logo=arduino)
![Sensor](https://img.shields.io/badge/Sensor-MAX30105-red?style=flat-square)
![MCU](https://img.shields.io/badge/MCU-ESP32-blue?style=flat-square)
![IoT](https://img.shields.io/badge/IoT-Blynk-green?style=flat-square)

</div>

---

## 📌 Tentang Proyek

**SmartCare** adalah prototipe perangkat medis pintar yang memantau kadar kolesterol secara *non-invasive* — tanpa pengambilan sampel darah. Cukup letakkan jari pada sensor, dan dalam hitungan detik sistem akan mengklasifikasikan kadar kolesterol ke dalam kategori **Baik**, **Waspada**, atau **Bahaya**.

Sistem ini memanfaatkan sinyal **Heart Rate** dan **SpO₂** yang dibaca oleh sensor **MAX30105**, kemudian diproses melalui algoritma **Fuzzy Logic Mamdani** di mikrokontroler **ESP32**, dan hasilnya ditampilkan secara real-time pada layar **OLED** serta platform IoT **Blynk**.

> 📄 Proyek ini dikembangkan berdasarkan penelitian skripsi:  
> *"Sistem Monitoring Kadar Kolesterol Secara Non-Invasive Menggunakan Sensor MAX30105 Dengan Metode Fuzzy Logic Mamdani"*  
> — Azra Ramadhan Pohan, Universitas Lampung, 2025

---

## ✨ Fitur Utama

| Fitur | Keterangan |
|---|---|
| 🩺 **Non-Invasive** | Tanpa jarum atau pengambilan darah |
| 🔴 **Sensor MAX30105** | Deteksi Heart Rate & SpO₂ via PPG |
| 🧠 **Fuzzy Logic Mamdani** | Klasifikasi cerdas kadar kolesterol |
| 📺 **Display OLED** | Tampilan real-time langsung di perangkat |
| 📱 **Blynk IoT** | Monitoring jarak jauh via smartphone |
| ⚡ **ESP32** | Pemrosesan cepat dengan WiFi terintegrasi |
| 📊 **Akurasi 93.85%** | Diuji terhadap alat standar medis |

---

## 🖥️ Tampilan Antarmuka

```
┌─────────────────────────────────┐
│  ❤️  HEART RATE: 72 bpm         │  ← Tampilan OLED Real-Time
│  🫧  OXYGEN LEVEL: 98% SpO2     │
│      NORMAL                      │
│  📊  Zonasi Kesehatan: NORMAL    │
└─────────────────────────────────┘
```

> 💡 Hasil juga dikirim ke **Blynk Dashboard** dengan latensi rata-rata **1.38 detik**.

---

## 🏗️ Arsitektur Sistem

```
┌──────────────┐     I²C      ┌──────────────┐     WiFi     ┌──────────────┐
│  MAX30105    │ ──────────▶  │   ESP32      │ ──────────▶  │   Blynk IoT  │
│  (Sensor)    │              │  (Processing) │              │  (Dashboard) │
└──────────────┘              └──────┬───────┘              └──────────────┘
                                     │
                                     │ SPI
                                     ▼
                              ┌──────────────┐
                              │  OLED Display│
                              │  240 × 320   │
                              └──────────────┘
```

### Alur Kerja Fuzzy Logic

```
[Heart Rate] ──┐
               ├──▶ [ Fuzzifikasi ] ──▶ [ Rule Base (16 Rules) ] ──▶ [ Defuzzifikasi ] ──▶ Kadar Kolesterol
[SpO₂]     ──┘
```

**Input Variables:**
- `SpO₂` → Hipoksemia Parah | Hipoksemia | Abnormal | Normal
- `Heart Rate` → Rendah | Normal | Tinggi | Sangat Tinggi

**Output:**
- `Kadar Kolesterol` → **Baik** (< 200 mg/dL) | **Waspada** (200–239) | **Bahaya** (≥ 240)

---

## 🔧 Hardware & Komponen

| Komponen | Spesifikasi | Fungsi |
|---|---|---|
| **ESP32** | Xtensa Dual-Core LX6, 160 MHz | Mikrokontroler utama |
| **MAX30105** | PPG Sensor, 18-bit ADC | Deteksi Heart Rate & SpO₂ |
| **OLED TFT** | 240×320, SSD2212 | Display lokal |
| **Blynk Platform** | IoT Cloud | Remote monitoring |
| **PCB** | Custom design | Perangkat keras terintegrasi |
| **Akrilik** | Custom enclosure | Casing perangkat |

### Wiring Pin (ESP32 ↔ MAX30105)

```
ESP32          MAX30105
─────          ────────
3.3V    ──▶   VCC
GND     ──▶   GND
GPIO21  ──▶   SDA
GPIO22  ──▶   SCL
```

---

## 📁 Struktur Repositori

```
SmartCare/
│
├── 📂 firmware/               # Kode ESP32 (Arduino IDE)
│   ├── main.ino               # Program utama
│   ├── fuzzy_logic.cpp        # Implementasi Fuzzy Mamdani
│   ├── fuzzy_logic.h
│   ├── sensor_max30105.cpp    # Driver sensor
│   └── blynk_config.h        # Konfigurasi IoT
│
├── 📂 hardware/               # Desain rangkaian & PCB
│   ├── schematic.pdf
│   ├── pcb_layout.pdf
│   └── bom.xlsx               # Bill of Materials
│
├── 📂 ui-ux/                  # Desain antarmuka
│   ├── blynk_dashboard/       # Layout Blynk
│   ├── oled_mockup/           # Desain layar OLED
│   └── figma_assets/          # File desain
│
├── 📂 mechanical/             # Desain fisik & casing
│   ├── enclosure_3d/          # File 3D (.stl)
│   └── assembly_guide.pdf
│
├── 📂 docs/                   # Dokumentasi
│   ├── skripsi_reference.pdf
│   ├── fuzzy_rules.md
│   └── testing_results.xlsx
│
├── 📂 assets/                 # Gambar & media
│   └── device_photo.png
│
└── README.md
```

---

## 🚀 Cara Menjalankan

### Prasyarat

- [Arduino IDE](https://www.arduino.cc/en/software) v2.x
- ESP32 Board Package
- Library yang dibutuhkan:

```
SparkFun MAX3010x Pulse and Proximity Sensor Library
Blynk Library
Adafruit GFX Library
Adafruit ILI9341 (OLED TFT)
```

### Instalasi

```bash
# 1. Clone repositori ini
git clone https://github.com/your-org/smartcare.git
cd smartcare

# 2. Buka firmware di Arduino IDE
# File → Open → firmware/main.ino

# 3. Konfigurasi kredensial di blynk_config.h
#define BLYNK_TEMPLATE_ID   "YourTemplateID"
#define BLYNK_TEMPLATE_NAME "SmartCare"
#define BLYNK_AUTH_TOKEN    "YourAuthToken"
#define WIFI_SSID           "YourWiFiSSID"
#define WIFI_PASS           "YourWiFiPassword"

# 4. Upload ke ESP32
# Tools → Board → ESP32 Dev Module
# Upload ▶
```

---

## 📊 Hasil Pengujian

### Akurasi Sistem

| Parameter | Kondisi | Akurasi | Error |
|---|---|---|---|
| **Heart Rate** | Tanpa cahaya tambahan | 96.6% | 3.4% |
| **Heart Rate** | Dengan cahaya tambahan | 90.0% | 10.0% |
| **SpO₂** | Normal | ~96% | ~4% |
| **Kadar Kolesterol** | Keseluruhan | **93.85%** | **6.15%** |

### Latensi Sistem

| Pengukuran | Nilai |
|---|---|
| Rata-rata delay OLED → Blynk | **1.38 detik** |
| Protokol komunikasi | WiFi (802.11 b/g/n) |

> ⚠️ **Catatan:** Intensitas cahaya eksternal dapat memengaruhi akurasi sensor. Disarankan penggunaan di tempat dengan pencahayaan ruangan normal.

---

## 👥 Tim Pengembang — Kelompok 6

<table>
  <tr>
    <td align="center"><b>🖥️ Software</b></td>
    <td align="center"><b>🔩 Mechanic</b></td>
    <td align="center"><b>🎨 UI/UX</b></td>
    <td align="center"><b>⚙️ Hardware</b></td>
  </tr>
  <tr>
    <td align="center">
      Pengembangan firmware ESP32,<br/>implementasi Fuzzy Logic,<br/>integrasi Blynk IoT
    </td>
    <td align="center">
      Desain casing akrilik,<br/>perakitan komponen,<br/>panduan assembly
    </td>
    <td align="center">
      Desain tampilan OLED,<br/>dashboard Blynk,<br/>user experience flow
    </td>
    <td align="center">
      Desain skematik PCB,<br/>wiring komponen,<br/>pengujian rangkaian
    </td>
  </tr>
</table>

---

## 🗺️ Roadmap

- [x] Prototype v1.0 — Deteksi Heart Rate & SpO₂
- [x] Implementasi Fuzzy Logic Mamdani (16 rules)
- [x] Integrasi Blynk IoT Dashboard
- [x] Kalibrasi sensor & pengujian akurasi
- [ ] Penambahan input parameter: berat badan & usia
- [ ] Dukungan baterai (portable mode)
- [ ] Eksplorasi metode alternatif (ANFIS, Neural Network)
- [ ] Sertifikasi perangkat medis

---

## 🧪 Referensi Ilmiah

Proyek ini dikembangkan berdasarkan:

1. Pohan, A.R. (2025). *Sistem Monitoring Kadar Kolesterol Secara Non-Invasive Menggunakan Sensor MAX30105 Dengan Metode Fuzzy Logic Mamdani*. Universitas Lampung.
2. Anupongongarch, P. (2020). *A Study on Design and Construction of Non-invasive Cholesterol Sensor*. Rangsit University.
3. Rahmawati, T., et al. (2023). *Development of Non-Invasive Cholesterol Monitoring System Using TCRT5000 Sensor with Android Compatibility*. Jurnal Fisika, 13(2), 77–84.
4. NCEP ATP III Guidelines — Kadar Kolesterol Normal: <200 mg/dL, Borderline: 200–239 mg/dL, Tinggi: ≥240 mg/dL.

---

## 📄 Lisensi

Proyek ini dilisensikan di bawah [MIT License](LICENSE).

---

<div align="center">

**SmartCare** — *Monitoring Kolesterol, Tanpa Rasa Sakit* 🫀

Made with ❤️ by **Kelompok 6**

</div>
