#define ENABLE_GxEPD2_GFX 0
#define MIN_MAX_SCREEN_DELAY 10
#define DISPLAY_ARDUINO_I2C_ADDRESS 50
#define BUFFER_SIZE 70

#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <EasyButton.h>
#include "GxEPD2_boards_added.h"
#include <LowPower.h>



#define MAX_DISPLAY_BUFFER_SIZE 400 // 800
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
GxEPD2_BW<GxEPD2_290, MAX_HEIGHT(GxEPD2_290)> display(GxEPD2_290(/*CS=10*/ SS, /*DC=*/ 8, /*RST=*/ 9, /*BUSY=*/ 7));


U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
#define SMALL_FONT u8g2_font_helvR12_tf
#define LARGE_FONT u8g2_font_inr16_mn

// serial debug output enabled
volatile bool debug = true;
volatile bool screenRefresh = false;

// memory reset pin
#define MEM_RESET_PIN 2

float temperature_in = -99.9; // deg. C
byte humidity_in = -120; // %

float temperature_out = -99.9; // deg. C

float battery_voltage = -12.357; // V
int battery_current = -9999; // mA

unsigned int light_intensity = 65535; // lux
int air_pressure = -1012; // hPa

// input data buffer
char buff[BUFFER_SIZE];

struct MinMaxValues {
  float temp_in_max;
  float temp_in_min;
  byte humidity_in_max;
  byte humidity_in_min;
  float temp_out_max;
  float temp_out_min;
  int air_pressure_max;
  int air_pressure_min;
  unsigned int light_intensity_max;
  unsigned int light_intensity_min;
};

MinMaxValues minMaxValues;


