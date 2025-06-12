#include <Arduino.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#include "../include/initPins.h"
#include "../include/config.h"
#include "../include/initServer.h"
#include "../include/initWifi.h"

extern WebServer server;
extern LiquidCrystal_I2C lcd;
extern DHT dht;
extern void updateDatabase(float temp, float humidity);
extern void updateDisplay(float temp, float humidity);
volatile bool buttonPressed = false;

uint8_t displayState = 0;

float lastTemp = 0;
float lastHumidity = 0;

uint32_t lastReadTime = 0;
const uint32_t readInterval = 2000;
void setup()
{
    Serial.begin(115200);
    initPins();
    initServer();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Starting...");
}

void loop()
{
    server.handleClient();
    if (buttonPressed)
    {
        displayState = (displayState == 0) ? 1 : 0;
        buttonPressed = false;
        updateDisplay(lastTemp, lastHumidity);
    }

    uint32_t currentMillis = millis();
    if (currentMillis - lastReadTime >= readInterval)
    {
        lastReadTime = currentMillis;

        float temp = dht.readTemperature();
        float humidity = dht.readHumidity();

        if (abs(lastTemp - temp) > 0.2 || abs(lastHumidity - humidity) > 0.2)
        {
            lastTemp = temp;
            lastHumidity = humidity;
            updateDatabase(temp, humidity);
            updateDisplay(temp, humidity);
        }
    }
}