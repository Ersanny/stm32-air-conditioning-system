# STM32F103 Air Conditioning System Panel — OLED UI
- Youtube video: https://www.youtube.com/watch?v=fZP-vNPlWaw

A bare-metal climate control interface built on **STM32F103C8T6** ("Blue Pill") driving a **0.96" SSD1306 OLED** over I²C. The project demonstrates a complete, register-level display driver and UI stack — no HAL, no LL, no graphics libraries.

> Personal project by **[Ersanny](https://github.com/Ersanny)** — built as a hands-on exercise in low-level embedded development, written entirely from scratch.

---

## What this project does

The OLED renders a real-time climate control dashboard:

- **Large central number** — the set temperature (target)
- **Bottom-left** — fan speed percentage
- **Bottom-right** — current room temperature
- **Top-left icon** — automatically switches between **cooling / heating / standby** based on the difference between set and room temperature
- **Boot-up animation** — GitHub logo slides in from the left, "Ersanny" text slides in from the right, both rest for 1.5 s, then slide out → main UI starts

Currently the temperature and fan values are hard-coded; the wiring is in place for real sensor and button input to be plugged in.

---

## Hardware

| Component                | Detail                                       |
|--------------------------|----------------------------------------------|
| MCU                      | STM32F103C8T6 (Cortex-M3, 64 KB Flash, 20 KB RAM) |
| Display                  | SSD1306 0.96" OLED, 128 × 64, I²C            |
| Clock                    | 8 MHz HSI (no external crystal)              |
| I²C lines                | PB6 (SCL), PB7 (SDA), 100 kHz                |
| Programmer               | ST-Link V2 (Keil μVision)                    |

---

## Architecture

The codebase is split into three layers, each isolated from the others by a thin API.

```
┌─────────────────────────────────────────────────┐
│ main.c          ── application loop             │
│                    SSD1306_DrawUI(set,room,fan) │
├─────────────────────────────────────────────────┤
│ ssd1306.c/.h    ── display driver + UI render   │
│                    frame buffer, fonts, layers  │
├─────────────────────────────────────────────────┤
│ i2c.c/.h        ── register-level I²C with      │
│                    timeout-protected primitives │
└─────────────────────────────────────────────────┘
```

### Frame buffer rendering

The driver keeps a **1024-byte (8 × 128) RAM mirror** of the screen. Every drawing function writes to RAM only; `SSD1306_Flush()` then transmits the entire buffer in **8 I²C transactions** (one per page).

Why this matters: a naive approach issuing one I²C Start/Stop per byte would need ~1024 transactions per frame. The buffered design is roughly **128× faster** and lets you safely composite multiple layers (background → icon → digits → small text) without flicker.

### Layered UI composition

`SSD1306_DrawUI()` builds the screen as four ordered layers:

1. **Background bitmap** — selected from `ui_sogutma / ui_isitma / ui_bekleme` based on auto-detected mode
2. **Large digits** — the set temperature, centered
3. **Bottom-right number** — current room temperature
4. **Bottom-left number** — fan percentage

Layers are drawn to the buffer in order, then sent to the OLED in a single flush.

### Error propagation chain

Every I²C primitive returns `I2C_Status` (OK or TIMEOUT). The status flows up through the call chain:

```
I2C_WriteByte → SSD1306_Flush → SSD1306_DrawUI → main
```

If the bus hangs (e.g. OLED disconnected), each `while (!flag)` loop bails out after ~10 ms and the failure surfaces all the way to `main()`, which can take recovery action. The MCU never deadlocks waiting on absent hardware.

### Auto mode detection

The application doesn't pass a mode to the driver — `DrawUI` derives it from the inputs:

```c
if      (room_temp > set_temp) mod = COOLING;
else if (room_temp < set_temp) mod = HEATING;
else                            mod = STANDBY;
```

One source of truth, no risk of caller mismatch.

---

## Key functions

| Function                  | Responsibility                                       |
|---------------------------|------------------------------------------------------|
| `I2C_Init()`              | Configure GPIOB pins, set I²C1 to 100 kHz standard mode |
| `I2C_WriteByte()`         | Send one byte with TXE/BTF timeout protection        |
| `SSD1306_Init()`          | 100 ms power-up delay, then send the 28-command init sequence |
| `SSD1306_Flush()`         | Push the entire frame buffer in 8 page transactions  |
| `SSD1306_DrawBigDigit()`  | Render an 18 × 32 px digit into the buffer at (x, page) |
| `SSD1306_WriteChar()`     | Render a 10 × 12 px digit across two pages           |
| `SSD1306_DrawUI()`        | Compose the four-layer climate UI and flush          |
| `SSD1306_Splash()`        | Run the boot animation (slide-in / hold / slide-out) |

---

## Notable design decisions

**100 ms power-up delay in `Init`.** Without it, the SSD1306 sometimes appears 180° rotated after power-on because its charge pump isn't ready when the orientation commands arrive. A 100 ms busy-wait at the top of `SSD1306_Init` fixed it deterministically.

**Two fonts, both repurposed from Adafruit GFX.** Public font generators emit GFX-format data (row-by-row, MSB-packed). The SSD1306 wants column-by-column, page-packed data. A small Python script transposes between the two formats — see `tools/` if reproducing.

**Fonts are digit-only.** Both `fontOS40` (large) and `fontOS16` (small) hold only `0`–`9`. The UI never needs letters at runtime (text on screen comes from pre-rendered bitmaps), so saving Flash by stripping the alphabet was worth it.

**Splash assets are `static`.** `github_logo`, `ersanny_text`, `draw_logo`, `draw_ersanny`, and `splash_delay_ms` are all file-local — they can't leak into other parts of the codebase.

**Boot-up "Reset and Run".** Keil's flash download settings include *Reset and Run* so the chip starts the new code automatically after upload. Without it the MCU sat in an undefined state until manually reset.

---

## Resource footprint

| Resource | Used   | Total | Utilization |
|----------|--------|-------|-------------|
| Flash    | ~5.5 KB | 64 KB | ~8.6%       |
| RAM      | ~1 KB   | 20 KB | ~5%         |

Plenty of headroom for sensors, buttons, an RTOS, persistent storage, or anything else.

---

## Tools used

- **[Keil μVision](https://www.keil.com/)** — IDE, compiler, ST-Link flash & debug
- **[oleddisplay.squix.ch](https://oleddisplay.squix.ch/)** — Font generation (Adafruit GFX format)
- **[image2cpp](https://javl.github.io/image2cpp/)** — Bitmap-to-C-array conversion for UI icons and GitHub logo
- **[piskelapp](https://www.piskelapp.com/p/create/sprite/)** — Custom pixel-art design (logo)
- **Python** — Custom scripts for GFX → SSD1306 page-format font transposition
- **STM32F10x reference manual & SSD1306 datasheet** — register- and command-level documentation

---

## File layout

```
.
├── main.c          # Entry point, application loop
├── i2c.h / i2c.c   # Register-level I²C driver
├── ssd1306.h       # Public API
└── ssd1306.c       # Driver: frame buffer, fonts, UI render, splash
```

## License

Personal/educational project. Use, fork, and learn from it freely.

— **Ersanny**
