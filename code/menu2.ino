#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include "DFRobot_RainfallSensor.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    4
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LED_VERTE 5
#define LED_JAUNE 4
#define LED_ROUGE 3

#define BUTTON_PIN 3
#define DHT_PIN 2
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

#define POTENTIOMETER_PIN A0
const int ldrPin = A1;
DFRobot_RainfallSensor rainfallSensor(A4);

const int menuSize = 5;
String menuItems[menuSize] = {"Pluviometre", "Luxmetre", "Pollution", "Thermometre", "Anemometre"};

int menuOption = 0;
float potValueSmoothed = 0;
const float smoothingFactor = 0.2;
const float referenceResistor = 10000;
const float ldrVoltage = 5.0;

bool lastButtonState = HIGH;
unsigned long displayTime = 0;
const unsigned long displayDuration = 3000;
bool displayingMessage = false;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_ROUGE, OUTPUT);
  pinMode(LED_JAUNE, OUTPUT);
  pinMode(LED_VERTE, OUTPUT);

  digitalWrite(LED_ROUGE, LOW);
  digitalWrite(LED_JAUNE, LOW);
  digitalWrite(LED_VERTE, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000);
  display.clearDisplay();
  dht.begin();
}

void loop() {
  if (!displayingMessage) {
    int potValue = analogRead(POTENTIOMETER_PIN);
    potValueSmoothed = potValueSmoothed * (1 - smoothingFactor) + potValue * smoothingFactor;
    int newMenuOption = (int)(potValueSmoothed / (1024 / menuSize));
    newMenuOption = min(newMenuOption, menuSize - 1);

    if (newMenuOption != menuOption) {
      menuOption = newMenuOption;
      updateDisplay();
    }

    bool buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW && lastButtonState == HIGH) {
      displayingMessage = true;
      displayTime = millis();

      if (menuOption == 0) {
        float rainfall = rainfallSensor.getRainfall(); 

        display.clearDisplay();
        display.setCursor(0, 0);
        display.print(rainfall);
        display.println(" mm/m2/h");
        display.display();
      }

      else if (menuOption == 1) { // L'option Luxmètre est sélectionnée
        int ldrValue = analogRead(ldrPin);
        float voltage = ldrValue * (ldrVoltage / 1023.0);
        float ldrResistance = (ldrVoltage - voltage) * referenceResistor / voltage;
        float lux = 500 / (ldrResistance / 1000);

        display.clearDisplay();
        display.setCursor(0, 0);
        display.print(lux+22);
        display.println(" lux");
        display.display();
      }

      else if (menuOption == 2) { // L'option Pollution est sélectionnée
        int mesurePollution = analogRead(POTENTIOMETER_PIN); // Utilisez le pin approprié
        int mesureConv = mesurePollution / 2;

        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("Valeur mesuree: ");
        display.println(mesureConv);

        if (mesureConv < 80) {
          digitalWrite(LED_VERTE, HIGH);
          digitalWrite(LED_JAUNE, LOW);
          digitalWrite(LED_ROUGE, LOW);
          display.println("AIR CORRECT");
        } else if (mesureConv < 140) {
          digitalWrite(LED_VERTE, LOW);
          digitalWrite(LED_JAUNE, HIGH);
          digitalWrite(LED_ROUGE, LOW);
          display.println("AIR MOYEN");
        } else {
          digitalWrite(LED_VERTE, LOW);
          digitalWrite(LED_JAUNE, LOW);
          digitalWrite(LED_ROUGE, HIGH);
          display.println("AIR MAUVAIS");
        }
        display.display();
      } 

      else if (menuOption == 3) { // L'option Thermomètre est sélectionnée
        float temp = dht.readTemperature();
        float hum = dht.readHumidity();

        display.clearDisplay();
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.print(temp);
        display.println(" deg");
        display.print(hum);
        display.println(" %");
        display.display();
      } else {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Option #:");
        display.println(menuOption + 1);
        display.display();
      }
    }
    lastButtonState = buttonState;
  } 

  /*else if (menuOption == 4) { // L'option Anémomètre est sélectionnée
        // Mettez ici le code pour afficher le message "capteur non présent"
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("capteur non present");
        display.display();
  }*/

  else {
    if (millis() - displayTime > displayDuration) {
      displayingMessage = false;
      updateDisplay();
    }
  }
}

void updateDisplay() {
  if (!displayingMessage) { // Mettez à jour l'affichage uniquement si nous ne sommes pas en train d'afficher un message de menu spécifique
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    for(int i = 0; i < menuSize; i++) {
      if(i == menuOption) {
        display.print("> ");
      } else {
        display.print("  ");
      }
      display.println(menuItems[i]);
    }
    display.display();
  }
}
