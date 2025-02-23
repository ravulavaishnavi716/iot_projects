Key Functionalities:
Remote Control via Blynk App

The system connects to Blynk IoT using WiFi.
Users can control Relay 1 (V1) and Relay 2 (V2) from the Blynk app.
It also sends real-time temperature and humidity data from the DHT11 sensor to Blynk (V3 & V4).
Manual Control via Physical Switches

Two push buttons (connected to GPIO 33 & 32) allow local control.
Pressing a switch toggles the respective relay ON/OFF.
Bluetooth Control via HC-05

The system can receive Bluetooth commands (1, 2, 3, 4) to turn relays ON/OFF.
Communication is handled using HardwareSerial on RX (16) and TX (17) pins.
Local Web Server for HTTP Control

Users can send simple HTTP requests to control relays:
http://device-ip/relay1/on → Turns Relay 1 ON
http://device-ip/relay1/off → Turns Relay 1 OFF
http://device-ip/relay2/on → Turns Relay 2 ON
http://device-ip/relay2/off → Turns Relay 2 OFF
WiFi Status LED Indicator

A blue LED (GPIO 2) lights up when WiFi is connected.
Periodic DHT11 Sensor Readings

Every 5 seconds, the ESP32 reads temperature and humidity from the DHT11 sensor and updates Blynk.
Possible Use Cases:
Smart Home Automation: Control lights, fans, or other appliances remotely.
Industrial Monitoring: Monitor temperature/humidity and control devices.
Agriculture: Automate irrigation based on temperature/humidity levels.
Enhancements You Can Consider:
Add MQTT for cloud-based control.
Add OTA Updates for remote firmware updates.
Integrate voice control (e.g., Google Assistant/Alexa).
