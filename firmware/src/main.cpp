
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/twi.h>
#include <stdint.h>
#include <string.h>

// =====================================================
// CLOCK & TIMING
// =====================================================
#define F_CPU_HZ 16000000UL
#define BAUD 9600UL

// =====================================================
// TIMER0 — millis() bare-metal
// Timer0 CTC, prescaler 64 → OCR0A=249 → tick 1ms
// =====================================================
volatile uint32_t _millis_count = 0;

ISR(TIMER0_COMPA_vect) { _millis_count++; }

static inline uint32_t millis_get(void)
{
    uint32_t val;
    uint8_t sreg = SREG;
    cli();
    val = _millis_count;
    SREG = sreg;
    return val;
}

static void timer0_init(void)
{
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS01) | (1 << CS00);
    OCR0A = 249;
    TIMSK0 = (1 << OCIE0A);
}

static void delay_ms(uint16_t ms)
{
    uint32_t start = millis_get();
    while ((millis_get() - start) < (uint32_t)ms)
    {
    }
}

// =====================================================
// UART — bare-metal ATmega328P
// =====================================================
static void uart_init(void)
{
    uint16_t ubrr = (uint16_t)((F_CPU_HZ + (BAUD * 4UL)) / (BAUD * 8UL) - 1UL);
    UCSR0A = (1 << U2X0);
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr & 0xFF);
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

static void uart_putc(char c)
{
    while (!(UCSR0A & (1 << UDRE0)))
    {
    }
    UDR0 = (uint8_t)c;
}

static void uart_puts(const char *s)
{
    while (*s)
        uart_putc(*s++);
}

static void uart_puts_P(const char *s)
{
    char c;
    while ((c = pgm_read_byte(s++)))
        uart_putc(c);
}

static void uart_print_u32(uint32_t val)
{
    if (val == 0)
    {
        uart_putc('0');
        return;
    }
    char buf[11];
    int8_t i = 0;
    while (val > 0)
    {
        buf[i++] = (char)('0' + (val % 10));
        val /= 10;
    }
    for (int8_t j = i - 1; j >= 0; j--)
        uart_putc(buf[j]);
}

static void uart_print_float1(float val)
{
    if (val < 0.0f)
    {
        uart_putc('-');
        val = -val;
    }
    uint32_t integer = (uint32_t)val;
    uint32_t frac = (uint32_t)((val - (float)integer) * 10.0f + 0.5f);
    if (frac >= 10)
    {
        integer++;
        frac = 0;
    }
    uart_print_u32(integer);
    uart_putc('.');
    uart_putc((char)('0' + frac));
}

#define UART_P(s) uart_puts_P(PSTR(s))

// =====================================================
// I2C (TWI) — bare-metal ATmega328P
// =====================================================
#define I2C_FREQ 100000UL
#define TWBR_VAL ((uint8_t)((F_CPU_HZ / I2C_FREQ - 16) / 2))

static void i2c_init(void)
{
    TWSR = 0x00;
    TWBR = TWBR_VAL;
    TWCR = (1 << TWEN);
}

static uint8_t i2c_start(uint8_t addr_rw)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)))
    {
    }
    uint8_t st = TW_STATUS;
    if (st != TW_START && st != TW_REP_START)
        return 0;
    TWDR = addr_rw;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)))
    {
    }
    st = TW_STATUS;
    return (st == TW_MT_SLA_ACK || st == TW_MR_SLA_ACK) ? 1 : 0;
}

static void i2c_stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    while (TWCR & (1 << TWSTO))
    {
    }
}

static uint8_t i2c_write(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)))
    {
    }
    return (TW_STATUS == TW_MT_DATA_ACK) ? 1 : 0;
}

static uint8_t i2c_read_ack(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)))
    {
    }
    return TWDR;
}

static uint8_t i2c_read_nack(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)))
    {
    }
    return TWDR;
}

static uint8_t i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t val)
{
    if (!i2c_start((dev_addr << 1) | TW_WRITE))
    {
        i2c_stop();
        return 0;
    }
    i2c_write(reg);
    i2c_write(val);
    i2c_stop();
    return 1;
}

