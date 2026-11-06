#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // Nano CE = D9, CSN = D10
const byte address[6] = "00001";

int mpuAddress = 0x68;

// Gesture angle threshold in degrees (Horizontal = 0 deg)
const float TILT_THRESHOLD = 15.0;

// Smoothed angles
float fPitch = 0.0;
float fRoll = 0.0;

unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 40; // 25 Hz

bool readRawMPU(int16_t &rx, int16_t &ry, int16_t &rz) {
  Wire.beginTransmission(mpuAddress);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return false;
  
  if (Wire.requestFrom(mpuAddress, 6, true) == 6) {
    rx = (int16_t)(Wire.read() << 8 | Wire.read());
    ry = (int16_t)(Wire.read() << 8 | Wire.read());
    rz = (int16_t)(Wire.read() << 8 | Wire.read());

    // Filter out I2C glitches
    if (rx == -32768 || ry == -32768 || rz == -32768) return false;
    if (rx == 0 && ry == 0 && rz == 0) return false;
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(9600);
  delay(500);

  Wire.begin();
  #if defined(WIRE_HAS_TIMEOUT)
  Wire.setWireTimeout(3000, true);
  #endif

  Serial.println("\n--- GESTURE TRANSMITTER (NANO) ---");

  // 1. Detect and wake MPU-6050
  Wire.beginTransmission(0x68);
  if (Wire.endTransmission() == 0) mpuAddress = 0x68;
  else mpuAddress = 0x69;

  for (int retry = 0; retry < 5; retry++) {
    Wire.beginTransmission(mpuAddress);
    Wire.write(0x6B);
    Wire.write(0);
    if (Wire.endTransmission(true) == 0) break;
    delay(50);
  }

  // 2. Initialize nRF24L01 (Retry on cold boot, never lock up)
  Serial.println("Initializing nRF24L01...");
  while (!radio.begin()) {
    Serial.println("Waiting for nRF24 power...");
    delay(200);
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(76);
  radio.setPayloadSize(1);
  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println("Transmitter Ready! (Horizontal = Neutral STOP)\n");
}

void loop() {
  int16_t rawX = 0, rawY = 0, rawZ = 0;

  if (readRawMPU(rawX, rawY, rawZ)) {
    float x = (float)rawX;
    float y = (float)rawY;
    float z = (float)rawZ;

    // Symmetric 3D tilt angle formulas relative to horizontal (0 deg)
    float pitch = atan2(x, sqrt(y * y + z * z)) * 180.0 / PI;
    float roll  = atan2(y, sqrt(x * x + z * z)) * 180.0 / PI;

    // Fast, responsive smoothing filter
    fPitch = 0.5 * fPitch + 0.5 * pitch;
    fRoll  = 0.5 * fRoll  + 0.5 * roll;
  }

  // Determine gesture command (Horizontal = 'S')
  char cmd = 'S';

  if (abs(fPitch) >= abs(fRoll)) {
    if (fPitch > TILT_THRESHOLD) {
      cmd = 'F'; // Tilt Forward
    } else if (fPitch < -TILT_THRESHOLD) {
      cmd = 'B'; // Tilt Backward
    }
  } else {
    // Left and Right swapped to match hand orientation
    if (fRoll > TILT_THRESHOLD) {
      cmd = 'L'; // Tilt Left
    } else if (fRoll < -TILT_THRESHOLD) {
      cmd = 'R'; // Tilt Right
    }
  }

  unsigned long now = millis();
  if (now - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = now;

    bool ok = radio.write(&cmd, 1);

    Serial.print("Pitch: ");
    Serial.print((int)fPitch);
    Serial.print(" deg | Roll: ");
    Serial.print((int)fRoll);
    Serial.print(" deg | CMD: ");
    Serial.print(cmd);
    Serial.println(ok ? " [ACK OK]" : " [NO ACK]");
  }

  delay(15);
}
