/**The MIT License (MIT)

Copyright (c) 2016 by Seokjin Seo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

See more at http://usemodj.com

 Required libraries:
  - XPT2046: https://github.com/spapadim/XPT2046
  - ESP8266_UTFT: https://github.com/usemodj/ESP8266_UTFT

 Modify: XPT2046.cpp
 void XPT2046::begin(uint16_t width, uint16_t height) { 
   ...
   //SPI.begin(); //comment out
   ...
 }  

 보건기상지수
기후변화에 능동적으로 신체를 대처할수 있는 보건기상지수(뇌졸중가능지수, 피부질환가능지수, 감기가능지수, 꽃가루농도위험지수 등 5종)를 특정위치(지점번호)별로 조회할 수 있는 서비스 입니다. 

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <UTFT.h>
#include <UTFTUi.h>
#include <SPI.h>
#include <XPT2046.h>
#include "NTPTimeClient.h"
#include "BrainStrokeIndexJsonClient.h"
#include "ImageFont.h"

#define ESP_SPI_FREQ 4000000
/***************************
 * Begin Settings
 **************************/
// WIFI
const char* WIFI_SSID = "*******"; 
const char* WIFI_PWD = "*******";

// 보건기상지수 Settings
// *  www.data.go.kr
// *    OPEN API: 보건기상지수(뇌졸중가능지수)
const String SERVICE_KEY = "****************";
const String AREA_NO = "4615063500"; //지점코드(도사동:순천만)

// Setup
const int UPDATE_INTERVAL_SECS = 30 * 60; // Update every 30 minutes
// TimeClient settings
const float UTC_OFFSET = 9; //South Korea (Seoul)

// Display Settings
const byte LCD_MODEL = ILI9341_S5P;
const int LCD_CS = 15;
const int LCD_RST = 5;
const int LCD_SER = 2;
// Modify the line below to match your display and wiring:
//UTFT(byte model, int CS, int RST, int SER=0);
//UTFT display( ILI9341_S5P, 15, 5, 2);
UTFT display( LCD_MODEL, LCD_CS, LCD_RST, LCD_SER);
UTFTUi ui(&display);
XPT2046 myTouch(/*cs=*/ 4, /*irq=*/ 5);

// Declare which fonts we will be using
extern uint8_t BigFont[];
extern uint8_t SmallFont[];
extern uint8_t SourceCodePro_32[];
extern uint8_t SourceCodePro_23[];
extern uint8_t SourceCodePro_11[];

NTPTimeClient timeClient(UTC_OFFSET);

BrainStrokeIndexJsonClient brainStrokeIndex;

bool drawFrame1(UTFT *display, UTFTUiState* state, int x, int y);
bool drawFrame2(UTFT *display, UTFTUiState* state, int x, int y);
// this array keeps function pointers to all frames
// frames are the single views that slide from BOTTOM to TOP
bool (*frames[])(UTFT *display, UTFTUiState* state, int x, int y) = { drawFrame1, drawFrame2};
int numberOfFrames = 2;

// flag changed in the ticker function every 10 minutes
bool readyForUpdate = false;
String lastUpdate = "--";
Ticker ticker;

/*************************
**   Custom functions   **
*************************/
void drawProgress(UTFT *display, int percentage, String label) {
  int centerX = display->getDisplayXSize()/2;
  int centerY = display->getDisplayYSize()/2;
  display->setTextAlignment(CENTER);
  display->setFont(SmallFont);
  display->setColor(VGA_BLACK);
  display->fillRect(0, centerY-15, 320, centerY);
  display->setColor(VGA_WHITE);
  display->drawString(label, centerX, centerY - 15);
  display->drawRect(centerX - 100, centerY, centerX + 100, centerY + 16);
  display->fillRect(centerX - 100, centerY + 2, centerX - 100 + 200 * percentage / 100 , centerY + 14);
}

void updateData(UTFT *display) {
  drawProgress(display, 10, "Updating time...");
  timeClient.updateTime();

  drawProgress(display, 50, "Updating brain stroke index...");
  brainStrokeIndex.updateBrainStrokeIndex(SERVICE_KEY, AREA_NO);
  lastUpdate = timeClient.getFormattedTime();
  readyForUpdate = false;
  drawProgress(display, 100, "Done...");
  delay(1000);
}

void drawBarGraph(UTFT *display, int x, int y){
  int Y = display->getDisplayYSize() - 40;
  int W = (display->getDisplayXSize() - 80) / 4;
  display->setColor(VGA_WHITE);
  display->fillRect(40 + x, Y - 20 + y, 41+W + x, Y + y); // 0 Low
  display->setColor(VGA_GREEN);
  display->fillRect(41+W + x, Y - 40 + y, 41+W*2 + x, Y + y); // 1 Normal
  display->setColor(VGA_YELLOW);
  display->fillRect(41+W*2 + x, Y - 60 + y, 41+W*3 + x, Y + y); // 2 High
  display->setColor(VGA_RED);
  display->fillRect(41+W*3 + x, Y - 80 + y, 41+W*4 + x, Y + y); // 3 Very High
  
  display->setColor(VGA_GREEN);
  display->setFont(SmallFont);
  display->setBackColor(VGA_TRANSPARENT);
  display->setTextAlignment(CENTER);
  display->drawString("Low", 40+W/2 + x, Y -15 + y);
  display->setColor(VGA_WHITE);
  display->drawString("Normal", 40+W+W/2 + x, Y -15 + y);
  display->drawString("High", 40+W*2+W/2 + x, Y -15 + y);
  display->drawString("Very", 40+W*3+W/2 + x, Y -25 + y);
  display->drawString("High", 40+W*3+W/2 + x, Y -15 + y);
  //display->drawStringMaxWidth("Very High", 40+W*3+W/2 + x, Y-25 + y, W/2);
  
  display->setColor(VGA_WHITE);
  display->setFont(BigFont);
  for(int i = 0; i < 4; i++){
    display->printNumI(i, 40 + W*i + W/2 + x, Y+5 + y);
  }
}

