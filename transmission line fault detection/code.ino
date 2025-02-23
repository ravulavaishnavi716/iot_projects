#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Robojax_AllegroACS_Current_Sensor.h>
#include <SoftwareSerial.h>

// Define pins for current sensors
const int VIN1 = A0; 
const int VIN2 = A1; 
const int VIN3 = A2; 
const float VCC = 5.04; 
const int MODEL = 0; 

// Define pins for relays
const int RELAY_R = 2;
const int RELAY_Y = 3;
const int RELAY_B = 4;

// Initialize sensors
Robojax_AllegroACS_Current_Sensor sensor1(MODEL, VIN1);
Robojax_AllegroACS_Current_Sensor sensor2(MODEL, VIN2);
Robojax_AllegroACS_Current_Sensor sensor3(MODEL, VIN3);

// Initialize LCD display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define SoftwareSerial pins for ESP8266
SoftwareSerial espSerial(10, 11); // RX, TX

void setup() {
    Serial.begin(9600); // initialize serial monitor
    espSerial.begin(9600); // initialize software serial for ESP8266
    lcd.begin(16 ,2);
    lcd.backlight();
    lcd.clear();
    lcd.print("Robojax Tutorial");
    delay(2000);
    lcd.clear();
    lcd.print("ACS770 Current");
    lcd.setCursor(0, 1);
    lcd.print("Sensor");
    delay(2000);
    lcd.clear();

    // Initialize relay pins
    pinMode(RELAY_R, OUTPUT);
    pinMode(RELAY_Y, OUTPUT);
    pinMode(RELAY_B, OUTPUT);

    // Turn off all relays initially
    digitalWrite(RELAY_R, LOW);
    digitalWrite(RELAY_Y, LOW);
    digitalWrite(RELAY_B, LOW);
}

