<div align="center">

# 🫀 SmartCare

### *Non-Invasive Cholesterol Monitoring System*

> Sistem pemantauan kadar kolesterol secara **real-time** tanpa prosedur invasif,  
> menggunakan sensor optik dan kecerdasan *Fuzzy Logic Mamdani*.

![Device](https://img.shields.io/badge/Device-SmartCare_v1.0-blueviolet?style=flat-square&logo=arduino)
![Sensor](https://img.shields.io/badge/Sensor-MAX30102-red?style=flat-square)
![Firmware](https://img.shields.io/badge/Firmware-Bare_Metal-black?style=flat-square)
![MCU](https://img.shields.io/badge/MCU-ARDUINO_UNO-blue?style=flat-square)
![GUI](https://img.shields.io/badge/Interface-Python_GUI-green?style=flat-square)
<br/>

<img width="2586" height="1425" alt="design" src="https://github.com/user-attachments/assets/e6b18d47-cbc2-45a4-9689-d40758c7b134" />

</div>

---

## 📌 Tentang Proyek

**SmartCare** adalah prototipe perangkat medis pintar yang memantau kadar kolesterol secara *non-invasive* — tanpa pengambilan sampel darah. Cukup letakkan jari pada sensor, dan dalam hitungan detik sistem akan mengklasifikasikan kadar kolesterol ke dalam kategori **Baik**, **Waspada**, atau **Bahaya**.

Sistem ini memanfaatkan sinyal **Heart Rate** dan **SpO₂** yang dibaca oleh sensor **MAX30102**, kemudian diproses melalui algoritma **Fuzzy Logic Mamdani** di mikrokontroler **Arduino Uno**. *Firmware* perangkat ini ditulis secara **Bare Metal** (tanpa *library* eksternal) untuk memaksimalkan efisiensi. Hasil pemrosesan ditampilkan secara real-time pada layar **OLED 0.96 inch** serta divisualisasikan melalui antarmuka **GUI Python** di komputer.

> 📄 Proyek ini dikembangkan berdasarkan penelitian skripsi:  
> *"Sistem Monitoring Kadar Kolesterol Secara Non-Invasive Menggunakan Sensor MAX30102 Dengan Metode Fuzzy Logic Mamdani"* > — Azra Ramadhan Pohan, Universitas Lampung, 2025

---

## ✨ Fitur Utama

| Fitur | Keterangan |
|---|---|
| 🩺 **Non-Invasive** | Tanpa jarum atau pengambilan darah |
| 🔴 **Sensor MAX30102** | Deteksi Heart Rate & SpO₂ via PPG |
| 🧠 **Fuzzy Logic Mamdani** | Klasifikasi cerdas kadar kolesterol |
| 💻 **Bare Metal Programming** | *Firmware* ringan dan cepat tanpa dependensi *library* eksternal |
| 📺 **Display OLED 0.96"** | Tampilan real-time langsung di perangkat keras |
| 🖥️ **GUI Python** | Antarmuka pengguna interaktif di komputer via komunikasi Serial |
| 📊 **Akurasi 93.85%** | Diuji terhadap alat standar medis |

---

## 🖥️ Tampilan Antarmuka

<img width="1126" height="663" alt="Tampilan GUI" src="https://github.com/user-attachments/assets/0bae8336-4374-4dcc-9a9c-1640e342e082" />


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
  │              │           │   (Bare Metal)   │         ┌──────────────────┐
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
### 🧠 Pemodelan Fuzzy Logic Mamdani (MATLAB)

Sebelum diimplementasikan ke dalam mikrokontroler menggunakan bahasa C/C++ secara *bare metal*, sistem pakar Fuzzy Logic dimodelkan dan disimulasikan terlebih dahulu menggunakan **MATLAB Fuzzy Logic Toolbox**. 

**1. Desain Sistem Inferensi Fuzzy (FIS)**
Pemodelan ini menggunakan metode Mamdani dengan dua variabel input (`Saturasi_Oksigen` dan `Denyut_Jantung`) yang akan menghasilkan satu variabel output (`Kadar_Kolesterol`).

![FIS Editor MATLAB]<img width="702" height="596" alt="image" src="https://github.com/user-attachments/assets/5c82826f-bb02-46de-afcd-fe3202c08907" />


**2. Fungsi Keanggotaan (Membership Function)**
Pemetaan nilai numerik dari pembacaan sensor ke dalam variabel linguistik (Himpunan Fuzzy).
* **Input 1 - Saturasi Oksigen:** Dibagi menjadi 4 himpunan (*Hipoksemia_Parah, Hipoksemia, Abnormal, Normal*).
<img width="1920" height="1030" alt="image" src="https://github.com/user-attachments/assets/5861c41e-600e-4f1a-9c38-cfd8da526c89" />


* **Input 2 - Denyut Jantung:** Dibagi menjadi 4 himpunan (*Rendah, Normal, Tinggi, Sangat_Tinggi*).
<img width="1920" height="1030" alt="image" src="https://github.com/user-attachments/assets/6aa250b7-c70f-4aee-92ef-30b8a9246009" />


* **Output - Kadar Kolesterol:** Dibagi menjadi 3 himpunan (*Baik, Waspada, Bahaya*).
<img width="1920" height="1030" alt="image" src="https://github.com/user-attachments/assets/558b0296-2ac7-4886-814e-e29576db18c7" />


**3. Basis Aturan (Rule Base)**
Pembentukan 16 aturan (IF-THEN rules) berdasarkan kombinasi nilai dari kedua input untuk menentukan tingkat bahaya kolesterol.

<img width="702" height="596" alt="image" src="https://github.com/user-attachments/assets/b286f915-86ad-4dc1-abab-a48cb6fc5f60" />


**4. Evaluasi dan Defuzzifikasi (Rule Viewer)**
Simulasi proses inferensi dan defuzzifikasi (menggunakan metode Centroid). Pada contoh di bawah, dengan input `Saturasi_Oksigen` = 96.7 dan `Denyut_Jantung` = 104, sistem memprediksi `Kadar_Kolesterol` berada di angka 219 (Masuk ke dalam zonasi Waspada/Bahaya).

<img width="701" height="597" alt="image" src="https://github.com/user-attachments/assets/75faff1d-2317-428a-b564-efadb1b20976" />

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
├── 📂 firmware/               # Kode Sistem Bare Metal (C/C++)
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

**Untuk Hardware (Arduino Uno):**

* [Arduino IDE](https://www.arduino.cc/en/software) v2.x (Atau *toolchain* AVR GCC/Atmel Studio)
* **Tanpa Library Tambahan:** Program ini ditulis secara *bare metal*, sehingga tidak memerlukan instalasi *library* eksternal untuk sensor maupun OLED. Konfigurasi I2C (TWI) dilakukan langsung ke level register.

**Untuk Software (GUI Python):**

* Python 3.x
* Library Python: `pyserial`, `tkinter` (atau library GUI yang kamu gunakan)

### Instalasi & Eksekusi

```bash
# 1. Clone repositori ini
git clone [https://github.com/Dnnda/Smart-Care.git](https://github.com/Dnnda/Smart-Care.git)
cd Smart-Care

# 2. Upload Firmware ke Arduino
# Buka file utama di folder firmware menggunakan Arduino IDE
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
| Metode Komunikasi | Komunikasi Serial (USB) / UART |
| Baud Rate | 9600 bps (Sesuaikan dengan register USART) |

> ⚠️ **Catatan:** Intensitas cahaya eksternal dapat memengaruhi akurasi pembacaan sensor MAX30102. Disarankan penggunaan di tempat dengan pencahayaan ruangan normal dan pastikan jari diletakkan dengan stabil.

---

## 👥 Tim Pengembang — Kelompok 3

**🔗 Link Repository Utama:** [Dnnda/Smart-Care](https://github.com/Dnnda/Smart-Care)

| Nama Lengkap | NRP | Peran Utama | Profil GitHub |
| --- | --- | --- | --- |
| **Innova Ryan Likita** | 2124600010 | Software | [@innovaryanlikitacyber](https://github.com/innovaryanlikitacyber) |
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


**SmartCare** — *Monitoring Kolesterol, Tanpa Rasa Sakit* 🫀

Made with ❤️ by **Kelompok 3**
