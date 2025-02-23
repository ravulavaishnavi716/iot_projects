#define BLYNK_TEMPLATE_ID "TMPL3gT7ueOdQ"
#define BLYNK_TEMPLATE_NAME "fault detection"
#define BLYNK_AUTH_TOKEN "vpiQCuN5F_sh7Ondy6X3x35qdjfiabCX"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// WiFi credentials
char ssid[] = "Rushi";
char pass[] = "Rushi1928";

// Relay and sensor configuration
const int relayPins[] = {D1, D2, D3, D4}; // Define GPIO pins for relays
const char* relayNames[] = {
  "Head Light Wire",
  "Horn Wire",
  "Indicators Wire",
  "DC-DC Converter Wire"
};
const int relayCount = 4;
const int voltageSensorPin = A0; // Voltage sensor connected to A0
int offset = 20; // Voltage correction offset
const int relayOnDuration = 5000; // Relay on time in milliseconds

BlynkTimer timer; // Timer for periodic tasks

// Function to send data to both Serial monitor and Blynk
void sendToSerialAndBlynk(const String& message) {
  Serial.println(message);
  Blynk.virtualWrite(V1, message);
}

void setup() {
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Initialize relay pins
  for (int i = 0; i < relayCount; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
  }

  sendToSerialAndBlynk("System initialized.");

  // Set up a repeating function to control relays and read sensor data
  timer.setInterval(15000L, controlRelaysAndSendData);
}

void controlRelaysAndSendData() {
  for (int i = 0; i < relayCount; i++) {
    String activationMessage = String("Activating ") + relayNames[i];
    sendToSerialAndBlynk(activationMessage);

    digitalWrite(relayPins[i], HIGH); // Turn on the relay
    delay(100); // Allow relay to stabilize

    // Read voltage from the sensor
    int rawValue = analogRead(voltageSensorPin);
    double voltage = map(rawValue, 0, 1023, 0, 2500) + offset; // Convert ADC to voltage
    voltage /= 100.0; // Convert to volts

    String voltageMessage = "Voltage: " + String(voltage, 2) + " V";
    sendToSerialAndBlynk(voltageMessage);

    if (voltage < 5.0) {
      String faultMessage = "Fault in " + String(relayNames[i]) + ", Voltage: " + String(voltage, 2) + " V";
      sendToSerialAndBlynk(faultMessage);
    } else {
      String okMessage = "Voltage OK in " + String(relayNames[i]) + ": " + String(voltage, 2) + " V";
      sendToSerialAndBlynk(okMessage);
    }

    delay(relayOnDuration - 100); // Keep relay on for remaining time
    digitalWrite(relayPins[i], LOW); // Turn off the relay
  }
}

void loop() {
  Blynk.run();
  timer.run();
}