static uint8_t i2c_read_regs(uint8_t dev_addr, uint8_t reg,
                             uint8_t *buf, uint8_t len)
{
    if (!i2c_start((dev_addr << 1) | TW_WRITE))
    {
        i2c_stop();
        return 0;
    }
    i2c_write(reg);
    if (!i2c_start((dev_addr << 1) | TW_READ))
    {
        i2c_stop();
        return 0;
    }
    for (uint8_t i = 0; i < len; i++)
        buf[i] = (i < len - 1) ? i2c_read_ack() : i2c_read_nack();
    i2c_stop();
    return 1;
}

// =====================================================
// SSD1306 OLED 128x32 — raw I2C framebuffer
// =====================================================
#define OLED_ADDR 0x3C
#define OLED_W 128
#define OLED_H 32
#define OLED_PAGES 4 // 32px / 8 = 4 pages

static uint8_t fb[OLED_W * OLED_PAGES]; // 512 byte framebuffer
static uint8_t oledReady = 0;

static void oled_cmd(uint8_t cmd)
{
    i2c_start((OLED_ADDR << 1) | TW_WRITE);
    i2c_write(0x00);
    i2c_write(cmd);
    i2c_stop();
}

static uint8_t oled_init(void)
{
    delay_ms(10);
    const uint8_t seq[] PROGMEM = {
        0xAE,
        0xD5, 0x80,
        0xA8, 0x1F,
        0xD3, 0x00,
        0x40,
        0x8D, 0x14,
        0x20, 0x00,
        0xA1,
        0xC8,
        0xDA, 0x02,
        0x81, 0xCF,
        0xD9, 0xF1,
        0xDB, 0x40,
        0xA4,
        0xA6,
        0xAF};
    for (uint8_t i = 0; i < sizeof(seq); i++)
        oled_cmd(pgm_read_byte(&seq[i]));
    return 1;
}

static void oled_flush(void)
{
    oled_cmd(0x21);
    oled_cmd(0);
    oled_cmd(127);
    oled_cmd(0x22);
    oled_cmd(0);
    oled_cmd(3);
    i2c_start((OLED_ADDR << 1) | TW_WRITE);
    i2c_write(0x40);
    for (uint16_t i = 0; i < (uint16_t)(OLED_W * OLED_PAGES); i++)
        i2c_write(fb[i]);
    i2c_stop();
}

static void oled_clear(void) { memset(fb, 0, sizeof(fb)); }

// =====================================================
// FONT 5x8 MINIMALIS — ASCII 32..122
// =====================================================
static const uint8_t font5x8[][5] PROGMEM = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ' ' 32
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // '!' 33
    {0x00, 0x07, 0x00, 0x07, 0x00}, // '"' 34
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // '#' 35
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // '$' 36
    {0x23, 0x13, 0x08, 0x64, 0x62}, // '%' 37
    {0x36, 0x49, 0x55, 0x22, 0x50}, // '&' 38
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '\''39
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // '(' 40
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // ')' 41
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // '*' 42
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // '+' 43
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ',' 44
    {0x08, 0x08, 0x08, 0x08, 0x08}, // '-' 45
    {0x00, 0x60, 0x60, 0x00, 0x00}, // '.' 46
    {0x20, 0x10, 0x08, 0x04, 0x02}, // '/' 47
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // '0' 48
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // '1' 49
    {0x42, 0x61, 0x51, 0x49, 0x46}, // '2' 50
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // '3' 51
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // '4' 52
    {0x27, 0x45, 0x45, 0x45, 0x39}, // '5' 53
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // '6' 54
    {0x01, 0x71, 0x09, 0x05, 0x03}, // '7' 55
    {0x36, 0x49, 0x49, 0x49, 0x36}, // '8' 56
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // '9' 57
    {0x00, 0x36, 0x36, 0x00, 0x00}, // ':' 58
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ';' 59
    {0x08, 0x14, 0x22, 0x41, 0x00}, // '<' 60
    {0x14, 0x14, 0x14, 0x14, 0x14}, // '=' 61
    {0x00, 0x41, 0x22, 0x14, 0x08}, // '>' 62
    {0x02, 0x01, 0x51, 0x09, 0x06}, // '?' 63
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // '@' 64
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 'A' 65
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 'B' 66
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 'C' 67
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 'D' 68
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 'E' 69
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // 'F' 70
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 'G' 71
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 'H' 72
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 'I' 73
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 'J' 74
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 'K' 75
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 'L' 76
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 'M' 77
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 'N' 78
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 'O' 79
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 'P' 80
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 'Q' 81
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 'R' 82
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 'S' 83
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 'T' 84
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 'U' 85
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 'V' 86
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // 'W' 87
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 'X' 88
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 'Y' 89
    {0x61, 0x51, 0x49, 0x45, 0x43}, // 'Z' 90
    {0x00, 0x00, 0x00, 0x00, 0x00}, // '[' 91 (unused)
    {0x00, 0x00, 0x00, 0x00, 0x00}, // '\'  92
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ']' 93
    {0x00, 0x00, 0x00, 0x00, 0x00}, // '^' 94
    {0x00, 0x00, 0x00, 0x00, 0x00}, // '_' 95
    {0x00, 0x00, 0x00, 0x00, 0x00}, // '`' 96
    {0x20, 0x54, 0x54, 0x54, 0x78}, // 'a' 97
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // 'b' 98
    {0x38, 0x44, 0x44, 0x44, 0x20}, // 'c' 99
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // 'd' 100
    {0x38, 0x54, 0x54, 0x54, 0x18}, // 'e' 101
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // 'f' 102
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, // 'g' 103
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // 'h' 104
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // 'i' 105
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // 'j' 106
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // 'k' 107
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // 'l' 108
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // 'm' 109
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // 'n' 110
    {0x38, 0x44, 0x44, 0x44, 0x38}, // 'o' 111
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // 'p' 112
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // 'q' 113
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // 'r' 114
    {0x48, 0x54, 0x54, 0x54, 0x20}, // 's' 115
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // 't' 116
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // 'u' 117
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // 'v' 118
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // 'w' 119
    {0x44, 0x28, 0x10, 0x28, 0x44}, // 'x' 120
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // 'y' 121
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // 'z' 122
};
#define FONT_COUNT (sizeof(font5x8) / 5)

