#define BLYNK_TEMPLATE_ID "TMPL3cRDNgj0P"
#define BLYNK_TEMPLATE_NAME "home automation"
#define BLYNK_AUTH_TOKEN "RcyU9twdfB8j-ufirI9mx4XNF80aJy6i"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <WebServer.h>

// WiFi credentials
char ssid[] = "Rushi";
char pass[] = "Rushi1928";

// Relay pins
const int relay1Pin = 25;
const int relay2Pin = 26;

// Switch pins
const int switch1Pin = 33;
const int switch2Pin = 32;

// Bluetooth HC-05 RX/TX pins
#define RX_PIN 16
#define TX_PIN 17

// Blue LED pin (indicates WiFi connection)
const int wifiLedPin = 2;

// DHT11 pin and type
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Relay states
bool relay1State = false;
bool relay2State = false;

// Previous switch states
bool lastSwitch1State = HIGH;
bool lastSwitch2State = HIGH;

// Bluetooth serial
HardwareSerial BTSerial(1);

// HTTP Web Server
WebServer server(80);

// Timer for DHT11 readings
unsigned long lastDHTReadTime = 0;
const unsigned long dhtInterval = 5000; // 5 seconds

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);

  // Initialize Bluetooth
  BTSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  // Initialize Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Set relay pins as outputs
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);

  // Set switch pins as inputs with pullup resistors
  pinMode(switch1Pin, INPUT_PULLUP);
  pinMode(switch2Pin, INPUT_PULLUP);

  // Set WiFi LED pin as output
  pinMode(wifiLedPin, OUTPUT);
  digitalWrite(wifiLedPin, LOW);

  // Ensure relays are off at startup
  digitalWrite(relay1Pin, LOW);
  digitalWrite(relay2Pin, LOW);

  // Configure HTTP server endpoints
  setupServer();

  // Start HTTP server
  server.begin();
  Serial.println("HTTP server started");

  // Initialize DHT sensor
  dht.begin();
}

void loop() {
  Blynk.run();
  server.handleClient();

  // WiFi LED Indicator
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(wifiLedPin, HIGH);
  } else {
    digitalWrite(wifiLedPin, LOW);
  }

  // Check switch states for Relay 1
  bool currentSwitch1State = digitalRead(switch1Pin);
  if (currentSwitch1State == LOW && lastSwitch1State == HIGH) {
    relay1State = !relay1State;
    updateRelay(1, relay1State);
  }
  lastSwitch1State = currentSwitch1State;

  // Check switch states for Relay 2
  bool currentSwitch2State = digitalRead(switch2Pin);
  if (currentSwitch2State == LOW && lastSwitch2State == HIGH) {
    relay2State = !relay2State;
    updateRelay(2, relay2State);
  }
  lastSwitch2State = currentSwitch2State;

  // Check Bluetooth commands
  if (BTSerial.available()) {
    char command = BTSerial.read();
    handleBluetoothCommand(command);
  }

  // DHT11 Readings
  if (millis() - lastDHTReadTime >= dhtInterval) {
    lastDHTReadTime = millis();
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (isnan(temp) || isnan(hum)) {
      Serial.println("Unable to read DHT11 sensor");
    } else {
      Serial.print("Temperature: ");
      Serial.print(temp);
      Serial.println("°C");
      Serial.print("Humidity: ");
      Serial.print(hum);
      Serial.println("%");

      // Send readings to Blynk
      Blynk.virtualWrite(V3, temp);
      Blynk.virtualWrite(V4, hum);
    }
  }
}

// Function to handle Blynk virtual write for Relay 1
BLYNK_WRITE(V1) {
  relay1State = param.asInt();
  updateRelay(1, relay1State);
}

// Function to handle Blynk virtual write for Relay 2
BLYNK_WRITE(V2) {
  relay2State = param.asInt();
  updateRelay(2, relay2State);
}

// Function to update relay state and sync with Blynk
void updateRelay(int relayNumber, bool state) {
  if (relayNumber == 1) {
    digitalWrite(relay1Pin, state ? HIGH : LOW);
    Blynk.virtualWrite(V1, state);
    Serial.print("Relay 1 is ");
    Serial.println(state ? "ON" : "OFF");
  } else if (relayNumber == 2) {
    digitalWrite(relay2Pin, state ? HIGH : LOW);
    Blynk.virtualWrite(V2, state);
    Serial.print("Relay 2 is ");
    Serial.println(state ? "ON" : "OFF");
  }
}

// Function to handle Bluetooth commands
void handleBluetoothCommand(char command) {
  switch (command) {
    case '1': // Turn Relay 1 ON
      relay1State = true;
      updateRelay(1, relay1State);
      break;

    case '2': // Turn Relay 1 OFF
      relay1State = false;
      updateRelay(1, relay1State);
      break;

    case '3': // Turn Relay 2 ON
      relay2State = true;
      updateRelay(2, relay2State);
      break;

    case '4': // Turn Relay 2 OFF
      relay2State = false;
      updateRelay(2, relay2State);
      break;

    default:
      Serial.println("Unknown command");
      break;
  }
}

// Function to setup HTTP server endpoints
void setupServer() {
  server.on("/relay1/on", []() {
    relay1State = true;
    updateRelay(1, relay1State);
    server.send(200, "text/plain", "Relay 1 ON");
  });

  server.on("/relay1/off", []() {
    relay1State = false;
    updateRelay(1, relay1State);
    server.send(200, "text/plain", "Relay 1 OFF");
  });

  server.on("/relay2/on", []() {
    relay2State = true;
    updateRelay(2, relay2State);
    server.send(200, "text/plain", "Relay 2 ON");
  });

  server.on("/relay2/off", []() {
    relay2State = false;
    updateRelay(2, relay2State);
    server.send(200, "text/plain", "Relay 2 OFF");
  });
}
