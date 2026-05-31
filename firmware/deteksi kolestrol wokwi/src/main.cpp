/*
 * =====================================================
 * MOCKING SENSOR MAX30100 VIA ADC POTENSIOMETER
 * Target : ATmega328P @ 16 MHz (Pure Bare-Metal Wokwi)
 * =====================================================
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/twi.h>
#include <stdint.h>
#include <string.h>

// CLOCK & TIMING
#define F_CPU_HZ 16000000UL
#define BAUD 9600UL

volatile uint32_t _millis_count = 0;
ISR(TIMER0_COMPA_vect) { _millis_count++; }

static inline uint32_t millis_get(void) {
    uint32_t val;
    uint8_t sreg = SREG;
    cli();
    val = _millis_count;
    SREG = sreg;
    return val;
}

static void timer0_init(void) {
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01) | (1 << CS00);
    OCR0A = 249;
    TIMSK0 = (1 << OCIE0A);
}

static void delay_ms(uint16_t ms) {
    uint32_t start = millis_get();
    while ((millis_get() - start) < (uint32_t)ms) {}
}

// PERIPHERAL INITIALIZATION (ADC & GPIO)
static void adc_init(void) {
    // Referensi tegangan ke AVCC dengan kapasitor eksternal di AREF
    ADMUX = (1 << REFS0);
    // Aktifkan ADC dan atur Prescaler ke 128 (16MHz / 128 = 125kHz)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

static uint16_t adc_read(uint8_t ch) {
    // Pilih channel data ADC (0-7)
    ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);
    // Mulai konversi
    ADCSRA |= (1 << ADSC);
    // Tunggu proses konversi selesai
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

static void gpio_init(void) {
    // Set Pin D2 (PD2) sebagai Input
    DDRD &= ~(1 << DDD2);
    // Aktifkan Internal Pull-up pada Pin D2
    PORTD |= (1 << PORTD2);
}

// UART
static void uart_init(void) {
    uint16_t ubrr = (uint16_t)((F_CPU_HZ + (BAUD * 4UL)) / (BAUD * 8UL) - 1UL);
    UCSR0A = (1 << U2X0);
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr & 0xFF);
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static void uart_putc(char c) {
    while (!(UCSR0A & (1 << UDRE0))) {}
    UDR0 = (uint8_t)c;
}

static void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}

static void uart_puts_P(const char *s) {
    char c;
    while ((c = pgm_read_byte(s++))) uart_putc(c);
}

static void uart_print_u32(uint32_t val) {
    if (val == 0) { uart_putc('0'); return; }
    char buf[11]; int8_t i = 0;
    while (val > 0) { buf[i++] = (char)('0' + (val % 10)); val /= 10; }
    for (int8_t j = i - 1; j >= 0; j--) uart_putc(buf[j]);
}

static void uart_print_float1(float val) {
    if (val < 0.0f) { uart_putc('-'); val = -val; }
    uint32_t integer = (uint32_t)val;
    uint32_t frac = (uint32_t)((val - (float)integer) * 10.0f + 0.5f);
    if (frac >= 10) { integer++; frac = 0; }
    uart_print_u32(integer);
    uart_putc('.');
    uart_putc((char)('0' + frac));
}
#define UART_P(s) uart_puts_P(PSTR(s))

// I2C (TWI)
#define I2C_FREQ 400000UL 
#define TWBR_VAL ((uint8_t)((F_CPU_HZ / I2C_FREQ - 16) / 2))

static void i2c_init(void) {
    TWSR = 0x00;
    TWBR = TWBR_VAL;
    TWCR = (1 << TWEN);
}

static uint8_t i2c_start(uint8_t addr_rw) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT))) {}
    uint8_t st = TW_STATUS;
    if (st != TW_START && st != TW_REP_START) return 0;
    TWDR = addr_rw;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT))) {}
    return (TW_STATUS == TW_MT_SLA_ACK || TW_STATUS == TW_MR_SLA_ACK) ? 1 : 0;
}

static void i2c_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    while (TWCR & (1 << TWSTO)) {}
}

static uint8_t i2c_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT))) {}
    return (TW_STATUS == TW_MT_DATA_ACK) ? 1 : 0;
}

// OLED SSD1306
#define SSD1306_ADDR 0x3C

void OLED_Command(uint8_t cmd) {
    if (i2c_start((SSD1306_ADDR << 1) | TW_WRITE)) {
        i2c_write(0x00); i2c_write(cmd); i2c_stop();
    }
}

void OLED_SetCursor(uint8_t page, uint8_t column) {
    OLED_Command(0xB0 + page);
    OLED_Command(column & 0x0F);
    OLED_Command(0x10 | (column >> 4));
}

void OLED_Clear(void) {
    for(uint8_t page = 0; page < 8; page++) {
        OLED_SetCursor(page, 0);
        if (i2c_start((SSD1306_ADDR << 1) | TW_WRITE)) {
            i2c_write(0x40); 
            for(uint8_t col = 0; col < 128; col++) i2c_write(0x00);
            i2c_stop();
        }
    }
}

void OLED_Init(void) {
    delay_ms(100);
    OLED_Command(0xAE); OLED_Command(0xD5); OLED_Command(0x80); 
    OLED_Command(0xA8); OLED_Command(0x3F); OLED_Command(0xD3); OLED_Command(0x00); 
    OLED_Command(0x40); OLED_Command(0x8D); OLED_Command(0x14); OLED_Command(0x20); OLED_Command(0x00); 
    OLED_Command(0xA1); OLED_Command(0xC8); 
    OLED_Command(0xDA); OLED_Command(0x12); OLED_Command(0x81); OLED_Command(0xCF); 
    OLED_Command(0xD9); OLED_Command(0xF1); OLED_Command(0xDB); OLED_Command(0x40); 
    OLED_Command(0xA4); OLED_Command(0xA6); OLED_Command(0xAF); 
}

uint8_t ReverseBits(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

// Font Data
const uint8_t FONT_A[5] PROGMEM = {0x7F,0x48,0x48,0x48,0x7F};
const uint8_t FONT_B[5] PROGMEM = {0x7F,0x49,0x49,0x49,0x36};
const uint8_t FONT_C[5] PROGMEM = {0x3E,0x41,0x41,0x41,0x22};
const uint8_t FONT_D[5] PROGMEM = {0x7F,0x41,0x41,0x22,0x1C};
const uint8_t FONT_E[5] PROGMEM = {0x7F,0x49,0x49,0x49,0x41};
const uint8_t FONT_F[5] PROGMEM = {0x7F,0x09,0x09,0x09,0x01};
const uint8_t FONT_G[5] PROGMEM = {0x3E,0x41,0x49,0x49,0x7A};
const uint8_t FONT_H[5] PROGMEM = {0x7F,0x08,0x08,0x08,0x7F};
const uint8_t FONT_I[5] PROGMEM = {0x00,0x41,0x7F,0x41,0x00};
const uint8_t FONT_J[5] PROGMEM = {0x20,0x40,0x41,0x3F,0x01};
const uint8_t FONT_K[5] PROGMEM = {0x7F,0x08,0x14,0x22,0x41};
const uint8_t FONT_L[5] PROGMEM = {0x7F,0x01,0x01,0x01,0x01};
const uint8_t FONT_M[5] PROGMEM = {0x7F,0x40,0x30,0x40,0x7F};
const uint8_t FONT_N[5] PROGMEM = {0x7F,0x20,0x10,0x08,0x7F};
const uint8_t FONT_O[5] PROGMEM = {0x3E,0x41,0x41,0x41,0x3E};
const uint8_t FONT_P[5] PROGMEM = {0x7F,0x48,0x48,0x48,0x30};
const uint8_t FONT_Q[5] PROGMEM = {0x3E,0x41,0x51,0x21,0x5E};
const uint8_t FONT_R[5] PROGMEM = {0x7F,0x48,0x4C,0x4A,0x31};
const uint8_t FONT_S[5] PROGMEM = {0x39,0x49,0x49,0x49,0x4E};
const uint8_t FONT_T[5] PROGMEM = {0x40,0x40,0x7F,0x40,0x40};
const uint8_t FONT_U[5] PROGMEM = {0x7E,0x01,0x01,0x01,0x7E};
const uint8_t FONT_V[5] PROGMEM = {0x1F,0x20,0x40,0x20,0x1F};
const uint8_t FONT_W[5] PROGMEM = {0x7F,0x02,0x0C,0x02,0x7F};
const uint8_t FONT_X[5] PROGMEM = {0x63,0x14,0x08,0x14,0x63};
const uint8_t FONT_Y[5] PROGMEM = {0x60,0x10,0x0F,0x10,0x60};
const uint8_t FONT_Z[5] PROGMEM = {0x43,0x45,0x49,0x51,0x61};

const uint8_t FONT_0[5] PROGMEM = {0x3E,0x51,0x49,0x45,0x3E};
const uint8_t FONT_1[5] PROGMEM = {0x00,0x42,0x7F,0x40,0x00};
const uint8_t FONT_2[5] PROGMEM = {0x42,0x61,0x51,0x49,0x46};
const uint8_t FONT_3[5] PROGMEM = {0x21,0x41,0x45,0x4B,0x31};
const uint8_t FONT_4[5] PROGMEM = {0x18,0x14,0x12,0x7F,0x10};
const uint8_t FONT_5[5] PROGMEM = {0x27,0x45,0x45,0x45,0x39};
const uint8_t FONT_6[5] PROGMEM = {0x3C,0x4A,0x49,0x49,0x30};
const uint8_t FONT_7[5] PROGMEM = {0x01,0x71,0x09,0x05,0x03};
const uint8_t FONT_8[5] PROGMEM = {0x36,0x49,0x49,0x49,0x36};
const uint8_t FONT_9[5] PROGMEM = {0x06,0x49,0x49,0x29,0x1E};

const uint8_t FONT_DOT[5] PROGMEM = {0x00,0x60,0x60,0x00,0x00};
const uint8_t FONT_COLON[5] PROGMEM = {0x00,0x36,0x36,0x00,0x00};
const uint8_t FONT_PCT[5] PROGMEM = {0x23,0x13,0x08,0x64,0x62};
const uint8_t FONT_MINUS[5] PROGMEM = {0x08,0x08,0x08,0x08,0x08};

void OLED_PrintChar(char c) {
    const uint8_t *g = 0; uint8_t butuh_diputar = 0;
    if (c >= 'A' && c <= 'Z') {
        const uint8_t* const alpha[] = {FONT_A,FONT_B,FONT_C,FONT_D,FONT_E,FONT_F,FONT_G,FONT_H,FONT_I,FONT_J,FONT_K,FONT_L,FONT_M,FONT_N,FONT_O,FONT_P,FONT_Q,FONT_R,FONT_S,FONT_T,FONT_U,FONT_V,FONT_W,FONT_X,FONT_Y,FONT_Z};
        g = alpha[c - 'A']; butuh_diputar = 1;
    } else if (c >= '0' && c <= '9') {
        const uint8_t* const num[] = {FONT_0,FONT_1,FONT_2,FONT_3,FONT_4,FONT_5,FONT_6,FONT_7,FONT_8,FONT_9};
        g = num[c - '0']; butuh_diputar = 0;
    } else {
        switch(c) {
            case '.': g = FONT_DOT; break;
            case ':': g = FONT_COLON; break;
            case '%': g = FONT_PCT; break;
            case '-': g = FONT_MINUS; break;
            case ' ': 
                if (i2c_start((SSD1306_ADDR << 1) | TW_WRITE)) {
                    i2c_write(0x40); for(uint8_t i=0; i<4; i++) i2c_write(0x00); i2c_stop();
                }
                return;
        }
        butuh_diputar = 0;
    }
    if (g && i2c_start((SSD1306_ADDR << 1) | TW_WRITE)) {
        i2c_write(0x40); 
        for(uint8_t i=0; i<5; i++) {
            uint8_t data = pgm_read_byte(&g[i]);
            if (butuh_diputar) data = ReverseBits(data);
            i2c_write(data);
        }
        i2c_write(0x00); i2c_stop();
    }
}

void OLED_Print(const char* str) { while (*str) { OLED_PrintChar(*str++); } }

static void OLED_PrintInt(uint32_t val) {
    if (val == 0) { OLED_PrintChar('0'); return; }
    char buf[11]; int8_t i = 0;
    while (val > 0) { buf[i++] = (char)('0' + (val % 10)); val /= 10; }
    for (int8_t j = i - 1; j >= 0; j--) OLED_PrintChar(buf[j]);
}

static void OLED_PrintFloat1(float val) {
    if (val < 0.0f) { OLED_PrintChar('-'); val = -val; }
    uint32_t integer = (uint32_t)val;
    uint32_t frac = (uint32_t)((val - (float)integer) * 10.0f + 0.5f);
    if (frac >= 10) { integer++; frac = 0; }
    OLED_PrintInt(integer); OLED_PrintChar('.'); OLED_PrintChar((char)('0' + frac));
}

// FUZZY LOGIC VARS & ENGINE
#define BPM_BUFFER_SIZE 8
static float bpmBuffer[BPM_BUFFER_SIZE];
static uint8_t bpmIndex = 0, bufFilled = 0; static float avgBPM = 0.0f;
static float calc_avg_bpm(float newBPM) {
    bpmBuffer[bpmIndex++] = newBPM;
    if (bpmIndex >= BPM_BUFFER_SIZE) { bpmIndex = 0; bufFilled = 1; }
    float sum = 0.0f; uint8_t cnt = bufFilled ? BPM_BUFFER_SIZE : bpmIndex;
    for (uint8_t i = 0; i < cnt; i++) sum += bpmBuffer[i];
    return sum / cnt;
}

typedef struct { float a, b, c, d; } TrapMF;
typedef enum { CHOL_BAIK = 0, CHOL_WASPADA = 1, CHOL_BAHAYA = 2 } CholClass;

static const TrapMF SPO2_MF[4] PROGMEM = {{0,0,75,80},{75,80,88,90},{85,90,94,95},{92,95,100,100}};
static const TrapMF BPM_MF[4] PROGMEM = {{50,50,60,70},{60,70,80,90},{80,90,100,110},{100,110,120,120}};
static const TrapMF CHOL_MF[3] PROGMEM = {{150,150,180,200},{190,200,230,240},{230,240,250,250}};
static const uint8_t RULE_TABLE[4][4] PROGMEM = {
    {CHOL_BAHAYA, CHOL_BAHAYA, CHOL_BAHAYA, CHOL_BAHAYA},
    {CHOL_BAHAYA, CHOL_BAHAYA, CHOL_BAHAYA, CHOL_BAHAYA},
    {CHOL_WASPADA, CHOL_WASPADA, CHOL_BAHAYA, CHOL_BAHAYA},
    {CHOL_WASPADA, CHOL_BAIK, CHOL_WASPADA, CHOL_BAHAYA}
};

static TrapMF read_trap(const TrapMF *base, uint8_t idx) {
    TrapMF out; memcpy_P(&out, &base[idx], sizeof(TrapMF)); return out;
}

static float trap_mf(float x, const TrapMF *mf) {
    if (x <= mf->a) return (mf->a == mf->b) ? 1.0f : 0.0f;
    if (x >= mf->d) return (mf->c == mf->d) ? 1.0f : 0.0f;
    if (x >= mf->b && x <= mf->c) return 1.0f;
    if (x > mf->a && x < mf->b) { float den = mf->b - mf->a; return (den == 0.0f) ? 1.0f : (x - mf->a) / den; }
    float den = mf->d - mf->c; return (den == 0.0f) ? 1.0f : (mf->d - x) / den;
}

static float defuzzify_centroid(const float out[3]) {
    float num = 0.0f, den = 0.0f;
    TrapMF mB = read_trap(CHOL_MF, CHOL_BAIK), mW = read_trap(CHOL_MF, CHOL_WASPADA), mBh = read_trap(CHOL_MF, CHOL_BAHAYA);
    for (int16_t x = 150; x <= 250; x++) {
        float fx = (float)x;
        float a = out[CHOL_BAIK] < trap_mf(fx, &mB) ? out[CHOL_BAIK] : trap_mf(fx, &mB);
        float b = out[CHOL_WASPADA] < trap_mf(fx, &mW) ? out[CHOL_WASPADA] : trap_mf(fx, &mW);
        float c = out[CHOL_BAHAYA] < trap_mf(fx, &mBh) ? out[CHOL_BAHAYA] : trap_mf(fx, &mBh);
        float m = a > b ? a : b; if (c > m) m = c;
        num += fx * m; den += m;
    }
    return (den < 0.0001f) ? 0.0f : num / den;
}

static uint8_t classify_from_crisp(float crisp) {
    TrapMF mB = read_trap(CHOL_MF, CHOL_BAIK), mW = read_trap(CHOL_MF, CHOL_WASPADA), mBh = read_trap(CHOL_MF, CHOL_BAHAYA);
    float b = trap_mf(crisp, &mB), w = trap_mf(crisp, &mW), bh = trap_mf(crisp, &mBh);
    if (bh >= w && bh >= b) return CHOL_BAHAYA;
    if (w >= b) return CHOL_WASPADA;
    return CHOL_BAIK;
}

static float estimate_cholesterol(float spo2, float bpm, uint8_t *kelas) {
    float muSpo2[4], muBpm[4], outStr[3] = {0.0f, 0.0f, 0.0f};
    for (uint8_t i = 0; i < 4; i++) {
        TrapMF ms = read_trap(SPO2_MF, i), mb = read_trap(BPM_MF, i);
        muSpo2[i] = trap_mf(spo2, &ms); muBpm[i] = trap_mf(bpm, &mb);
    }
    for (uint8_t i = 0; i < 4; i++) {
        for (uint8_t j = 0; j < 4; j++) {
            float fs = muSpo2[i] < muBpm[j] ? muSpo2[i] : muBpm[j];
            uint8_t cls = pgm_read_byte(&RULE_TABLE[i][j]);
            if (fs > outStr[cls]) outStr[cls] = fs;
        }
    }
    float crisp = defuzzify_centroid(outStr);
    if (crisp <= 0.0f) { *kelas = CHOL_BAIK; return 0.0f; }
    *kelas = classify_from_crisp(crisp); return crisp;
}


// MAIN PROCESS LOOP
int main(void)
{
    timer0_init();
    sei();
    uart_init();
    i2c_init();
    adc_init();   // Init ADC internal bare-metal
    gpio_init();  // Init D2 sebagai input pull-up
    
    delay_ms(100);
    OLED_Init();
    OLED_Clear();
    OLED_SetCursor(3, 10);
    OLED_Print("SIMULATION READY");
    delay_ms(1000);

    uint32_t lastReport = millis_get();
    #define REPORT_MS 500U  // Dipercepat agar respons knob potensio instan

    for (;;)
    {
        uint32_t now = millis_get();

        if (now - lastReport >= REPORT_MS)
        {
            lastReport = now;

            // 1. Baca status jari dari Switch D2 (Jika switch ON/ke GND -> PIND bernilai 0/LOW)
            uint8_t fing = !(PIND & (1 << PIND2)); 

            // 2. Baca nilai ADC Potensiometer 
            uint16_t raw_bpm = adc_read(0);  // Baca Channel 0 (Pin A0)
            uint16_t raw_spo2 = adc_read(1); // Baca Channel 1 (Pin A1)

            // 3. Konversi nilai 0-1023 ke range parameter medis realistis
            float bpm = 40.0f + ((float)raw_bpm * 120.0f / 1023.0f);   // Range: 40 - 160 BPM
            float spo2 = 75.0f + ((float)raw_spo2 * 25.0f / 1023.0f);  // Range: 75 - 100 %

            if (bpm > 40.0f && bpm < 180.0f)
                avgBPM = calc_avg_bpm(bpm);

            uint8_t vSpo2 = (spo2 >= 70.0f && spo2 <= 100.0f);
            uint8_t vBpm = (avgBPM >= 50.0f && avgBPM <= 120.0f);

            uint8_t fuzzyClass = CHOL_BAIK;
            float chol = 0.0f;

            // --- TAMPILAN KE OLED ---
            OLED_Clear();
            
            if (!fing) {
                OLED_SetCursor(3, 25);
                OLED_Print("CEK JARI");
            }
            else if (!vSpo2 || !vBpm) {
                OLED_SetCursor(3, 20);
                OLED_Print("MENGUKUR...");
            }
            else {
                // Jalankan mesin logika fuzzy menggunakan data potensio
                chol = estimate_cholesterol(spo2, avgBPM, &fuzzyClass);
                
                OLED_SetCursor(0, 0);
                OLED_Print("BPM : "); OLED_PrintFloat1(avgBPM);
                
                OLED_SetCursor(2, 0);
                OLED_Print("SPO2: "); OLED_PrintFloat1(spo2); OLED_Print("%");
                
                OLED_SetCursor(4, 0);
                OLED_Print("KOL : "); OLED_PrintFloat1(chol);
                
                OLED_SetCursor(6, 0);
                OLED_Print("STS : ");
                if (fuzzyClass == CHOL_BAIK) OLED_Print("BAIK");
                else if (fuzzyClass == CHOL_WASPADA) OLED_Print("WASPADA");
                else OLED_Print("BAHAYA");
            }

            // --- SERIAL OUTPUT ---
            UART_P("BPM: "); uart_print_float1(avgBPM); 
            UART_P(" | SpO2: "); uart_print_float1(spo2);   
            UART_P(" | Kol: ");
            if (fing && vSpo2 && vBpm) { uart_print_float1(chol); } else { UART_P("0.0"); }
            UART_P(" | Jari: "); uart_puts(fing ? "ON" : "OFF"); 
            UART_P("\r\n"); 
        }
    }
    return 0;
}