void drawScreen(byte screen)
{
  screenRefresh = true;
  uint16_t bg = GxEPD_WHITE;
  uint16_t fg = GxEPD_BLACK;
  u8g2Fonts.setFontMode(1);                 // use u8g2 transparent mode (this is default)
  u8g2Fonts.setFontDirection(0);            // left to right (this is default)
  u8g2Fonts.setForegroundColor(fg);         // apply Adafruit GFX color
  u8g2Fonts.setBackgroundColor(bg);         // apply Adafruit GFX color
  // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
  uint16_t x =  20;
  uint16_t y =  20;
  byte y_offset = 47;
  display.firstPage();
  do
  {
    display.fillScreen(bg);
    if (screen == 0) {
      break;
    }
    // temperature inside
    x =  20;
    y =  15;
    u8g2Fonts.setFont(SMALL_FONT);
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.println("teplota uvnitr");
    switch (screen) {

      case 1:
        u8g2Fonts.setFont(LARGE_FONT);
        u8g2Fonts.setCursor(x + 3, y + 25);
        u8g2Fonts.print(temperature_in, 1);
        u8g2Fonts.setFont(SMALL_FONT);
        u8g2Fonts.println(" st. C");
        break;
      case 2:
        u8g2Fonts.setCursor(x - 10, y + 17);
        u8g2Fonts.print("max ");
        u8g2Fonts.print(minMaxValues.temp_in_max, 1);
        u8g2Fonts.print(" C");

        u8g2Fonts.setCursor(x - 10, y + 31);
        u8g2Fonts.print("min ");
        u8g2Fonts.print(minMaxValues.temp_in_min, 1);
        u8g2Fonts.print(" C");
        break;
      case 127:
      u8g2Fonts.setCursor(x + 17, y + 25);
      u8g2Fonts.print("RESET");
      break;
    }

    // humidity inside
    x =  18;
    y +=  y_offset;
    u8g2Fonts.setFont(SMALL_FONT);
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.println("vlhkost uvnitr");
    switch (screen) {
    case 1:
        u8g2Fonts.setFont(LARGE_FONT);
        u8g2Fonts.setCursor(x + 20, y + 27);
        u8g2Fonts.print(humidity_in);
        u8g2Fonts.setFont(SMALL_FONT);
        u8g2Fonts.println(" %");
        break;
    case 2:
        u8g2Fonts.setCursor(x - 10, y + 17);
        u8g2Fonts.print("max ");
        u8g2Fonts.print(minMaxValues.humidity_in_max);
        u8g2Fonts.print(" %");

        u8g2Fonts.setCursor(x - 10, y + 31);
        u8g2Fonts.print("min ");
        u8g2Fonts.print(minMaxValues.humidity_in_min);
        u8g2Fonts.print(" %");
        break;
      case 127:
      u8g2Fonts.setCursor(x + 17, y + 25);
      u8g2Fonts.print("RESET");
      break;
}

    // temperature out
    x =  18;
    y +=  y_offset;
    u8g2Fonts.setFont(SMALL_FONT);
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.println("teplota venku");
    switch (screen) {
      case 1:
        u8g2Fonts.setFont(LARGE_FONT);
        u8g2Fonts.setCursor(x + 3, y + 25);
        u8g2Fonts.print(temperature_out, 1);
        u8g2Fonts.setFont(SMALL_FONT);
        u8g2Fonts.println(" st. C");
        break;
      case 2:
        u8g2Fonts.setCursor(x - 10, y + 17);
        u8g2Fonts.print("max ");
        u8g2Fonts.print(minMaxValues.temp_out_max, 1);
        u8g2Fonts.print(" C");

        u8g2Fonts.setCursor(x - 10, y + 31);
        u8g2Fonts.print("min ");
        u8g2Fonts.print(minMaxValues.temp_out_min, 1);
        u8g2Fonts.print(" C");
        break;
      case 127:
      u8g2Fonts.setCursor(x + 17, y + 25);
      u8g2Fonts.print("RESET");
      break;
}
    // light intensity
    x =  18;
    y +=  y_offset;
    u8g2Fonts.setFont(SMALL_FONT);
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.println("osvetleni");
    switch (screen) {
      case 1:
        u8g2Fonts.setFont(LARGE_FONT);
        u8g2Fonts.setCursor(x + 3, y + 25);
        u8g2Fonts.print(light_intensity);
        u8g2Fonts.setFont(SMALL_FONT);
        u8g2Fonts.println(" lux");
        break;
      case 2:
        u8g2Fonts.setCursor(x - 10, y + 17);
        u8g2Fonts.print("max ");
        u8g2Fonts.print(minMaxValues.light_intensity_max);
        u8g2Fonts.print(" lux");

        u8g2Fonts.setCursor(x - 10, y + 31);
        u8g2Fonts.print("min ");
        u8g2Fonts.print(minMaxValues.light_intensity_min);
        u8g2Fonts.print(" lux");
        break;
      case 127:
      u8g2Fonts.setCursor(x + 17, y + 25);
      u8g2Fonts.print("RESET");
      break;
}
    // atmospheric pressure
    x =  18;
    y +=  y_offset;
    u8g2Fonts.setFont(SMALL_FONT);
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.println("tlak vzduchu");
    switch (screen) {
      case 1:
        u8g2Fonts.setFont(LARGE_FONT);
        u8g2Fonts.setCursor(x + 3, y + 25);
        u8g2Fonts.print(air_pressure);
        u8g2Fonts.setFont(SMALL_FONT);
        u8g2Fonts.println(" hPa");
        break;
      case 2:
        u8g2Fonts.setCursor(x - 10, y + 17);
        u8g2Fonts.print("max ");
        u8g2Fonts.print(minMaxValues.air_pressure_max);
        u8g2Fonts.print(" hPa");

        u8g2Fonts.setCursor(x - 10, y + 31);
        u8g2Fonts.print("min ");
        u8g2Fonts.print(minMaxValues.air_pressure_min);
        u8g2Fonts.print(" hPa");
        break;
      case 127:
      u8g2Fonts.setCursor(x + 17, y + 25);
      u8g2Fonts.print("RESET");
      break;
}
    // battery status
    x =  0;
    y +=  y_offset + 10;
    u8g2Fonts.setFont(SMALL_FONT);
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.print(" napeti: ");
    u8g2Fonts.print(battery_voltage, 1);
    u8g2Fonts.println(" V");
    u8g2Fonts.print(" proud: ");
    u8g2Fonts.print(battery_current);
    u8g2Fonts.println(" mA");
    u8g2Fonts.print(" vykon: ");
    u8g2Fonts.print(0.001 * battery_current * battery_voltage, 3);
    u8g2Fonts.println(" W");


  }
  while (display.nextPage());
  screenRefresh = false;
  switch (screen) { // screen timeout
    case 2:
      delay(MIN_MAX_SCREEN_DELAY * 1000);
      break;
    case 127:
      delay(3000);
      break;
    default:
      break;

  }
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

void receiveEvent(int howMany) {


  int t_o, t_i, p_i, volt, curr, pow;

  unsigned int li;
  byte h_i;
  byte index = 0;
  while (Wire.available()) {
    buff[index] = Wire.read();
    index++;
 }

  sscanf(buff, "%04X%04X%02X%04X%04X%04X%04X%04X", &t_o, &t_i, &h_i, &p_i, &li, &volt, &curr, &pow);
  temperature_in = float(t_i / 10.0);
  humidity_in = h_i;
  temperature_out = float(t_o / 10.0);
  battery_voltage = float(volt * 0.001);
  battery_current = curr;
  light_intensity = li;
  air_pressure = p_i;

  if (debug) {
    Serial.print("Temp in: ");
    Serial.print(temperature_in, 1);
    Serial.print(" C | ");

    Serial.print("Humidity: ");
    Serial.print(humidity_in);
    Serial.print(" % | ");

    Serial.print("Temp out: ");
    Serial.print(temperature_out, 1);
    Serial.print(" C | ");

    Serial.print("Pressure: ");
    Serial.print(air_pressure);
    Serial.print(" hPa | ");

    Serial.print("Light: ");
    Serial.print(light_intensity);
    Serial.print(" lux | ");

    Serial.print("Voltage: ");
    Serial.print(battery_voltage, 1);
    Serial.print(" V | ");

    Serial.print("Current: ");
    Serial.print(battery_current);
    Serial.print(" mA | ");

    Serial.print("Power: ");
    Serial.print(battery_current * battery_voltage, 0);
    Serial.println(" mW");
  }
  screenRefresh = true;
}

void setup()
{

  // clock_prescale_set(clock_div_2);

  Serial.begin(19200);

  // read EEPROM stored min/max values
  EEPROM.get(0, minMaxValues);
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);

  display.init(); // init e-ink display module
  u8g2Fonts.begin(display); // connect u8g2 procedures to Adafruit GFX

  Wire.begin(DISPLAY_ARDUINO_I2C_ADDRESS);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event

  drawScreen(0);

}

void loop()
{
  delay(200);
  if (screenRefresh) {
    screenRefresh = false;
    drawScreen(1);
  }
}
