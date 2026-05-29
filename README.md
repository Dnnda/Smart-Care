
<div align="center">

# 🫀 SmartCare

### *Non-Invasive Cholesterol Monitoring System*

> Sistem pemantauan kadar kolesterol secara **real-time** tanpa prosedur invasif,  
> menggunakan sensor optik dan kecerdasan *Fuzzy Logic Mamdani*.

![Device](https://img.shields.io/badge/Device-SmartCare_v1.0-blueviolet?style=flat-square&logo=arduino)
![Sensor](https://img.shields.io/badge/Sensor-MAX30102-red?style=flat-square)
![MCU](https://img.shields.io/badge/MCU-ARDUINO_UNO-blue?style=flat-square)
![GUI](https://img.shields.io/badge/Interface-Python_GUI-green?style=flat-square)
<br/>

<img width="2586" height="1425" alt="design" src="https://github.com/user-attachments/assets/e6b18d47-cbc2-45a4-9689-d40758c7b134" />


</div>

---

## 📌 Tentang Proyek

**SmartCare** adalah prototipe perangkat medis pintar yang memantau kadar kolesterol secara *non-invasive* — tanpa pengambilan sampel darah. Cukup letakkan jari pada sensor, dan dalam hitungan detik sistem akan mengklasifikasikan kadar kolesterol ke dalam kategori **Baik**, **Waspada**, atau **Bahaya**.

Sistem ini memanfaatkan sinyal **Heart Rate** dan **SpO₂** yang dibaca oleh sensor **MAX30102**, kemudian diproses melalui algoritma **Fuzzy Logic Mamdani** di mikrokontroler **Arduino Uno**, dan hasilnya ditampilkan secara real-time pada layar **OLED 0.96 inch** serta antarmuka **GUI Python** di komputer.

> 📄 Proyek ini dikembangkan berdasarkan penelitian skripsi:  
> *"Sistem Monitoring Kadar Kolesterol Secara Non-Invasive Menggunakan Sensor MAX30102 Dengan Metode Fuzzy Logic Mamdani"* > — Azra Ramadhan Pohan, Universitas Lampung, 2025

---

## ✨ Fitur Utama

| Fitur | Keterangan |
|---|---|
| 🩺 **Non-Invasive** | Tanpa jarum atau pengambilan darah |
| 🔴 **Sensor MAX30102** | Deteksi Heart Rate & SpO₂ via PPG |
| 🧠 **Fuzzy Logic Mamdani** | Klasifikasi cerdas kadar kolesterol |
| 📺 **Display OLED 0.96"** | Tampilan real-time langsung di perangkat keras |
| 🖥️ **GUI Python** | Antarmuka pengguna interaktif di komputer via komunikasi Serial |
| 📊 **Akurasi 93.85%** | Diuji terhadap alat standar medis |

---

## 🖥️ Tampilan Antarmuka

![Foto Antarmuka OLED/GUI](masukkan_link_foto_antarmuka_di_sini_nanti)

```text
┌─────────────────────────────────┐
│  ❤️  HEART RATE: 72 bpm         │  ← Tampilan OLED 0.96" & GUI Python
│  🫧  OXYGEN LEVEL: 98% SpO2     │
│                                 │
│  📊  Zonasi Kesehatan: NORMAL   │
└─────────────────────────────────┘

```

> 💡 Data dikirim dari Arduino ke komputer menggunakan **Komunikasi Serial (USB)** dan divisualisasikan secara langsung melalui GUI Python.

---

## 🏗️ Arsitektur Sistem

```text
Input Sensor                                                Output
  ┌──────────────┐           ┌──────────────────┐         ┌──────────────────┐
  │              │           │                  │ ──────▶ │ OLED 0.96 inch   │
  │    Sensor    │ ────────▶ │  Mikrokontroler  │         └──────────────────┘
  │   MAX30102   │           │   Arduino Uno    │
  │              │           │                  │         ┌──────────────────┐
  └──────────────┘           └────────┬─────────┘ ──────▶ │    GUI Python    │
                                      │                   │ (Komunikasi USB) │
                                      │                   └──────────────────┘

```

### Alur Kerja Fuzzy Logic

```text
[Heart Rate] ──┐
               ├──▶ [ Fuzzifikasi ] ──▶ [ Rule Base (16 Rules) ] ──▶ [ Defuzzifikasi ] ──▶ Kadar Kolesterol
[SpO₂]      ──┘

```

**Input Variables:**

* `SpO₂` → Hipoksemia Parah | Hipoksemia | Abnormal | Normal
* `Heart Rate` → Rendah | Normal | Tinggi | Sangat Tinggi

**Output:**

* `Kadar Kolesterol` → **Baik** (< 200 mg/dL) | **Waspada** (200–239) | **Bahaya** (≥ 240)

---

## 🔧 Hardware & Komponen

| Komponen | Spesifikasi | Fungsi |
| --- | --- | --- |
| **Arduino Uno** | ATmega328P | Mikrokontroler utama |
| **MAX30102** | PPG Sensor, Oximeter | Deteksi Heart Rate & SpO₂ |
| **OLED Display** | 0.96 inch, I2C | Display lokal |
| **Kabel USB** | Komunikasi Serial | Mengirim data ke GUI Python |
| **PCB** | Custom design | Perangkat keras terintegrasi |

### Wiring Pin (Arduino Uno ↔ Modul I2C)

*Sensor MAX30102 dan OLED 0.96" berbagi jalur I2C yang sama.*

```text
Arduino Uno          MAX30102 / OLED 0.96"
────────────         ─────────────────────
3.3V / 5V     ──▶    VCC
GND           ──▶    GND
A4            ──▶    SDA
A5            ──▶    SCL

```

---

## 📁 Struktur Repositori

```text
SmartCare/

├── 📂 assets/                 # Gambar & media (Masukkan foto di sini)
│
├── 📂 mechanical/             # Desain fisik & casing
│
├── 📂 firmware/               # Kode Sistem (Arduino IDE)
│
├── 📂 software/               # Source code GUI Python
│
├── 📂 hardware/               # Desain rangkaian & PCB
│
├── 📂 docs/                   # Dokumentasi
│
└── README.md

```

---

## 🚀 Cara Menjalankan

### Prasyarat

**Untuk Hardware (Arduino):**

* [Arduino IDE](https://www.arduino.cc/en/software) v2.x
* Library Arduino:
* `SparkFun MAX3010x Pulse and Proximity Sensor Library`
* `Adafruit GFX Library`
* `Adafruit SSD1306` (Untuk OLED 0.96")



**Untuk Software (GUI Python):**

* Python 3.x
* Library Python: `pyserial`, `tkinter` (atau library GUI yang kamu gunakan)

### Instalasi & Eksekusi

```bash
# 1. Clone repositori ini
git clone [https://github.com/your-org/smartcare.git](https://github.com/your-org/smartcare.git)
cd smartcare

# 2. Upload Firmware ke Arduino
# Buka file firmware/main.ino di Arduino IDE
# Pilih Board: Tools → Board → Arduino Uno
# Pilih Port yang sesuai, lalu klik Upload ▶

# 3. Jalankan GUI Python
cd software
pip install -r requirements.txt  # Jika ada file requirements
python main_gui.py               # Sesuaikan dengan nama file Python kamu

```

---

## 📊 Hasil Pengujian

### Akurasi Sistem

| Parameter | Kondisi | Akurasi | Error |
| --- | --- | --- | --- |
| **Heart Rate** | Tanpa cahaya tambahan | 96.6% | 3.4% |
| **Heart Rate** | Dengan cahaya tambahan | 90.0% | 10.0% |
| **SpO₂** | Normal | ~96% | ~4% |
| **Kadar Kolesterol** | Keseluruhan | **93.85%** | **6.15%** |

### Protokol Komunikasi

| Pengukuran | Nilai |
| --- | --- |
| Metode Komunikasi | Komunikasi Serial (USB) |
| Baud Rate | 9600 bps |

> ⚠️ **Catatan:** Intensitas cahaya eksternal dapat memengaruhi akurasi pembacaan sensor MAX30102. Disarankan penggunaan di tempat dengan pencahayaan ruangan normal dan pastikan jari diletakkan dengan stabil.

---

## 👥 Tim Pengembang — Kelompok 3

**🔗 Link Repository Utama:** [Dnnda/Smart-Care](https://github.com/Dnnda/Smart-Care)

![Foto Tim](masukkan_link_foto_tim_di_sini_nanti)

| Nama Lengkap | NRP | Peran Utama | Profil GitHub |
|---|---|---|---|
| **Innova Ryan Likita** | 2124600018 | Software | [@innovaryanlikitacyber](https://github.com/innovaryanlikitacyber) |
| **Muhammad Daffa Aditya Alfarizky** | 2124600014 | Hardware | [@dappadityaa](https://github.com/dappadityaa) |
| **Dimas Nanda Pratama** | 2124600018 | Mechanic | [@Dnnda](https://github.com/Dnnda) |
| **Sahrul Effendi** | 2124600024 | UI/UX | [@SahrulEffendi](https://github.com/SahrulEffendi) |
| **Adi Chandra Winata** | 2124600028 | UI/UX | [@chandrawinataa-ui](https://github.com/chandrawinataa-ui) |
| **Muhammad Alif Algifari** | 2124640002 | Hardware | [@ahmadalif200](https://github.com/ahmadalif200) |

---

## 🧪 Referensi Ilmiah

Proyek ini dikembangkan berdasarkan:

1. Pohan, A.R. (2025). *Sistem Monitoring Kadar Kolesterol Secara Non-Invasive Menggunakan Sensor MAX30102 Dengan Metode Fuzzy Logic Mamdani*. Universitas Lampung.
2. Anupongongarch, P. (2020). *A Study on Design and Construction of Non-invasive Cholesterol Sensor*. Rangsit University.
3. Rahmawati, T., et al. (2023). *Development of Non-Invasive Cholesterol Monitoring System Using TCRT5000 Sensor with Android Compatibility*. Jurnal Fisika, 13(2), 77–84.
4. NCEP ATP III Guidelines — Kadar Kolesterol Normal: <200 mg/dL, Borderline: 200–239 mg/dL, Tinggi: ≥240 mg/dL.

---

## 📄 Lisensi

[Tambahkan Lisensi di sini, misal: MIT License]

---

**SmartCare** — *Monitoring Kolesterol, Tanpa Rasa Sakit* 🫀

Made with ❤️ by **Kelompok 3**
