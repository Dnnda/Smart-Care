![Banner Proyek](Assets/banner-smart-care.png)``

# SP3S - Smart Portable Stove Safety System

## Rancang Bangun Sistem Pengaman Otomatis Kebocoran Gas dan Overheat pada Kompor Portable

---

### 📝 Deskripsi Proyek

Proyek **Smart Care** ini merupakan sistem keamanan otomatis pada kompor portable yang dirancang untuk mendeteksi kebocoran gas LPG dan kondisi *overheat* pada tabung gas secara *real-time*. Sistem menggunakan sensor gas MQ-6 untuk mendeteksi kebocoran gas serta thermistor NTC 100K untuk memonitor suhu pada area tabung gas.

Apabila terdeteksi kebocoran gas atau suhu melebihi batas aman, sistem akan:
* Mengaktifkan alarm *buzzer* peringatan.
* Menutup aliran gas secara otomatis menggunakan *servo motor*.
* Memberikan *monitoring* kondisi sistem secara *real-time*.

---

### 🛠️ Komponen Perangkat Keras (Hardware)

Sistem ini dibangun menggunakan perpaduan komponen elektronika berikut:
* **Mikrokontroler:** (Isi dengan jenis board yang kamu pakai, misal Arduino/ESP32)
* **Sensor Gas:** MQ-6
* **Sensor Suhu:** Thermistor NTC 100K
* **Aktuator:** Motor Servo & Buzzer

---

### 📂 Struktur Repository

Repository ini disusun ke dalam beberapa direktori untuk memudahkan pengembangan dan pembacaan:

* 📁 **`3D Design/`** : Berisi file rancangan model 3D untuk wadah/casing alat.
* 📁 **`Assets/`** : Berisi gambar dokumentasi, foto alat fisik, dan aset visual pendukung.
* 📁 **`Blokdiagram/`** : Gambar skema blok sistem keamanan dan *wiring* komponen elektronika.
* 📁 **`Firmware/`** : *Source code* program utama (C/C++) yang ditanamkan pada mikrokontroler.
* 📁 **`Flowchart/`** : Diagram alir logika pemrograman dan respon sistem terhadap bahaya.
* 📁 **`UI-UX/`** : Rancangan desain antarmuka (Figma) untuk aplikasi pemantauan *real-time*.

---

### 🚀 Cara Instalasi dan Penggunaan

1. *Clone* repository ini ke komputer lokal Anda:
   ```bash
   git clone [https://github.com/Dnnda/Smart-Care.git](https://github.com/Dnnda/Smart-Care.git)