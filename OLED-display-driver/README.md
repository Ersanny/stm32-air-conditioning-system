# OLED Display Driver

Bare-metal SSD1306 OLED display driver for the **STM32F103C8T6 (Blue Pill)**. Communicates via I2C protocol using direct register access — no HAL, no external libraries.

---

## 🛠️ Hardware

| Component | Model | Detail |
|---|---|---|
| Microcontroller | STM32F103C8T6 (Blue Pill) | ARM Cortex-M3, 8MHz |
| Display | 0.96" OLED | 128×64 pixels, monochrome |
| Display Driver | SSD1306 | I2C interface |
| I2C Address | 0x3C | (8-bit: 0x78) |

**Wiring:**

```
STM32F103        OLED Module
─────────        ───────────
PB6 (SCL) ──── SCL
PB7 (SDA) ──── SDA
3.3V      ──── VCC
GND       ──── GND

Pull-up resistors (mandatory):
  PB6 ── [4.7kΩ] ── 3.3V
  PB7 ── [4.7kΩ] ── 3.3V
```

> ⚠️ I2C requires external pull-up resistors on SCL and SDA lines. Without them, the bus cannot reach HIGH state and communication will fail.

---

## 📁 File Structure

```
oled-display/
├── Src/
│   ├── main.c        # Entry point — display test
│   ├── i2c.c         # I2C1 hardware driver (register level)
│   └── ssd1306.c     # SSD1306 OLED driver + 5x8 font table
└── Inc/
    ├── i2c.h         # I2C function prototypes
    └── ssd1306.h     # SSD1306 function prototypes
```

---

## 🏗️ Architecture

The project follows a layered architecture — each layer has a single responsibility:

```
main.c          →  "What to display"
    └── ssd1306.c   →  "How to control the OLED"
          └── i2c.c     →  "How to physically send data"
                └── STM32 I2C Hardware
```

This separation means:
- `i2c.c` knows nothing about OLED — it just sends bytes
- `ssd1306.c` knows nothing about I2C details — it just calls `I2C_WriteByte()`
- `main.c` knows nothing about pixels — it just calls `SSD1306_WriteString()`

---

## 📡 I2C Protocol

I2C (Inter-Integrated Circuit) is a 2-wire serial communication protocol. Each device on the bus has a unique 7-bit address.

**Message structure:**
```
[START] [Address + W] [ACK] [Data byte] [ACK] ... [STOP]

START  → Signal that communication is beginning
Address → Which device are we talking to? (0x3C for OLED)
W bit  → 0 = write, 1 = read
ACK    → Receiver confirms receipt
STOP   → Communication is complete
```

**Key registers used:**

| Register | Purpose |
|---|---|
| `RCC->APB1ENR` | Enable I2C1 clock (bit 21) |
| `RCC->APB2ENR` | Enable GPIOB clock (bit 3) |
| `GPIOB->CRL` | PB6, PB7 → AF Open-Drain, 50MHz |
| `I2C1->CR2` | Peripheral clock frequency (8MHz) |
| `I2C1->CCR` | Clock speed (CCR=40 → 100kHz) |
| `I2C1->TRISE` | Max rise time (TRISE=9 for 100kHz) |
| `I2C1->CR1` | Enable/disable I2C, START, STOP |
| `I2C1->DR` | Data register — read/write data |
| `I2C1->SR1` | Status register — flags (SB, ADDR, TXE, BTF) |
| `I2C1->SR2` | Status register 2 — used to clear ADDR flag |

**Why Open-Drain GPIO?**
```
Push-pull → pin can drive HIGH or LOW actively
Open-drain → pin can only pull LOW; HIGH requires external pull-up

I2C needs open-drain because multiple devices share the same bus.
If two devices both try to drive the line simultaneously,
open-drain prevents short circuits — only pull-down wins.
```

**Clock speed calculation:**
```
Standard Mode I2C: CCR = fPCLK1 / (2 × fI2C)
CCR = 8,000,000 / (2 × 100,000) = 40

Rise time: TRISE = fPCLK1(MHz) + 1 = 8 + 1 = 9
```

