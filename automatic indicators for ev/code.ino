#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu1(0x68); // Address of first MPU6050
MPU6050 mpu2(0x69); // Address of second MPU6050

const int indicatorRelayRight = 2;  // Indicator Relay Right connected to pin 2
const int indicatorRelayLeft = 3;   // Indicator Relay Left connected to pin 3
const int indicatorSwitchRight = 4; // Indicator Switch Right connected to pin 4
const int indicatorSwitchLeft = 5;  // Indicator Switch Left connected to pin 5

float baseY1 = 0.0;
float baseY2 = 0.0;
bool initialized = false;
bool mpu1Connected = false;
bool mpu2Connected = false;

void setup() {
  Serial.begin(9600);
  pinMode(indicatorRelayRight, OUTPUT);
  pinMode(indicatorRelayLeft, OUTPUT);
  pinMode(indicatorSwitchRight, INPUT_PULLUP);
  pinMode(indicatorSwitchLeft, INPUT_PULLUP);

  digitalWrite(indicatorRelayRight, LOW); // Initially turn off indicator relay right
  digitalWrite(indicatorRelayLeft, LOW);  // Initially turn off indicator relay left

  Wire.begin();
  mpu1.initialize();
  mpu2.initialize();

  mpu1Connected = mpu1.testConnection();
  mpu2Connected = mpu2.testConnection();

  if (!mpu1Connected && !mpu2Connected) {
    while (true) {
      Serial.println("No MPU6050 sensors detected! System halted.");
      delay(1000);
    }
  }

  Serial.println("System initialized.");
}

void loop() {
  // Read initial Y-axis values as base if not already initialized
  if (!initialized) {
    if (mpu1Connected) baseY1 = mpu1.getAccelerationY();
    if (mpu2Connected) baseY2 = mpu2.getAccelerationY();
    initialized = true;
    Serial.println("Base Y-axis values set:");
    if (mpu1Connected) {
      Serial.print("Base Y1: ");
      Serial.println(baseY1);
    }
    if (mpu2Connected) {
      Serial.print("Base Y2: ");
      Serial.println(baseY2);
    }
  }

  // Read Y-axis acceleration values
  float currentY1 = 0.0, currentY2 = 0.0;
  float changeY1 = 0.0, changeY2 = 0.0;

  if (mpu1Connected) {
    currentY1 = mpu1.getAccelerationY();
    changeY1 = abs((currentY1 - baseY1) / baseY1) * 100.0;
  }

  if (mpu2Connected) {
    currentY2 = mpu2.getAccelerationY();
    changeY2 = abs((currentY2 - baseY2) / baseY2) * 100.0;
  }

  // Read switch states
  bool switchRightState = !digitalRead(indicatorSwitchRight); // Active LOW
  bool switchLeftState = !digitalRead(indicatorSwitchLeft);   // Active LOW

  // Control Indicator Relay Right
  if (mpu1Connected) {
    if (switchRightState) {
      if (changeY1 > 80.0) {
        digitalWrite(indicatorRelayRight, LOW); // Turn off relay if change exceeds 80%
      } else {
        digitalWrite(indicatorRelayRight, HIGH); // Turn on relay if switch is on and no significant change
      }
    } else {
      digitalWrite(indicatorRelayRight, LOW); // Turn off relay if switch is off
    }
  }

  // Control Indicator Relay Left
  if (mpu2Connected) {
    if (switchLeftState) {
      if (changeY2 > 80.0) {
        digitalWrite(indicatorRelayLeft, LOW); // Turn off relay if change exceeds 80%
      } else {
        digitalWrite(indicatorRelayLeft, HIGH); // Turn on relay if switch is on and no significant change
      }
    } else {
      digitalWrite(indicatorRelayLeft, LOW); // Turn off relay if switch is off
    }
  }

  // Print system status
  Serial.println("---------------------------------");
  if (mpu1Connected) {
    Serial.print("Current Y1: ");
    Serial.println(currentY1);
    Serial.print("Change Y1: ");
    Serial.print(changeY1);
    Serial.println("%");
    Serial.print("Switch Right State: ");
    Serial.println(switchRightState ? "ON" : "OFF");
    Serial.print("Indicator Relay Right State: ");
    Serial.println(digitalRead(indicatorRelayRight) ? "ON" : "OFF");
  } else {
    Serial.println("MPU1 not connected.");
  }

  if (mpu2Connected) {
    Serial.print("Current Y2: ");
    Serial.println(currentY2);
    Serial.print("Change Y2: ");
    Serial.print(changeY2);
    Serial.println("%");
    Serial.print("Switch Left State: ");
    Serial.println(switchLeftState ? "ON" : "OFF");
    Serial.print("Indicator Relay Left State: ");
    Serial.println(digitalRead(indicatorRelayLeft) ? "ON" : "OFF");
  } else {
    Serial.println("MPU2 not connected.");
  }
  Serial.println("---------------------------------");

  delay(2000); // Small delay to stabilize readings
}