static void fb_draw_char(uint8_t x, uint8_t page, char c)
{
    if (page >= OLED_PAGES)
        return;
    uint8_t idx;
    if ((uint8_t)c >= 32 && (uint8_t)c <= 122)
        idx = (uint8_t)c - 32;
    else
        idx = 0;
    if (idx >= FONT_COUNT)
        idx = 0;

    for (uint8_t col = 0; col < 5; col++)
    {
        if (x + col >= OLED_W)
            break;
        fb[(page * OLED_W) + x + col] = pgm_read_byte(&font5x8[idx][col]);
    }
    if (x + 5 < OLED_W)
        fb[(page * OLED_W) + x + 5] = 0x00;
}

static void fb_draw_str(uint8_t x, uint8_t page, const char *s)
{
    while (*s && x < OLED_W)
    {
        fb_draw_char(x, page, *s++);
        x += 6;
    }
}

// Helper: int ke string (untuk OLED)
static void i32_to_str(int32_t val, char *buf)
{
    if (val == 0)
    {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    uint8_t neg = 0;
    if (val < 0)
    {
        neg = 1;
        val = -val;
    }
    char tmp[12];
    int8_t i = 0;
    while (val > 0)
    {
        tmp[i++] = (char)('0' + (val % 10));
        val /= 10;
    }
    if (neg)
        tmp[i++] = '-';
    int8_t j = 0;
    while (i > 0)
        buf[j++] = tmp[--i];
    buf[j] = '\0';
}

// Gabung dua string ke buf
static void str_cat(char *buf, const char *s)
{
    while (*buf)
        buf++;
    while (*s)
        *buf++ = *s++;
    *buf = '\0';
}

// =====================================================
// MAX30100 — register map
// =====================================================
#define MAX30100_ADDR 0x57
#define MAX30100_REG_INT_STATUS 0x00
#define MAX30100_REG_INT_ENABLE 0x01
#define MAX30100_REG_FIFO_WR_PTR 0x02
#define MAX30100_REG_FIFO_OV_CNT 0x03
#define MAX30100_REG_FIFO_RD_PTR 0x04
#define MAX30100_REG_FIFO_DATA 0x05
#define MAX30100_REG_MODE_CONFIG 0x06
#define MAX30100_REG_SPO2_CONFIG 0x07
#define MAX30100_REG_LED_CONFIG 0x09
#define MAX30100_REG_PART_ID 0xFF

#define MAX30100_MODE_SPO2_HR 0x03
#define MAX30100_SPO2_HI_RES_EN (1 << 6)
#define MAX30100_SR_100 0x00
#define MAX30100_PW_1600 0x03

// =====================================================
// SIGNAL PROCESSING — filter chain
// =====================================================
typedef struct
{
    float w;
} DCFilter_t;

typedef struct
{
    float v[2];
} ButterworthLP_t;

#define MEAN_FILTER_SIZE 15
typedef struct
{
    float values[MEAN_FILTER_SIZE];
    uint8_t index;
    float sum;
    uint8_t count;
} MeanDiff_t;

// DC removal — high-pass IIR
static float dc_remove(DCFilter_t *f, float x)
{
    float old_w = f->w;
    f->w = x + 0.95f * old_w; // α=0.95 → ~cutoff 0.8Hz @ 100SPS
    return f->w - old_w;
}

// Butterworth LP 2nd order, fc≈3Hz @ 100SPS — Direct Form II
static float butter_lp(ButterworthLP_t *f, float x)
{
    // Coefficients fc=3/100 Butterworth 2nd order
    const float b0 = 0.02008337f;
    const float b1 = 0.04016674f;
    const float b2 = 0.02008337f;
    const float a1 = -1.56101807f;
    const float a2 = 0.64135154f;
    float w = x - a1 * f->v[0] - a2 * f->v[1];
    float result = b0 * w + b1 * f->v[0] + b2 * f->v[1];
    f->v[1] = f->v[0];
    f->v[0] = w;
    return result;
}

// Mean diff filter
static float mean_diff(MeanDiff_t *f, float x)
{
    f->sum -= f->values[f->index];
    f->sum += x;
    f->values[f->index] = x;
    f->index = (f->index + 1) % MEAN_FILTER_SIZE;
    if (f->count < MEAN_FILTER_SIZE)
        f->count++;
    return (f->sum / f->count) - x;
}

// =====================================================
// PULSE OXIMETER STATE
// =====================================================
typedef struct
{
    DCFilter_t irDC, redDC;
    ButterworthLP_t lpIR;
    MeanDiff_t meanDiff;

    // Beat detection
    float prevIR;
    float threshold;
    uint32_t lastBeatMs;
    float bpm;

    // SpO2 — akumulator RMS per beat window
    float acIR_sum;
    float acRed_sum;
    uint16_t acSamples;

    // Output tersaring
    float spO2;
    uint8_t fingerOn; // 1 = jari terpasang
} PulseOx_t;

static PulseOx_t pox_state;

static uint8_t max30100_init(void)
{
    uint8_t part_id = 0;
    i2c_read_regs(MAX30100_ADDR, MAX30100_REG_PART_ID, &part_id, 1);
    if (part_id != 0x11)
        return 0;

    i2c_write_reg(MAX30100_ADDR, MAX30100_REG_MODE_CONFIG, 0x40); // reset
    delay_ms(50);
    i2c_write_reg(MAX30100_ADDR, MAX30100_REG_MODE_CONFIG, MAX30100_MODE_SPO2_HR);
    i2c_write_reg(MAX30100_ADDR, MAX30100_REG_SPO2_CONFIG,
                  MAX30100_SPO2_HI_RES_EN | (MAX30100_SR_100 << 2) | MAX30100_PW_1600);
    // LED: RED=11mA(0x4), IR=11mA(0x4)
    i2c_write_reg(MAX30100_ADDR, MAX30100_REG_LED_CONFIG, 0x77);
    i2c_write_reg(MAX30100_ADDR, MAX30100_REG_INT_ENABLE, 0x10);

    memset(&pox_state, 0, sizeof(pox_state));
    pox_state.threshold = 0.03f;
    return 1;
}

static uint8_t max30100_read_fifo(uint16_t *ir, uint16_t *red)
{
    uint8_t wr, rd;
    i2c_read_regs(MAX30100_ADDR, MAX30100_REG_FIFO_WR_PTR, &wr, 1);
    i2c_read_regs(MAX30100_ADDR, MAX30100_REG_FIFO_RD_PTR, &rd, 1);
    if (wr == rd)
        return 0;

    uint8_t buf[4];
    i2c_read_regs(MAX30100_ADDR, MAX30100_REG_FIFO_DATA, buf, 4);
    // Format MAX30100: [RED_H][RED_L][IR_H][IR_L]
    *red = ((uint16_t)buf[0] << 8) | buf[1];
    *ir = ((uint16_t)buf[2] << 8) | buf[3];
    return 1;
}

static void max30100_update(void)
{
    uint16_t ir_raw, red_raw;
    while (max30100_read_fifo(&ir_raw, &red_raw))
    {
        // ---- Deteksi jari ----
        // Tanpa jari: IR DC < 6000. Dengan jari: IR DC > 30000 (16-bit, 100SPS)
        // Gunakan raw value langsung sebagai proxy DC
        pox_state.fingerOn = (ir_raw > 6000) ? 1 : 0;

        // ---- Filter chain ----
        float ir_ac = dc_remove(&pox_state.irDC, (float)ir_raw);
        float red_ac = dc_remove(&pox_state.redDC, (float)red_raw);
        float ir_lp = butter_lp(&pox_state.lpIR, ir_ac);
        float ir_mf = mean_diff(&pox_state.meanDiff, ir_lp);

        // ---- Beat detection ----
        float current = ir_mf;
        if (pox_state.fingerOn &&
            pox_state.prevIR < pox_state.threshold &&
            current >= pox_state.threshold)
        {
            uint32_t now = millis_get();
            uint32_t dt = now - pox_state.lastBeatMs;
            if (dt > 333 && dt < 1500) // 40..180 BPM
            {
                float nbpm = 60000.0f / (float)dt;
                pox_state.bpm = (pox_state.bpm < 10.0f)
                                    ? nbpm
                                    : 0.75f * pox_state.bpm + 0.25f * nbpm;
                UART_P("Beat Detected!\r\n");

                // ---- SpO2: hitung R dari RMS AC per beat window ----
                // ---- SpO2 IMPROVED ----
                if (pox_state.acSamples > 20)
                {
                    // RMS AC
                    float ac_ir = pox_state.acIR_sum / (float)pox_state.acSamples;
                    float ac_red = pox_state.acRed_sum / (float)pox_state.acSamples;

                    // DC component
                    float dc_ir = pox_state.irDC.w;
                    float dc_red = pox_state.redDC.w;

                    if (dc_ir > 1000.0f && dc_red > 1000.0f &&
                        ac_ir > 1.0f && ac_red > 1.0f)
                    {
                        // Ratio-of-ratios
                        float ratio_red = ac_red / dc_red;
                        float ratio_ir = ac_ir / dc_ir;

                        // Hindari divide terlalu kecil
                        if (ratio_ir < 0.0001f)
                            ratio_ir = 0.0001f;

                        float R = (ratio_red / ratio_ir) * 0.15f;

                        // Kalibrasi empiris MAX30100

                        // Kalibrasi MAX30100
                        R *= 0.48f;

                        // Batasi agar tidak liar
                        if (R < 0.4f)
                            R = 0.4f;
                        if (R > 1.8f)
                            R = 1.8f;

                        // Formula empiris MAX30100 paling stabil
                        float spo2 = 104.0f - (17.0f * R);

                        // Clamp realistis
                        if (spo2 > 100.0f)
                            spo2 = 100.0f;
                        if (spo2 < 85.0f)
                            spo2 = 85.0f;

                        // EMA smoothing
                        if (pox_state.spO2 < 1.0f)
                            pox_state.spO2 = spo2;
                        else
                            pox_state.spO2 =
                                (0.7f * pox_state.spO2) +
                                (0.3f * spo2);
                    }
                }
                // Reset akumulator
                pox_state.acIR_sum = 0.0f;
                pox_state.acRed_sum = 0.0f;
                pox_state.acSamples = 0;
            }
            pox_state.lastBeatMs = now;
        }
        pox_state.prevIR = current;

        // Adaptive threshold
        float abs_cur = current < 0.0f ? -current : current;
        pox_state.threshold = 0.65f * pox_state.threshold + 0.35f * (abs_cur * 0.4f);
        if (pox_state.threshold < 0.02f)
            pox_state.threshold = 0.02f;
        if (pox_state.threshold > 2.0f)
            pox_state.threshold = 2.0f;

        pox_state.acIR_sum += ir_ac * ir_ac;
        pox_state.acRed_sum += red_ac * red_ac;

        if (pox_state.acSamples < 120)
            pox_state.acSamples++;
        else
        {
            pox_state.acIR_sum *= 0.5f;
            pox_state.acRed_sum *= 0.5f;
            pox_state.acSamples = 60;
        }

        // Reset jika jari diangkat
        if (!pox_state.fingerOn)
        {
            pox_state.spO2 = 0.0f;
            pox_state.bpm = 0.0f;
            pox_state.acIR_sum = 0.0f;
            pox_state.acRed_sum = 0.0f;
            pox_state.acSamples = 0;
        }
    }
}

static inline float max30100_get_bpm(void) { return pox_state.bpm; }
static inline float max30100_get_spo2(void) { return pox_state.spO2; }
static inline uint8_t max30100_finger_on(void) { return pox_state.fingerOn; }

// =====================================================
// MOVING AVERAGE BPM (8 sample)
// =====================================================
#define BPM_BUFFER_SIZE 8
static float bpmBuffer[BPM_BUFFER_SIZE];
static uint8_t bpmIndex = 0;
static uint8_t bufFilled = 0;
static float avgBPM = 0.0f;

static float calc_avg_bpm(float newBPM)
{
    bpmBuffer[bpmIndex++] = newBPM;
    if (bpmIndex >= BPM_BUFFER_SIZE)
    {
        bpmIndex = 0;
        bufFilled = 1;
    }
    float sum = 0.0f;
    uint8_t cnt = bufFilled ? BPM_BUFFER_SIZE : bpmIndex;
    for (uint8_t i = 0; i < cnt; i++)
        sum += bpmBuffer[i];
    return sum / cnt;
}

// =====================================================
// FUZZY LOGIC 
// =====================================================
typedef struct
{
    float a, b, c, d;
} TrapMF;
typedef enum
{
    CHOL_BAIK = 0,
    CHOL_WASPADA = 1,
    CHOL_BAHAYA = 2
} CholClass;

static const TrapMF SPO2_MF[4] PROGMEM = {
    {0, 0, 75, 80}, {75, 80, 88, 90}, {85, 90, 94, 95}, {92, 95, 100, 100}};
static const TrapMF BPM_MF[4] PROGMEM = {
    {50, 50, 60, 70}, {60, 70, 80, 90}, {80, 90, 100, 110}, {100, 110, 120, 120}};
static const TrapMF CHOL_MF[3] PROGMEM = {
    {150, 150, 180, 200}, {190, 200, 230, 240}, {230, 240, 250, 250}};
static const uint8_t RULE_TABLE[4][4] PROGMEM = {
    {CHOL_BAHAYA, CHOL_BAHAYA, CHOL_BAHAYA, CHOL_BAHAYA},
    {CHOL_BAHAYA, CHOL_BAHAYA, CHOL_BAHAYA, CHOL_BAHAYA},
    {CHOL_WASPADA, CHOL_WASPADA, CHOL_BAHAYA, CHOL_BAHAYA},
    {CHOL_WASPADA, CHOL_BAIK, CHOL_WASPADA, CHOL_BAHAYA}};

static TrapMF read_trap(const TrapMF *base, uint8_t idx)
{
    TrapMF out;
    memcpy_P(&out, &base[idx], sizeof(TrapMF));
    return out;
}

static float trap_mf(float x, const TrapMF *mf)
{
    if (x <= mf->a)
        return (mf->a == mf->b) ? 1.0f : 0.0f;
    if (x >= mf->d)
        return (mf->c == mf->d) ? 1.0f : 0.0f;
    if (x >= mf->b && x <= mf->c)
        return 1.0f;
    if (x > mf->a && x < mf->b)
    {
        float den = mf->b - mf->a;
        return (den == 0.0f) ? 1.0f : (x - mf->a) / den;
    }
    float den = mf->d - mf->c;
    return (den == 0.0f) ? 1.0f : (mf->d - x) / den;
}

static float defuzzify_centroid(const float out[3])
{
    float num = 0.0f, den = 0.0f;
    TrapMF mB = read_trap(CHOL_MF, CHOL_BAIK);
    TrapMF mW = read_trap(CHOL_MF, CHOL_WASPADA);
    TrapMF mBh = read_trap(CHOL_MF, CHOL_BAHAYA);
    for (int16_t x = 150; x <= 250; x++)
    {
        float fx = (float)x;
        float a = out[CHOL_BAIK] < trap_mf(fx, &mB) ? out[CHOL_BAIK] : trap_mf(fx, &mB);
        float b = out[CHOL_WASPADA] < trap_mf(fx, &mW) ? out[CHOL_WASPADA] : trap_mf(fx, &mW);
        float c = out[CHOL_BAHAYA] < trap_mf(fx, &mBh) ? out[CHOL_BAHAYA] : trap_mf(fx, &mBh);
        float m = a > b ? a : b;
        if (c > m)
            m = c;
        num += fx * m;
        den += m;
    }
    return (den < 0.0001f) ? 0.0f : num / den;
}

static uint8_t classify_from_crisp(float crisp)
{
    TrapMF mB = read_trap(CHOL_MF, CHOL_BAIK);
    TrapMF mW = read_trap(CHOL_MF, CHOL_WASPADA);
    TrapMF mBh = read_trap(CHOL_MF, CHOL_BAHAYA);
    float b = trap_mf(crisp, &mB);
    float w = trap_mf(crisp, &mW);
    float bh = trap_mf(crisp, &mBh);
    if (bh >= w && bh >= b)
        return CHOL_BAHAYA;
    if (w >= b)
        return CHOL_WASPADA;
    return CHOL_BAIK;
}

static float estimate_cholesterol(float spo2, float bpm, uint8_t *kelas)
{
    float muSpo2[4], muBpm[4];
    float outStr[3] = {0.0f, 0.0f, 0.0f};
    for (uint8_t i = 0; i < 4; i++)
    {
        TrapMF ms = read_trap(SPO2_MF, i);
        TrapMF mb = read_trap(BPM_MF, i);
        muSpo2[i] = trap_mf(spo2, &ms);
        muBpm[i] = trap_mf(bpm, &mb);
    }
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            float fs = muSpo2[i] < muBpm[j] ? muSpo2[i] : muBpm[j];
            uint8_t cls = pgm_read_byte(&RULE_TABLE[i][j]);
            if (fs > outStr[cls])
                outStr[cls] = fs;
        }
    }
    float crisp = defuzzify_centroid(outStr);
    if (crisp <= 0.0f)
    {
        *kelas = CHOL_BAIK;
        return 0.0f;
    }
    *kelas = classify_from_crisp(crisp);
    return crisp;
}

