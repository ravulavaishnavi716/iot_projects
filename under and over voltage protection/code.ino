#include<math.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  // 20x4 LCD

int decimalPrecision = 2;             // decimal places for all values shown in LED Display & Serial Monitor
int VoltageAnalogInputPin = A0;       // Which pin to measure voltage Value (Pin A0 is reserved for button function)
int relayPin = 7;                     // Pin connected to the relay

float voltageSampleRead = 0;
float voltageLastSample = 0;
float voltageSampleSum = 0;
float voltageSampleCount = 0;
float voltageMean;
float RMSVoltageMean;
float adjustRMSVoltageMean;
float FinalRMSVoltage;

float voltageOffset1 = 0.00;          // AC Voltage Offset
float voltageOffset2 = 0.00;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2); // initialize the lcd
  lcd.backlight();
  pinMode(relayPin, OUTPUT);
}

void loop() {
  if (micros() >= voltageLastSample + 1000) {
    voltageSampleRead = (analogRead(VoltageAnalogInputPin) - 512) + voltageOffset1;
    voltageSampleSum = voltageSampleSum + sq(voltageSampleRead);
    voltageSampleCount++;
    voltageLastSample = micros();
  }

  if (voltageSampleCount == 1000) {
    voltageMean = voltageSampleSum / voltageSampleCount;
    RMSVoltageMean = (sqrt(voltageMean)) * 1.5;
    adjustRMSVoltageMean = RMSVoltageMean + voltageOffset2;
    FinalRMSVoltage = RMSVoltageMean + voltageOffset2;

    // Voltage condition check
    if (FinalRMSVoltage < 180) {
      // Low voltage
      digitalWrite(relayPin, HIGH);  // Turn on relay
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Low Voltage");
    } else if (FinalRMSVoltage > 240) {
      // Overvoltage
      digitalWrite(relayPin, HIGH);  // Turn on relay
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Over Voltage");
    } else {
      // Normal voltage
      digitalWrite(relayPin, LOW);   // Turn off relay
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Normal Voltage");
    }

    // Display voltage value
    lcd.setCursor(0, 1);
    lcd.print("Voltage: ");
    lcd.print(FinalRMSVoltage, decimalPrecision);
    lcd.print("V");

    // Reset variables
    voltageSampleSum = 0;
    voltageSampleCount = 0;
  }
}
