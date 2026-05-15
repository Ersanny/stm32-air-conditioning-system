# stm32-air-conditioning-system

A bare-metal air conditioning control system built on the **STM32F103C8T6 (Blue Pill)** microcontroller. All peripherals are driven at the register level — no HAL, no abstraction libraries.

> 🚧 This project is actively under development. New modules are added as each peripheral is implemented from scratch.

---

## 🛠️ Hardware

| Component | Model | Purpose |
|---|---|---|
| Microcontroller | STM32F103C8T6 (Blue Pill) | Main controller |
| IR Receiver | VS1838B | Remote control input |
| IR Remote | NEC Protocol | User input |
| Temperature Sensor | LM35 | Analog temperature reading |
| Display | 16x2 LCD (HD44780) | System status output |
| Motor | 5V DC Motor | Compressor simulation |
| Keypad | 4x4 Matrix | Manual user input |
| LEDs | — | Status indicators |

---

## 💻 Development Environment

- **IDE:** Keil MDK (µVision)
- **Target:** STM32F103C8T6 — ARM Cortex-M3
- **Programming style:** Bare-metal (direct register access via CMSIS)
- **Debugger/Programmer:** ST-Link V2

---

## 📁 Project Structure

```
stm32-air-conditioning-system/
│
├── ir-led-controller/          # IR remote → LED control (NEC decode)
│   ├── Src/
│   │   ├── main.c
│   │   ├── arayuz.c            # GPIO: LED & button drivers
│   │   └── ir_remote.c         # NEC protocol decoder
│   └── Inc/
│       ├── arayuz.h
│       └── ir_remote.h
│
├── pwm-motor-control/          # Coming soon
├── lm35-temperature/           # Coming soon
├── lcd-driver/                 # Coming soon
└── full-ac-system/             # Coming soon — full integration
```

---

## ✅ Modules

### 1. IR LED Controller — `ir-led-controller/`

Control 3 LEDs independently using an IR remote control.

**Implemented:**
- NEC protocol decoder — fully bare-metal, no timer peripheral (polling-based)
- GPIO output driver for 3 external LEDs (PA1, PA2, PA3)
- GPIO input configuration with pull-up (Button on PA0)
- Built-in LED driver (PC13, Active LOW)

**Key registers used:**

| Register | Purpose |
|---|---|
| `RCC->APB2ENR` | Enable GPIOA, GPIOC clocks |
| `GPIOx->CRL/CRH` | Pin mode configuration |
| `GPIOx->ODR` | Output write / pull direction |
| `GPIOx->IDR` | Input read |
| `SysTick` | Microsecond timing for NEC decode |

**NEC Protocol frame structure:**
```
[9ms LOW] [4.5ms HIGH] [Address 8bit] [~Address 8bit] [Command 8bit] [~Command 8bit]

Bit 0 → 562µs LOW + ~562µs  HIGH
Bit 1 → 562µs LOW + ~1687µs HIGH
```

---

## 🗺️ Roadmap

- [x] GPIO output — LED control
- [x] GPIO input — Button & IR receiver
- [x] NEC IR protocol decoder
- [ ] ADC — LM35 temperature reading
- [ ] TIM2 PWM — DC motor speed control
- [ ] LCD driver — 4-bit mode, HD44780
- [ ] 4x4 matrix keypad scanner
- [ ] Full system integration with state machine

---

## 📖 References

- [STM32F103 Reference Manual (RM0008)](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32F103C8T6 Datasheet](https://www.st.com/en/microcontrollers-microprocessors/stm32f103c8.html)
- *Making Embedded Systems* — Elecia White

---

## 👤 Author

**Ersan**  
Embedded Systems Developer  
Bare-metal programming on ARM Cortex-M series