static const char *class_to_text(uint8_t cls)
{
    if (cls == CHOL_BAIK)
        return "baik";
    if (cls == CHOL_WASPADA)
        return "waspada";
    return "bahaya";
}

/*
 * Rentang kolesterol referensi klinis:
 *   < 200 mg/dL  → Normal / Baik
 *   200-239       → Waspada (Borderline)
 *   >= 240        → Bahaya (Tinggi)
 * Fungsi ini mengembalikan label rentang berdasarkan nilai crisp
 */
static const char *chol_range_text(float chol)
{
    if (chol < 200.0f)
        return "<200 Normal";
    if (chol < 240.0f)
        return "200-239 Border";
    return ">=240 Tinggi";
}

// =====================================================
// OLED DISPLAY
// =====================================================
static void oled_show_info(const char *l1, const char *l2)
{
    if (!oledReady)
        return;
    oled_clear();
    fb_draw_str(0, 0, l1);
    fb_draw_str(0, 2, l2);
    oled_flush();
}

/*
 * Layout OLED 128x32 (4 baris × 8px):
 * Baris 0 (page 0): BPM:XX  SpO2:XX%
 * Baris 1 (page 1): Kol: XXX mg/dL
 * Baris 2 (page 2): Sts: baik/waspada/bahaya
 * Baris 3 (page 3): <200 Normal | 200-239 | >=240
 */
