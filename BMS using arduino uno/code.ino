#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_MLX90614.h>
#include "ACS712.h"
#include <SoftwareSerial.h>

// SoftwareSerial for communication with ESP8266
SoftwareSerial espSerial(2, 3); // RX, TX

// Constants and Pins
const float R1 = 100000.0; // Resistor R1 in ohms (for voltage divider)
const float R2 = 10000.0;  // Resistor R2 in ohms
const int voltagePin = A0; // Analog pin for voltage measurement

// Initialize ACS712 Current Sensor
ACS712 sensor(ACS712_30A, A1);

// Battery Specifications
const float maxCellVoltage = 4.2; // Max voltage per cell
const float minCellVoltage = 3.0; // Min voltage per cell
const float numCellsSeries = 13;  // Cells in series
const float batteryCapacityAh = 24.3; // Battery capacity in Ah

const float maxVoltage = maxCellVoltage * numCellsSeries; // Full charge voltage
const float minVoltage = minCellVoltage * numCellsSeries; // Discharged voltage
float currentSOC = 100.0; // Initial SOC (start fully charged)

// Time Tracking
RTC_DS3231 rtc;
DateTime chargeStartTime, dischargeStartTime;
bool isCharging = false;
bool isDischarging = false;

// Energy and Power Tracking
float totalEnergyConsumed = 0.0; // Total energy in Wh
float powerUsage = 0.0;          // Power usage in Watts

// Temperature Tracking
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
float maxChargingTemp = -100.0; // Maximum temperature during charging
float maxDischargingTemp = -100.0; // Maximum temperature during discharging
float currentTemp = 0.0;

// Average Temperature Tracking
float tempSum = 0.0;    // Sum of all temperature readings
unsigned long tempCount = 0; // Number of readings
float avgTemp = 0.0;    // Average temperature

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600); // Initialize ESP8266 communication
  Wire.begin();

  // Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Initialize MLX90614
  if (!mlx.begin()) {
    Serial.println("Failed to initialize MLX90614");
    while (1);
  }

  // Calibrate ACS712 sensor
  sensor.calibrate();
}

void loop() {
  DateTime now = rtc.now();

  // Measure Battery Voltage
  int rawVoltage = analogRead(voltagePin);
  float batteryVoltage = (rawVoltage * 5.0 / 1023.0) * ((R1 + R2) / R2);

  // Measure Current
  float batteryCurrent = sensor.getCurrentDC();

  // Measure Temperature
  currentTemp = mlx.readObjectTempC();

  // Update Temperature Stats
  tempSum += currentTemp;
  tempCount++;
  avgTemp = tempSum / tempCount;

  if (isCharging && currentTemp > maxChargingTemp) {
    maxChargingTemp = currentTemp;
  }
  if (isDischarging && currentTemp > maxDischargingTemp) {
    maxDischargingTemp = currentTemp;
  }

  if (batteryCurrent > 0 && !isCharging) {
    isCharging = true;
    isDischarging = false;
    chargeStartTime = now;
  } else if (batteryCurrent < 0 && !isDischarging) {
    isCharging = false;
    isDischarging = true;
    dischargeStartTime = now;
  }

  powerUsage = batteryVoltage * batteryCurrent;

  static unsigned long lastMeasurementTime = 0;
  unsigned long currentTime = millis();
  if (lastMeasurementTime > 0) {
    float elapsedHours = (currentTime - lastMeasurementTime) / 3600000.0;
    float consumedAh = batteryCurrent * elapsedHours;
    currentSOC -= (consumedAh / batteryCapacityAh) * 100.0;
    if (currentSOC < 0) currentSOC = 0.0;
    if (currentSOC > 100) currentSOC = 100.0;
  }
  lastMeasurementTime = currentTime;

  totalEnergyConsumed += powerUsage * (currentTime - lastMeasurementTime) / 3600000.0;

  float voltageSOC = map(batteryVoltage, minVoltage, maxVoltage, 0, 100);
  currentSOC = (currentSOC + voltageSOC) / 2;

  // Create data string
  String data = "Voltage:" + String(batteryVoltage, 2) + "," +
                "Current:" + String(batteryCurrent, 2) + "," +
                "SOC:" + String(currentSOC, 2) + "," +
                "Power:" + String(powerUsage, 2) + "," +
                "Temp:" + String(currentTemp, 2);

  // Send data to Serial Monitor and ESP8266
  Serial.println(data);
  espSerial.println(data);

  delay(1000);
}
