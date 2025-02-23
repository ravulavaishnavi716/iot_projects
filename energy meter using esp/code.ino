// Fill-in information from your Blynk Template here
#define BLYNK_TEMPLATE_ID "TMPL39eJt_eFp"
#define BLYNK_TEMPLATE_NAME "M and I"

#define BLYNK_FIRMWARE_VERSION        "0.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_SPARKFUN_BLYNK_BOARD
#define USE_NODE_MCU_BOARD
//#define USE_WITTY_CLOUD_BOARD
//#define USE_WEMOS_D1_MINI

#include "BlynkEdgent.h"
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_SSD1306.h>

/****************************************************************************/

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_INA219 ina219;

unsigned long previousMillis = 0;
unsigned long interval = 2000;
float shuntvoltage = 0.00;
float busvoltage = 0.00;
float current = 0.00;
float loadvoltage = 0.00;
float energy = 0.00,  energyCost, energyPrevious, energyDifference;
float energyPrice = 6.50 ;
float power = 0.00;
float capacity = 0.00;

void setup()
{
  Serial.begin(115200);
  while (!Serial) {
    // will pause Zero, Leonardo, etc until serial console opens
    delay(1);
  }

  uint32_t currentFrequency;

  Serial.begin(115200);
  // initialize OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(10, 30);
  display.print("IoT Energy Meter");
  display.display(); 
  //Start Blynk 2.0
  BlynkEdgent.begin();

  // Initialize the INA219.
  // By default the initialization will use the largest range (32V, 2A).  However
  // you can call a setCalibration function to change this range (see comments).
  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) {
      delay(10);
    }
  }
  // To use a slightly lower 32V, 1A range (higher precision on amps):
  //ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  //ina219.setCalibration_16V_400mA();

  Serial.println("IoT Energy Meter with INA219 ...");
}

void loop()
{
  BlynkEdgent.run();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    ina219values();
    displaydata();
  }
}


void ina219values()  {

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current = ina219.getCurrent_mA();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  power = loadvoltage * current;
  energy = energy + power / 3600; //Wh
  capacity = capacity + current / 1000;
  energyDifference = energy - energyPrevious;
  energyPrevious = energy;
  energyCost = energyCost + (energyPrice / 1000 * energyDifference);
    // Send data to blynk
  Blynk.virtualWrite(V7, current);
  Blynk.virtualWrite(V5, String("Rs.") + String(energyPrice) );

  // nothing connected? set all to 0, otherwise they float around 0.

  if (loadvoltage < 1 )loadvoltage = 0;
  if (current < 1 )
  {
    current = 0;
    power = 0;
    energy = 0;
    capacity = 0;
    energyCost=0;
  }

  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power); Serial.println(" mW");
  Serial.print("Energy:        "); Serial.print(energy); Serial.println(" Wh");
  Serial.print("Capacity:      "); Serial.print(capacity); Serial.println(" Ah");
  Serial.print("Energy Cost:   "); Serial.print("Rs. "); Serial.println(energyCost);
  Serial.println("-------------------------");
}
void displaydata() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  // VOLTAGE
  Blynk.virtualWrite(V0, String(loadvoltage, 2) + String(" V") );
  display.setCursor(0, 5);
  display.print(loadvoltage, 2);
  display.print(" V");
  // CURRENT
  if (current > 1000) {
    Blynk.virtualWrite(V1, String((current / 1000), 2) + String(" A") );
    display.setCursor(60, 5);
    display.print((current / 1000), 2);
    display.println(" A");
    display.setCursor(0, 15);
    display.println("--------------------");
  }
  else
  {
    Blynk.virtualWrite(V1, String(current, 2) + String(" mA"));
    display.setCursor(60, 5);
    display.print(current, 1);
    display.println(" mA");
    display.setCursor(0, 15);
    display.println("--------------------");
  }


  display.setCursor(60, 20);
  display.print("|");
  display.setCursor(60, 24);
  display.print("|");
  display.setCursor(60, 28);
  display.print("|");
  display.setCursor(60, 32);
  display.print("|");
  display.setCursor(60, 36);
  display.print("|");
  display.setCursor(60, 40);
  display.print("|");
  display.setCursor(0, 46);
  display.print("--------------------");

  // POWER
  if (power > 1000) {
    Blynk.virtualWrite(V2, String((power / 1000), 2) + String(" W") );
    display.setCursor(0, 24);
    display.print(String((power / 1000), 2));
    display.println(" W");
  }
  else {
    Blynk.virtualWrite(V2, String(power, 2) + String(" mW") );
    display.setCursor(0, 24);
    display.print(power, 2);
    display.println(" mW");
  }

  //Energy Comsumption
  if (energy > 1000) {
    Blynk.virtualWrite(V3, String((energy / 1000), 2) + String(" kWh"));
    display.setCursor(0, 36);
    display.print((energy / 1000), 2);
    display.println(" kWh");
  }
  else {
    Blynk.virtualWrite(V3, String(energy, 2) + String(" Wh"));
    display.setCursor(0, 36);
    display.print(energy, 2);
    display.println(" Wh");
  }


  // CAPACITY
  if (capacity > 1000) {
    Blynk.virtualWrite(V4, String((capacity / 1000), 2) + String(" Ah") );
    display.setCursor(65, 24);
    display.println("Capacity:");
    display.setCursor(65, 36);
    display.print((capacity / 1000), 2);
    display.println(" Ah");
    
  }
  else
  {
    Blynk.virtualWrite(V4, String((capacity), 2) + String(" mAh") );
    display.setCursor(65, 24);
    display.println("Capacity:");
    display.setCursor(65, 36);
    display.print(capacity, 2);
    display.println(" Ah");
 
  }


  // ENERGY COST
  Blynk.virtualWrite(V6, String("Rs.") + String(energyCost, 5) );
  display.setCursor(10, 54);
  display.print("E Cost:Rs ");
  display.println(energyCost, 5);
  display.display();
}