static void oled_show_data(float bpm_avg, float spo2,
                           uint8_t vSpo2, uint8_t vBpm,
                           float chol, uint8_t fclass,
                           uint8_t finger)
{
    if (!oledReady)
        return;
    oled_clear();

    static char buf[22];
    static char tmp[8];

    // --- Baris 0: BPM & SpO2 dalam satu baris ---
    // "BPM:73 SpO2:98%"
    buf[0] = '\0';
    str_cat(buf, "BPM:");
    i32_to_str((int32_t)(bpm_avg + 0.5f), tmp);
    str_cat(buf, tmp);
    str_cat(buf, " SpO2:");
    i32_to_str((int32_t)(spo2 + 0.5f), tmp);
    str_cat(buf, tmp);
    str_cat(buf, "%");
    fb_draw_str(0, 0, buf);

    if (!finger)
    {
        fb_draw_str(0, 1, "Pasang jari...");
        fb_draw_str(0, 2, "");
        fb_draw_str(0, 3, "");
        oled_flush();
        return;
    }

    // --- Baris 1: Kolesterol ---
    buf[0] = '\0';
    if (vSpo2 && vBpm && chol > 0.0f)
    {
        str_cat(buf, "Kol:");
        i32_to_str((int32_t)(chol + 0.5f), tmp);
        str_cat(buf, tmp);
        str_cat(buf, " mg/dL");
    }
    else
    {
        str_cat(buf, "Kol: menunggu..");
    }
    fb_draw_str(0, 1, buf);

    // --- Baris 2: Status fuzzy ---
    buf[0] = '\0';
    str_cat(buf, "Sts:");
    if (vSpo2 && vBpm && chol > 0.0f)
        str_cat(buf, class_to_text(fclass));
    else
        str_cat(buf, "menunggu");
    fb_draw_str(0, 2, buf);

    // --- Baris 3: Rentang kolesterol klinis ---
    buf[0] = '\0';
    if (vSpo2 && vBpm && chol > 0.0f)
        str_cat(buf, chol_range_text(chol));
    else
        str_cat(buf, "----------------");
    fb_draw_str(0, 3, buf);

    oled_flush();
}

