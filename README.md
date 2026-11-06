# 🚗 Wireless Gesture-Controlled 4WD Robot Car

A real-time wireless **Gesture-Controlled 4-Wheel Drive (4WD) Robot Car** powered by an **Arduino Nano** handheld transmitter (with an **MPU-6050** 6-axis IMU) and an **Arduino Uno** onboard receiver over a **2.4 GHz nRF24L01+** RF link.

Tilting the handheld remote in 3D space (Forward, Backward, Left, or Right) translates hand gestures into low-latency directional commands at **25 Hz**, complete with automatic packet acknowledgment (`ACK`) and a **350 ms hardware failsafe** that halts all motors immediately if wireless signal is lost.

---

## ✨ Key Features

- **True Horizontal Neutral Reference (`0°`)**: Uses symmetric 3D accelerometer vector projection (`atan2` over orthogonal gravity components) so holding the remote flat (`< 15°`) always keeps the car stationary (`'S'`), regardless of power-on orientation.
- **Dominant-Axis Gesture Selection**: Compares `|Pitch|` vs. `|Roll|` with exponential smoothing (`α = 0.5`) so Forward/Backward and Left/Right gestures never interfere with one another.
- **SPI Master Bus Protection (Pin 10 Safe)**: Initializes the hardware SPI bus and nRF24L01+ transceiver before driving motor outputs on Digital Pin 10 (`SS`), preventing AVR SPI slave-mode lockups on cold battery boots.
- **Cold-Boot & Brownout Resilient**: Non-blocking startup retry loops and I2C bus timeout protection (`Wire.setWireTimeout`) allow both boards to be powered on in any order from batteries or switches.
- **Automatic Failsafe Stop**: If the transmitter is powered off or moves out of range for more than `350 ms`, the receiver automatically cuts power to all four motors.

---

## 🛠️ Hardware Components

| Component | Quantity | Role |
| :--- | :---: | :--- |
| **Arduino Uno** | 1 | Onboard Robot Receiver & Motor Controller |
| **Arduino Nano** | 1 | Handheld Gesture Transmitter |
| **nRF24L01+ (2.4 GHz RF Module)** | 2 | Bidirectional Wireless Transceiver Link (`Channel 76`, `1 Mbps`) |
| **MPU-6050 (GY-521)** | 1 | 3-Axis Accelerometer & Gyroscope (I2C Gesture Sensor) |
| **Dual H-Bridge Motor Driver (L298N / L293D)** | 1–2 | Drives the 4 DC Gear Motors (`FL`, `BL`, `FR`, `BR`) |
| **DC Gear Motors + Wheels + 4WD Chassis** | 4 | 4-Wheel Differential Drive Platform |

---

## 🔌 Pinout & Wiring

### 1. Transmitter — Arduino Nano (`transmitter_nano.ino`)

| Module | Pin on Module | Arduino Nano Pin | Notes |
| :--- | :--- | :--- | :--- |
| **MPU-6050** | `VCC` | `5V` (or `3.3V`) | GY-521 onboard LDO supports 5V |
| **MPU-6050** | `GND` | `GND` | Common ground |
| **MPU-6050** | `SDA` | `A4` | Hardware I2C Data |
| **MPU-6050** | `SCL` | `A5` | Hardware I2C Clock |
| **nRF24L01+** | `VCC` | `3.3V` | **3.3V only** (10 µF decoupling capacitor recommended) |
| **nRF24L01+** | `GND` | `GND` | Common ground |
| **nRF24L01+** | `CE` | `D9` | Chip Enable |
| **nRF24L01+** | `CSN` | `D10` | SPI Chip Select |
| **nRF24L01+** | `MOSI` | `D11` | Hardware SPI MOSI |
| **nRF24L01+** | `MISO` | `D12` | Hardware SPI MISO |
| **nRF24L01+** | `SCK` | `D13` | Hardware SPI Clock |

---

### 2. Receiver — Arduino Uno (`receiver_uno.ino`)

| Subsystem | Function | Arduino Uno Pin |
| :--- | :--- | :--- |
| **nRF24L01+** | `CE` / `CSN` | `A0` / `A1` |
| **nRF24L01+** | `MOSI` / `MISO` / `SCK` | `D11` / `D12` / `D13` |
| **nRF24L01+** | `VCC` / `GND` | `3.3V` / `GND` |
| **Front Left Motor (`FL`)** | `FL1` / `FL2` | `D3` / `D4` |
| **Back Left Motor (`BL`)** | `BL1` / `BL2` | `D5` / `D6` |
| **Front Right Motor (`FR`)** | `FR1` / `FR2` | `D7` / `D8` |
| **Back Right Motor (`BR`)** | `BR1` / `BR2` | `D9` / `D10` |

---

## 🎮 Gesture Control Mapping

$$
\text{Pitch} = \text{atan2}\!\left(a_x, \sqrt{a_y^2 + a_z^2}\right) \cdot \frac{180}{\pi}, \qquad
\text{Roll} = \text{atan2}\!\left(a_y, \sqrt{a_x^2 + a_z^2}\right) \cdot \frac{180}{\pi}
$$

| Hand Gesture | Angle Condition | Radio Command | Motor Action |
| :--- | :--- | :---: | :--- |
| **Flat / Horizontal** | $|\text{Pitch}| \le 15^\circ \;\text{and}\; |\text{Roll}| \le 15^\circ$ | `'S'` | **Stop** (All motors `LOW`) |
| **Tilt Forward** | $\text{Pitch} > +15^\circ$ | `'F'` | **Drive Forward** (All 4 wheels forward) |
| **Tilt Backward** | $\text{Pitch} < -15^\circ$ | `'B'` | **Drive Backward** (All 4 wheels reverse) |
| **Tilt Left** | $\text{Roll} > +15^\circ$ | `'L'` | **Turn Left** (Left wheels reverse, Right wheels forward) |
| **Tilt Right** | $\text{Roll} < -15^\circ$ | `'R'` | **Turn Right** (Left wheels forward, Right wheels reverse) |

---

## 📂 Repository Structure

```text
gesture-controlled-car/
├── receiver_uno.ino       # Arduino Uno onboard car receiver & motor control firmware
├── transmitter_nano.ino   # Arduino Nano MPU-6050 gesture transmitter firmware
└── README.md              # Project documentation, wiring diagrams, and gesture math
```

---

## 🚀 Getting Started

1. **Install Required Arduino Library**:
   - Install **RF24** by TMRh20 via the Arduino Library Manager or Arduino CLI:
     ```bash
     arduino-cli lib install "RF24"
     ```
2. **Flash the Receiver (Arduino Uno)**:
   - Open `receiver_uno.ino` and upload to your **Arduino Uno** (`arduino:avr:uno`).
3. **Flash the Transmitter (Arduino Nano)**:
   - Open `transmitter_nano.ino` and upload to your **Arduino Nano** (`arduino:avr:nano`).
4. **Drive**:
   - Power on both boards in any order. Hold the remote flat to keep the car stopped, and tilt in any direction to steer!
