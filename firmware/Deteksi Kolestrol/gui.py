import customtkinter as ctk
import tkinter as tk
import queue
import threading
import serial
import re

# Mengatur tema menjadi terang (light) bergaya medis
ctk.set_appearance_mode("light")
ctk.set_default_color_theme("blue")

PORT = "COM4"
BAUD = 9600


class SerialThread(threading.Thread):
    def __init__(self, output_queue):
        super().__init__(daemon=True)
        self.queue = output_queue
        self.running = True

        try:
            self.ser = serial.Serial(PORT, BAUD, timeout=1)
        except Exception as e:
            self.ser = None
            self.queue.put(f"ERROR: {e}")

    def run(self):
        if self.ser is None:
            return

        while self.running:
            try:
                line = self.ser.readline().decode(errors="ignore").strip()
                if line:
                    self.queue.put(line)
            except:
                pass

    def stop(self):
        self.running = False
        try:
            self.ser.close()
        except:
            pass


class App(ctk.CTk):

    def __init__(self):
        super().__init__()

        self.title("SMARTCARE - Deteksi Kolesterol")
        # Ukuran diperkecil sedikit karena log sudah dihilangkan
        self.geometry("900x500")
        # Paksa background utama menjadi putih bersih
        self.configure(fg_color="#FFFFFF")

        self.queue = queue.Queue()
        self.serial_thread = SerialThread(self.queue)
        self.serial_thread.start()

        self.create_ui()

        self.after(100, self.update_gui)

    def create_ui(self):

        # --- HEADER SMARTCARE ---
        header_label = ctk.CTkLabel(
            self,
            text="SMARTCARE",
            font=("Segoe UI", 46, "bold"),
            text_color="#1565C0" # Warna biru medis yang menenangkan
        )
        header_label.pack(pady=(30, 0))

        subtitle_label = ctk.CTkLabel(
            self,
            text="Sistem Deteksi Kolesterol Dini Berbasis Sensor Jari",
            font=("Segoe UI", 16),
            text_color="#7F8C8D" # Abu-abu elegan
        )
        subtitle_label.pack(pady=(0, 30))

        # --- FRAME KARTU (BPM, SpO2, Kolesterol) ---
        top_frame = ctk.CTkFrame(self, fg_color="transparent")
        top_frame.pack(fill="x", padx=40)

        # Warna biru gelap untuk teks nilai utama
        self.bpm_card = self.create_card(top_frame, "BPM (Detak Jantung)", "0", "#2C3E50")
        self.spo2_card = self.create_card(top_frame, "Saturasi Oksigen (SpO2)", "0%", "#2C3E50")
        self.chol_card = self.create_card(top_frame, "Kadar Kolesterol", "0 mg/dL", "#2C3E50")

        self.bpm_card.pack(side="left", expand=True, fill="both", padx=15, pady=10)
        self.spo2_card.pack(side="left", expand=True, fill="both", padx=15, pady=10)
        self.chol_card.pack(side="left", expand=True, fill="both", padx=15, pady=10)

        # --- FRAME STATUS ---
        middle = ctk.CTkFrame(self, fg_color="transparent")
        middle.pack(fill="x", padx=20, pady=30)

        self.status_label = ctk.CTkLabel(
            middle,
            text="STATUS: -",
            font=("Segoe UI", 28, "bold"),
            text_color="#34495E"
        )
        self.status_label.pack(pady=(0, 10))

        self.finger_label = ctk.CTkLabel(
            middle,
            text="Jari Sensor: OFF",
            font=("Segoe UI", 18, "bold"),
            text_color="#95A5A6"
        )
        self.finger_label.pack()

    def create_card(self, parent, title, value, val_color):
        # Background kartu menggunakan warna biru/abu sangat muda agar kontras dengan putih
        frame = ctk.CTkFrame(parent, corner_radius=15, fg_color="#F0F4F8", border_width=1, border_color="#E0E6ED")

        title_label = ctk.CTkLabel(
            frame,
            text=title,
            font=("Segoe UI", 16, "bold"),
            text_color="#7F8C8D"
        )
        title_label.pack(pady=(25, 10))

        value_label = ctk.CTkLabel(
            frame,
            text=value,
            font=("Segoe UI", 40, "bold"),
            text_color=val_color
        )
        value_label.pack(pady=(0, 30))

        frame.value_label = value_label

        return frame

    def update_gui(self):

        while not self.queue.empty():

            line = self.queue.get()

            bpm = re.search(r"BPM Raw:\s*([\d.]+)", line)
            spo2 = re.search(r"SpO2:\s*([\d.]+)", line)
            chol = re.search(r"Kol:\s*([\d.]+)", line)
            finger = re.search(r"Jari:\s*(ON|OFF)", line)

            if bpm:
                self.bpm_card.value_label.configure(
                    text=bpm.group(1)
                )

            if spo2:
                spo2_val = float(spo2.group(1))
                self.spo2_card.value_label.configure(
                    text=f"{spo2_val:.1f}%"
                )

            if chol:
                chol_val = float(chol.group(1))
                self.chol_card.value_label.configure(
                    text=f"{chol_val:.1f} mg/dL"
                )

                # Penyesuaian warna status untuk background putih
                if chol_val < 200:
                    status = "NORMAL"
                    color = "#27AE60"  # Hijau tegas

                elif chol_val < 240:
                    status = "WASPADA"
                    color = "#E67E22"  # Oranye (kuning sulit dibaca di background putih)

                else:
                    status = "BAHAYA"
                    color = "#C0392B"  # Merah tegas

                self.status_label.configure(
                    text=f"STATUS KOLESTEROL: {status}",
                    text_color=color
                )

            if finger:
                state = finger.group(1)

                if state == "ON":
                    self.finger_label.configure(
                        text="Sensor Jari: TERDETEKSI",
                        text_color="#27AE60"
                    )
                else:
                    self.finger_label.configure(
                        text="Sensor Jari: TIDAK ADA",
                        text_color="#C0392B"
                    )

        self.after(100, self.update_gui)

    def on_close(self):
        self.serial_thread.stop()
        self.destroy()


app = App()
app.protocol("WM_DELETE_WINDOW", app.on_close)
app.mainloop()