bool drawFrame1(UTFT *display, UTFTUiState* state, int x, int y) {
  int X = display->getDisplayXSize()/2;
  display->setTextAlignment( CENTER);
  display->setFont(BigFont);
  display->drawString("Brain Stroke Index", X + x, 3 + y);

  String date = timeClient.getMonth() + "/" + timeClient.getDay();
  display->setFont(SourceCodePro_32);
  display->drawString(date, X + x, 33 + y);
  
  //display->setFont(BigFont);
  String time = timeClient.getFormattedTime();
  display->drawString(time, X + x, 73 + y);

  display->drawBitmap(X - 40 + x, 113 + y, 80, 80, brain80);
  return false;
}

bool drawFrame2(UTFT *display, UTFTUiState* state, int x, int y) {
  display->setFont(BigFont);
  display->setColor(VGA_YELLOW);
  display->setTextAlignment(CENTER);
  display->drawString("Brain Stroke Index", display->getDisplayXSize()/2 + x, 3 + y);
  display->drawBitmap(210 + x, 33 + y, 80, 80, brain80);
  display->setColor(VGA_WHITE);
  display->setTextAlignment(LEFT);
  int todayWidth = display->getStringWidth("Today: ");
  int tomorrowWidth = display->getStringWidth("Tomorrow: ");
  int nextDayWidth = display->getStringWidth("Next Day: ");
  display->drawString("Today: ", 3 + x, 33 + y);
  display->drawString("Tomorrow: ", 3 + x, 60 + y);
  display->drawString("Next Day: ", 3 + x, 87 + y);
  display->setFont(BigFont);
  display->setColor(VGA_RED);
  display->printNumI(brainStrokeIndex.getBrainStrokeIndex().today, 3 + todayWidth + x, 33 + y);
  display->printNumI(brainStrokeIndex.getBrainStrokeIndex().tomorrow, 3 + tomorrowWidth + x, 60 + y);
  display->printNumI(brainStrokeIndex.getBrainStrokeIndex().dayAfterTomorrow, 3 + nextDayWidth + x, 87 + y);
  drawBarGraph(display, x, y);
  return false;
}

/*************************
**  Required functions  **
*************************/

void setup()
{
  delay(1000);
  
  Serial.begin(115200);
  SPI.setFrequency(ESP_SPI_FREQ);

// Initialize display
  display.InitLCD(LANDSCAPE); //LANDSCAPE, PORTRAIT
  display.clrScr();
  Serial.print("tftx ="); Serial.print(display.getDisplayXSize()); Serial.print(" tfty ="); Serial.println(display.getDisplayYSize());
  //begin(uint16_t width, uint16_t height);  // width and height with no rotation!
  //myTouch.begin(240, 320);  // Must be done before setting rotation
  if (display.getOrient() == PORTRAIT){
    myTouch.begin((uint16_t)display.getDisplayXSize(), (uint16_t)display.getDisplayYSize());  // Must be done before setting rotation
   
  } else {
    myTouch.begin((uint16_t)display.getDisplayYSize(), (uint16_t)display.getDisplayXSize());  // Must be done before setting rotation
  }
  // Replace these for your screen module
  myTouch.setCalibration(224, 1744, 1750, 272);
  myTouch.setRotation(myTouch.ROT270);

  display.setFont(SmallFont); 
  //display.setBackColor(0, 0, 255);
  //display.fillScr(0, 0, 255);
  //display.fillScr(VGA_OLIVE);


  WiFi.begin(WIFI_SSID, WIFI_PWD);
  
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.clrScr();
    display.print("Connecting to WiFi", display.getDisplayXSize()/2, 10);
    display.drawXbm(46, 30, 8, 8, counter % 3 == 0 ? activeSymbole : inactiveSymbole);
    display.drawXbm(60, 30, 8, 8, counter % 3 == 1 ? activeSymbole : inactiveSymbole);
    display.drawXbm(74, 30, 8, 8, counter % 3 == 2 ? activeSymbole : inactiveSymbole);
   
    counter++;
  }

  ui.setTargetFPS(30);
  ui.setTimePerFrame(10000); //10sec
  ui.setActiveSymbole(activeSymbole);
  ui.setInactiveSymbole(inactiveSymbole);
  // You can change this to
  // TOP_POS, LEFT_POS, BOTTOM_POS, RIGHT_POS
  ui.setIndicatorPosition(BOTTOM_POS);
  //ui.setIndicatorPosition(LEFT_POS);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);
  //ui.setFrameAnimation(SLIDE_UP);

  // Add frames
  ui.setFrames(frames, numberOfFrames);

  // Inital UI takes care of initalising the display too.
  //ui.InitLCD();

  // Flip Screen Vertically
  // display.flipScreenVertically();
  
  Serial.println("");

  updateData(&display);

  ticker.attach(UPDATE_INTERVAL_SECS, setReadyForUpdate);
}

void loop()
{
  // put your main code here, to run repeatedly:
  if(readyForUpdate && ui.getUiState().frameState == FIXED) {
    updateData(&display);
  }

  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }

  //yield();
}

void setReadyForUpdate() {
  Serial.println("Setting readyForUpdate to true");
  readyForUpdate = true;
}

