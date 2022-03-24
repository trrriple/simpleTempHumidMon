#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

Adafruit_AHTX0 aht;
#define AHT20_POW_PIN 11

#define OLED_SCREEN_ADDRESS 0x3C
#define OLED_SCREEN_WIDTH 128
#define OLED_SCREEN_HEIGHT 64
#define OLED_MAX_ON_TIME_MS 5000 /* How how to let OLED stay on for */
#define OLED_POW_PIN 10

SSD1306AsciiWire oled;

#define NUM_AVG_SAMPLES 20
static float tempSamples[NUM_AVG_SAMPLES];
static float humSamples[NUM_AVG_SAMPLES];
static uint8_t sampleI;

/* Voltage divider is 1/2. 
    Full AA ~ 1.6 * 3 = 4.8 v 
    Empty AA ~1.4 * 3 = 4.2 v
    With divider that puts us at 2.4v for full and 2.1 for empty 
    ADC is 10 bit 0-3.3 so LSB = 0.0032;
    Full 2.4v  ~= 745 Actual After testing 700
    Empty 1.1v ~= 500 Actual After testing 560
    This is about a delta of 100 counts so each count is ~1%
*/
#define BATT_VOLT_MON_PIN A0
#define BATT_VOLT_HIGH 700
#define BATT_VOLT_LOW  560

//------------------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);

    Wire.begin();
    Wire.setClock(400000L);

    pinMode(OLED_POW_PIN, OUTPUT);
    digitalWrite(OLED_POW_PIN, HIGH);
    /* give oled time to power on */
    delay(500);

    oled.begin(&Adafruit128x64, OLED_SCREEN_ADDRESS);
    oled.setFont(System5x7);
    oled.clear();

    pinMode(AHT20_POW_PIN, OUTPUT);
    digitalWrite(AHT20_POW_PIN, HIGH);
    /* give oled time to power on */
    delay(500);

    if (aht.begin())
    {
        Serial.println("Found AHT20");
        oled.println("Found AHT20");
    }
    else
    {
        Serial.println("Didn't find AHT20");
        oled.println("Didn't find AHT20");
        while (1)
        {
        };
    }

    oled.clear();
    oled.setCursor(40, 0);
    oled.print("Env Mon");
    oled.setCursor(96, 0);
    oled.print("B:");
    oled.setCursor(120, 0);
    oled.print("%");
    oled.setCursor(30, 1);
    oled.print("Instantaneous");
    oled.setCursor(0, 2);
    oled.print("Temp:");
    oled.setCursor(88, 2);
    oled.print("F");

    oled.setCursor(0, 3);
    oled.print("Hum :");
    oled.setCursor(88, 3);
    oled.print("%");

    oled.setCursor(30, 5);
    oled.print("Average (20s)");
    oled.setCursor(0, 6);
    oled.print("Temp:");
    oled.setCursor(88, 6);
    oled.print("F");
    oled.setCursor(0, 7);
    oled.print("Hum :");
    oled.setCursor(88, 7);
    oled.print("%");

}

void loop()
{
    sensors_event_t humidity, temp;

    aht.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data
    float tempF = (temp.temperature * 9.0 / 5.0) + 32;
    oled.clear(50, 88, 8, 24);
    oled.setCursor(50, 2);
    oled.print(tempF);
    oled.setCursor(50, 3);
    oled.print(humidity.relative_humidity);

    tempSamples[sampleI] = tempF;
    humSamples[sampleI] = humidity.relative_humidity;

    if (sampleI == NUM_AVG_SAMPLES - 1)
    {
        sampleI = 0;
    }
    else
    {
        sampleI++;
    }

    float avgTemp = 0;
    float avgHum = 0;
    for (int i = 0; i < NUM_AVG_SAMPLES; i++)
    {
        avgTemp += tempSamples[i];
        avgHum += humSamples[i];
    }

    avgTemp /= NUM_AVG_SAMPLES;
    avgHum /= NUM_AVG_SAMPLES;

    oled.clear(50, 88, 36, 64);
    oled.setCursor(50, 6);
    oled.print(avgTemp);
    oled.setCursor(50, 7);
    oled.print(avgHum);

    int battVoltRaw = analogRead(BATT_VOLT_MON_PIN);
    uint8_t battVoltPercent = 0;
    if(battVoltRaw >= BATT_VOLT_HIGH)
    {
        battVoltPercent = 99;
    }
    else if (battVoltRaw <= BATT_VOLT_LOW)
    {
        battVoltPercent = 0;
    }
    else
    {
        battVoltPercent = (uint8_t)(99.0 - 
                                    ((99.0 / (BATT_VOLT_HIGH - BATT_VOLT_LOW))
                                    * (BATT_VOLT_HIGH - battVoltRaw)));
    }

    Serial.println(battVoltRaw);
    oled.clear(108, 120, 0, 8);
    oled.setCursor(108, 0);
    oled.print(battVoltPercent);
    
    delay(1000);
}