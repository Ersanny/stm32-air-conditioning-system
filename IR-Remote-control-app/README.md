# IR LED Controller

Control 3 independent LEDs using an IR remote control on the **STM32F103C8T6 (Blue Pill)**. Fully bare-metal — all logic is implemented at the register level using direct CMSIS access.

---

## 🛠️ Hardware

| Component | Model | Pin |
|---|---|---|
| Microcontroller | STM32F103C8T6 (Blue Pill) | — |
| IR Receiver | VS1838B | PA6 |
| LED 1 | Red | PA1 |
| LED 2 | Green | PA2 |
| LED 3 | Blue | PA3 |

**VS1838B wiring:**
```
VS1838B (flat face toward you)
 ┌─────┐
 │     │
 └──┬──┘
   |||
   ││└── Pin 3 → 3.3V
   │└─── Pin 2 → GND
   └──── Pin 1 → PA6

+ 100nF capacitor between VCC and GND
```

---

## 📁 File Structure

```
ir-led-controller/
├── Src/
│   ├── main.c          # Entry point
│   ├── arayuz.c        # LED & button GPIO drivers
│   └── ir_remote.c     # NEC protocol decoder
└── Inc/
    ├── arayuz.h
    └── ir_remote.h
```

---

## 📡 NEC Protocol

The VS1838B receives 38kHz modulated IR signals. The NEC frame structure:

```
[9ms LOW] [4.5ms HIGH] [Address 8bit] [~Address 8bit] [Command 8bit] [~Command 8bit]

Bit 0 → 562µs LOW + ~562µs  HIGH  (~1.1ms  total)
Bit 1 → 562µs LOW + ~1687µs HIGH  (~2.25ms total)
```

Validation: `Command XOR ~Command` must equal `0xFF`. Frames that fail this check are discarded.

---

## ⚙️ Key Registers

| Register | Purpose |
|---|---|
| `RCC->APB2ENR` | Enable GPIOA clock |
| `GPIOA->CRL` | PA1, PA2, PA3 → push-pull output / PA6 → floating input |
| `GPIOA->ODR` | LED write |
| `GPIOA->IDR` | IR pin read |
| `SysTick` | Microsecond timing for pulse measurement |

---

## 💻 Development Environment

- **IDE:** Keil MDK (µVision)
- **Target:** STM32F103C8T6 — ARM Cortex-M3
- **Programming style:** Bare-metal (direct register access)
- **Debugger:** ST-Link V2

---

## 📖 References

- [STM32F103 Reference Manual — RM0008](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- *Making Embedded Systems* — Elecia White
