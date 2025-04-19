# Air-quality
```markdown
# 🌬️ MQ-135 Air Quality Monitoring System

[![Language: C](https://img.shields.io/badge/language-C-brightgreen.svg)]()
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

## 📖 Overview

This project implements an **air quality monitoring system** using the PIC microcontroller (e.g., PIC16F877A) with an MQ-135 gas sensor. It displays real-time PPM (parts per million) values on a 16×2 LCD and uses LEDs and a buzzer to indicate safe, warning, and danger levels.

---

## 📋 Table of Contents
1. [Features](#-features)
2. [Hardware Setup](#-hardware-setup)
3. [Software Setup](#-software-setup)
4. [Code Structure](#-code-structure)
5. [Configuration Bits](#-configuration-bits)
6. [Usage](#-usage)
7. [Customization](#-customization)
8. [License](#-license)

---

## ✨ Features

- **Real-Time Measurement**: Reads analog output of MQ-135 sensor and converts to PPM.
- **Digital Filtering**: Weighted moving average for stable readings.
- **LCD Display**: Shows "PPM: XX.X" on the second line and status on the first.
- **LED Indicators**:
  - Green (Safe) when PPM < 40
  - Yellow (Warning) when 40 ≤ PPM < 80
  - Red (Danger) when PPM ≥ 80
- **Audible Alerts**: Buzzer pulses at warning and rapid pulses at danger levels.
- **Modular Code**: Clear separation of ADC, LCD, and alert logic.

---

## 🧰 Hardware Setup

| Component         | PIC Pin | Description                  |
|-------------------|---------|------------------------------|
| MQ-135 Sensor     | RA5/AN4 | Analog input (ADC channel 4) |
| LCD RS            | RC1     | Register Select              |
| LCD RW            | RC0     | Read/Write (tied to write)   |
| LCD EN            | RC2     | Enable signal                |
| LCD Data D0–D7    | RD0–RD7 | 8-bit data bus               |
| LED (Safe/Green)  | RB0     | Indicates safe level         |
| LED (Warn/Yellow) | RB1     | Indicates warning level      |
| LED (Danger/Red)  | RB2     | Indicates danger level       |
| Buzzer            | RB3     | Audible alarm                |
| V<sub>cc</sub> (5V)| —       | Power supply                 |
| GND               | —       | Ground                       |

---

## 💻 Software Setup

1. **IDE & Compiler**:
   - MPLAB X IDE
   - XC8 Compiler
2. **Clone Repository**:
   ```bash
   cd air-quality-monitor
   ```
3. **Open Project**:
   - Launch MPLAB X, select *Open Project*, and load the `.X` file.
4. **Configuration Bits**:
   - Ensure oscillator, watchdog, and BOR settings match your hardware.
   - See the [Configuration Bits](#-configuration-bits) section.
5. **Build & Program**:
   - Click *Build* to compile.
   - Connect your PIC programmer (e.g., PICkit3) and click *Make and Program Device*.

---

## 🗂️ Code Structure

```text
├── src/
│   ├── main.c             # Core logic: ADC, filtering, PPM conversion, alerts
│   ├── lcd.c              # LCD driver functions (Cmd, Data, Send2Lcd)
│   ├── lcd.h              # LCD driver header
│   ├── adc.c              # ADC initialization and read functions
│   ├── adc.h              # ADC header
│   ├── utils.c            # Delay and helper functions
│   └── utils.h            # Utility header
├── air_quality.X         # MPLAB X project file
├── LICENSE               # MIT License
└── README.md             # Project documentation
```

---

## ⚙️ Configuration Bits

Uncomment and adjust the following in `main.c` as needed:
```c
#pragma config FOSC = HS      // HS oscillator (e.g., 20 MHz)
#pragma config WDTE = OFF     // Watchdog Timer disabled
#pragma config PWRTE = ON     // Power-up Timer enabled
#pragma config BOREN = ON     // Brown-out Reset enabled
#pragma config LVP = OFF      // Low-Voltage Programming disabled
#pragma config CPD = OFF      // Data EEPROM Code Protection off
#pragma config WRT = OFF      // Flash Program Memory Write protection off
#pragma config CP = OFF       // Code Protection off
```

---

## 🚦 Usage

1. **Power On**: System displays "AIR QUALITY" then "Initializing..." on LCD.
2. **Warm-Up**: Wait ~500 ms for sensor stabilization.
3. **Monitoring**: Reads and displays PPM every 500 ms.
4. **Alerts**:
   - **Safe** (<40 PPM): Green LED lit.
   - **Warning** (40–80 PPM): Yellow LED + intermittent buzzer and "AIR QUAL:WARNING".
   - **Danger** (≥80 PPM): Red LED + rapid buzzer and "AIR QUAL:DANGER".

---

## 🔧 Customization

- **Thresholds**: Modify `SAFE_THRESHOLD` and `WARNING_THRESHOLD` in `main.c`.
- **Filter Strength**: Adjust weighted average formula in the main loop.
- **Display Format**: Change `display_ppm()` logic for different precision.

---

## 📄 License

This project is licensed under the **MIT License**. See [LICENSE](LICENSE) for details.