void loop() {
    float avgCurrent1 = sensor1.getCurrentAverage(300);
    float avgCurrent2 = sensor2.getCurrentAverage(300);
    float avgCurrent3 = sensor3.getCurrentAverage(300);

    // Print current sensor values to the serial monitor
    Serial.print("Current Sensor 1: ");
    Serial.print(avgCurrent1);
    Serial.println(" A");
    Serial.print("Current Sensor 2: ");
    Serial.print(avgCurrent2);
    Serial.println(" A");
    Serial.print("Current Sensor 3: ");
    Serial.print(avgCurrent3);
    Serial.println(" A");

    float faultDistance1 = -1;
    float faultDistance2 = -1;
    float faultDistance3 = -1;

    // Determine fault distance for sensor 1
    if (avgCurrent1 >= 1.1 && avgCurrent1 <= 1.4) {
        faultDistance1 = 2;
    } else if (avgCurrent1 >= 0.7 && avgCurrent1 <= 0.9) {
        faultDistance1 = 4;
    } else if (avgCurrent1 >= 0.6 && avgCurrent1 <= 0.68) {
        faultDistance1 = 6;
    } else if (avgCurrent1 >= 0.5 && avgCurrent1 <= 0.59) {
        faultDistance1 = 8;
    }

    // Determine fault distance for sensor 2
    if (avgCurrent2 >= 2.060 && avgCurrent2 <= 2.110) {
        faultDistance2 = 2;
    } else if (avgCurrent2 >= 1.176 && avgCurrent2 <= 1.190) {
        faultDistance2 = 4;
    } else if (avgCurrent2 >= 0.8 && avgCurrent2 <= 0.9) {
        faultDistance2 = 6;
    } else if (avgCurrent2 >= 0.5 && avgCurrent2 <= 0.7) {
        faultDistance2 = 8;
    }

    // Determine fault distance for sensor 3
    if (avgCurrent3 >= 1 && avgCurrent3 <= 1.2) {
        faultDistance3 = 2;
    } else if (avgCurrent3 >= 0.73 && avgCurrent3 <= 0.78) {
        faultDistance3 = 4;
    } else if (avgCurrent3 >= 0.54 && avgCurrent3 <= 0.59) {
        faultDistance3 = 6;
    } else if (avgCurrent3 >= 0.44 && avgCurrent3 <= 0.5) {
        faultDistance3 = 8;
    }

    // Check for faults and update LCD and relays
    lcd.clear();
    digitalWrite(RELAY_R, LOW);
    digitalWrite(RELAY_Y, LOW);
    digitalWrite(RELAY_B, LOW);

    if (avgCurrent1 < 0.4 && avgCurrent2 < 0.4 && avgCurrent3 < 0.4) {
        Serial.println("No fault");
        espSerial.println("No fault");
        lcd.print("No fault");
    } else if (faultDistance1 != -1 && faultDistance2 != -1 && faultDistance3 != -1) {
        Serial.println("3 phase fault");
        espSerial.println("3 phase fault");
        lcd.print("3 phase fault");
        lcd.setCursor(0, 1);
        lcd.print(faultDistance1);
        lcd.print(",");
        lcd.print(faultDistance2);
        lcd.print(",");
        lcd.print(faultDistance3);
        lcd.print("KM R-Y-B");
        digitalWrite(RELAY_R, HIGH);
        digitalWrite(RELAY_Y, HIGH);
        digitalWrite(RELAY_B, HIGH);
    } else if (faultDistance1 != -1 && faultDistance2 != -1) {
        Serial.println("2 phase fault R-Y");
        espSerial.println("2 phase fault R-Y");
        lcd.print("2 phase fault");
        lcd.setCursor(0, 1);
        lcd.print(faultDistance1);
        lcd.print(",");
        lcd.print(faultDistance2);
        lcd.print("KM R-Y");
        digitalWrite(RELAY_R, HIGH);
        digitalWrite(RELAY_Y, HIGH);
    } else if (faultDistance2 != -1 && faultDistance3 != -1) {
        Serial.println("2 phase fault Y-B");
        espSerial.println("2 phase fault Y-B");
        lcd.print("2 phase fault");
        lcd.setCursor(0, 1);
        lcd.print(faultDistance2);
        lcd.print(",");
        lcd.print(faultDistance3);
        lcd.print("KM Y-B");
        digitalWrite(RELAY_Y, HIGH);
        digitalWrite(RELAY_B, HIGH);
    } else if (faultDistance3 != -1 && faultDistance1 != -1) {
        Serial.println("2 phase fault B-R");
        espSerial.println("2 phase fault B-R");
        lcd.print("2 phase fault");
        lcd.setCursor(0, 1);
        lcd.print(faultDistance3);
        lcd.print(",");
        lcd.print(faultDistance1);
        lcd.print("KM B-R");
        digitalWrite(RELAY_B, HIGH);
        digitalWrite(RELAY_R, HIGH);
    } else {
        if (faultDistance1 != -1) {
            Serial.print("L-L fault at ");
            Serial.print(faultDistance1);
            Serial.println("KM in R phase");
            espSerial.print("L-L fault at ");
            espSerial.print(faultDistance1);
            espSerial.println("KM in R phase");
            lcd.print("L-L fault");
            lcd.setCursor(0, 1);
            lcd.print(faultDistance1);
            lcd.print("KM in R phase");
            digitalWrite(RELAY_R, HIGH);
        }

        if (faultDistance2 != -1) {
            Serial.print("L-L fault at ");
            Serial.print(faultDistance2);
            Serial.println("KM in Y phase");
            espSerial.print("L-L fault at ");
            espSerial.print(faultDistance2);
            espSerial.println("KM in Y phase");
            lcd.print("L-L fault");
            lcd.setCursor(0, 1);
            lcd.print(faultDistance2);
            lcd.print("KM in Y phase");
            digitalWrite(RELAY_Y, HIGH);
        }

        if (faultDistance3 != -1) {
            Serial.print("L-L fault at ");
            Serial.print(faultDistance3);
            Serial.println("KM in B phase");
            espSerial.print("L-L fault at ");
            espSerial.print(faultDistance3);
            espSerial.println("KM in B phase");
            lcd.print("L-L fault");
            lcd.setCursor(0, 1);
            lcd.print(faultDistance3);
            lcd.print("KM in B phase");
            digitalWrite(RELAY_B, HIGH);
        }
    }

    delay(8000); // delay 8 seconds before next loop
}