// =====================================================
// MAIN
// =====================================================
int main(void)
{
    timer0_init();
    sei();

    uart_init();
    delay_ms(100);

    UART_P("\r\n=====================================\r\n");
    UART_P(" MAX30100 FUZZY + OLED BARE-METAL v2\r\n");
    UART_P("=====================================\r\n");
    UART_P("ATmega328P @ 16MHz, UART 9600bps U2X\r\n");
    UART_P("Fix: SpO2 formula, stabilisasi, range kolesterol\r\n\r\n");

    i2c_init();
    delay_ms(10);

    if (oled_init())
    {
        oledReady = 1;
        oled_show_info("Deteksi Kolesterol", "Init sensor...");
        UART_P("OLED: OK\r\n");
    }
    else
    {
        UART_P("WARN: OLED gagal\r\n");
    }

    if (!max30100_init())
    {
        UART_P("ERROR: MAX30100 tidak ditemukan!\r\n");
        UART_P("Cek wiring dan pull-up resistor.\r\n");
        oled_show_info("ERROR MAX30100", "Cek wiring");
        while (1)
        {
        }
    }

    UART_P("Sensor OK. Letakkan jari pada sensor...\r\n\r\n");
    UART_P("Format: BPM Raw | BPM Avg | SpO2 | Kolesterol | Status | Rentang\r\n");
    UART_P("----------------------------------------------------------------------\r\n");
    oled_show_info("Sensor siap", "Letakkan jari");

    uint32_t lastReport = millis_get();
    uint32_t lastOledUpd = millis_get();

#define REPORT_MS 1000U
#define OLED_MS 1500U

    for (;;)
    {
        max30100_update();

        uint32_t now = millis_get();

        if (now - lastReport >= REPORT_MS)
        {
            lastReport = now;

            float bpm = max30100_get_bpm();
            float spo2 = max30100_get_spo2();
            uint8_t fing = max30100_finger_on();

            if (bpm > 40.0f && bpm < 180.0f)
                avgBPM = calc_avg_bpm(bpm);

            uint8_t vSpo2 = (spo2 >= 70.0f && spo2 <= 100.0f);
            uint8_t vBpm = (avgBPM >= 50.0f && avgBPM <= 120.0f);

            uint8_t fuzzyClass = CHOL_BAIK;
            float chol = 0.0f;

            if (vSpo2 && vBpm)
                chol = estimate_cholesterol(spo2, avgBPM, &fuzzyClass);

            // ----- Serial log -----
            UART_P("BPM Raw: ");
            uart_print_float1(bpm);
            UART_P(" | Avg: ");
            uart_print_float1(avgBPM);
            UART_P(" | SpO2: ");
            uart_print_float1(spo2);
            uart_putc('%');
            UART_P(" | Jari: ");
            uart_puts(fing ? "ON" : "OFF");

            if (vSpo2 && vBpm && chol > 0.0f)
            {
                UART_P(" | Kol: ");
                uart_print_float1(chol);
                UART_P(" mg/dL");
                UART_P(" | Sts: ");
                uart_puts(class_to_text(fuzzyClass));
                UART_P(" | Rentang: ");
                uart_puts(chol_range_text(chol));
            }
            else
            {
                UART_P(" | Kol: data tidak valid");
            }
            UART_P("\r\n");

            // ----- OLED update -----
            if (now - lastOledUpd >= OLED_MS)
            {
                lastOledUpd = now;
                oled_show_data(avgBPM, spo2, vSpo2, vBpm, chol, fuzzyClass, fing);
            }
        }
    }

    return 0;
}