**Flag polling (no interrupts):**
```
After START   → poll SR1 bit 0 (SB flag)   until set
After address → poll SR1 bit 1 (ADDR flag) until set
Before data   → poll SR1 bit 7 (TXE flag)  until set
After data    → poll SR1 bit 2 (BTF flag)  until set
```

---

## 🖥️ SSD1306 OLED Driver

The SSD1306 is the display controller chip embedded inside the OLED module. It manages the 128×64 pixel matrix and accepts commands/data via I2C.

**Memory layout:**
```
128 columns × 64 rows = 8192 pixels
Each pixel = 1 bit (on/off)
Total = 1024 bytes of display RAM inside SSD1306

Organized as 8 pages × 128 columns:
  Page 0 → rows 0–7
  Page 1 → rows 8–15
  ...
  Page 7 → rows 56–63
```

**Control byte:**
```
Every I2C message to SSD1306 starts with a control byte:
  0x00 → next byte is a COMMAND (configures the display)
  0x40 → next byte is DATA (pixel data written to RAM)
```

**Sending a command:**
```
START → 0x3C (address) → ACK → 0x00 (control: command) → ACK → cmd → ACK → STOP
```

**Sending pixel data:**
```
START → 0x3C (address) → ACK → 0x40 (control: data) → ACK → pixel_byte → ACK → STOP
```

---

## 🔤 Font System

Characters are rendered using a 5×8 pixel bitmap font stored as a lookup table.

```c
static const uint8_t font[][5] = { ... };
```

Each character is 5 bytes wide (5 columns), each byte represents 8 vertical pixels:

```
Example: 'A' = {0x7E, 0x11, 0x11, 0x11, 0x7E}

Column 0: 0x7E = 0 1 1 1 1 1 1 0
Column 1: 0x11 = 0 0 0 1 0 0 0 1
Column 2: 0x11 = 0 0 0 1 0 0 0 1
Column 3: 0x11 = 0 0 0 1 0 0 0 1
Column 4: 0x7E = 0 1 1 1 1 1 1 0

Rendered:
. ■ ■ ■ ■ ■ ■ .
. . . ■ . . . ■
. . . ■ . . . ■
. . . ■ . . . ■
. ■ ■ ■ ■ ■ ■ .
```

Character index in font table: `index = ASCII_code - 32`

---

## ⚙️ API Reference

### i2c.h

| Function | Description |
|---|---|
| `I2C_Init()` | Initialize I2C1 at 100kHz, configure PB6/PB7 |
| `I2C_Start()` | Generate START condition, wait for SB flag |
| `I2C_Stop()` | Generate STOP condition |
| `I2C_WriteAddress(addr)` | Send 7-bit address with write bit, wait for ADDR flag |
| `I2C_WriteByte(data)` | Send one byte, wait for TXE then BTF flag |

### ssd1306.h

| Function | Description |
|---|---|
| `SSD1306_Init()` | Run initialization command sequence, wake display |
| `SSD1306_Clear()` | Write 0x00 to all 1024 bytes of display RAM |
| `SSD1306_SetCursor(row, col)` | Set page (0–7) and column (0–127) |
| `SSD1306_WriteChar(c)` | Render one ASCII character using font table |
| `SSD1306_WriteString(str)` | Render a null-terminated string |

---

## 💻 Development Environment

- **IDE:** Keil MDK (µVision)
- **Target:** STM32F103C8T6 — ARM Cortex-M3
- **Programming style:** Bare-metal (direct register access via CMSIS)
- **Debugger/Programmer:** ST-Link V2
- **Clock:** 8MHz internal HSI oscillator

---

## 🗺️ Roadmap

This module is part of the larger STM32 Air Conditioning System project:

- [x] I2C1 bare-metal driver
- [x] SSD1306 OLED driver
- [x] 5×8 ASCII font rendering
- [ ] LM35 temperature reading via ADC
- [ ] Display live temperature data
- [ ] Full AC system integration

---

## 📖 References

- [STM32F103 Reference Manual — RM0008](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) — Section 26 (I2C), Section 9 (GPIO)
- [SSD1306 Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)
- *Making Embedded Systems* — Elecia White

---

## 👤 Author

**Ersan**  
Embedded Systems Developer  
Bare-metal programming on ARM Cortex-M series
