#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(A0, A1);   // Uno CE=A0, CSN=A1
const byte address[6] = "00001";

char command = 'S';
char prevCommand = ' ';
unsigned long lastReceiveTime = 0;
const unsigned long TIMEOUT_MS = 350; // Stop car if signal lost for >350ms

// Motor Pins (User's exact wiring: 3 through 10)
const int FL1 = 3;  const int FL2 = 4;
const int BL1 = 5;  const int BL2 = 6;
const int FR1 = 7;  const int FR2 = 8;
const int BR1 = 9;  const int BR2 = 10;

void executeCommand(char cmd);
void forward();
void backward();
void left();
void right();
void stopMotors();
void move(int fl1, int fl2, int bl1, int bl2, int fr1, int fr2, int br1, int br2);

void setup() {
  Serial.begin(9600);
  delay(500);

  // 1. Initialize nRF24 FIRST so SPI Master mode on Pin 10 is established
  Serial.println("Initializing nRF24L01...");
  while (!radio.begin()) {
    Serial.println("Waiting for nRF24 power...");
    delay(200);
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(76);
  radio.setPayloadSize(1);
  radio.openReadingPipe(0, address);
  radio.startListening();

  // 2. Initialize motor pins AFTER SPI is initialized
  for (int i = 3; i <= 10; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  stopMotors();

  prevCommand = 'S';
  lastReceiveTime = millis();
  Serial.println("=== CAR RECEIVER READY ===");
}

void loop() {
  if (radio.available()) {
    char c = 'S';
    radio.read(&c, 1);

    if (c == 'F' || c == 'B' || c == 'L' || c == 'R' || c == 'S') {
      command = c;
      lastReceiveTime = millis();
      Serial.print("RX: ");
      Serial.println(command);
    }
  }

  // Failsafe: if transmitter is turned off or signal lost, stop car immediately
  if (millis() - lastReceiveTime > TIMEOUT_MS) {
    if (command != 'S') {
      command = 'S';
      Serial.println("Failsafe: Signal Lost -> STOP");
    }
  }

  // Update motors ONLY when command changes to eliminate motor noise on SPI
  if (command != prevCommand) {
    prevCommand = command;
    executeCommand(command);
  }
}

void executeCommand(char cmd) {
  switch (cmd) {
    case 'F': forward(); break;
    case 'B': backward(); break;
    case 'L': left(); break;
    case 'R': right(); break;
    default:  stopMotors(); break;
  }
}

void forward() {
  move(HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW);
}

void backward() {
  move(LOW, HIGH, LOW, HIGH, LOW, HIGH, LOW, HIGH);
}

void left() {
  move(LOW, HIGH, LOW, HIGH, HIGH, LOW, HIGH, LOW);
}

void right() {
  move(HIGH, LOW, HIGH, LOW, LOW, HIGH, LOW, HIGH);
}

void stopMotors() {
  move(LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW);
}

void move(int fl1, int fl2, int bl1, int bl2,
          int fr1, int fr2, int br1, int br2) {
  digitalWrite(FL1, fl1); digitalWrite(FL2, fl2);
  digitalWrite(BL1, bl1); digitalWrite(BL2, bl2);
  digitalWrite(FR1, fr1); digitalWrite(FR2, fr2);
  digitalWrite(BR1, br1); digitalWrite(BR2, br2);
}
