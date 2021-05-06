#include <Time.h>
#include <TimeLib.h>
//-- Knihovna U8G2 od Olikraus
//-- https://github.com/olikraus/u8g2/wiki
#include <U8g2lib.h>
#include <U8x8lib.h>

//-- Konstrukce displeje SSD1306, FULL BUFFER(1024b) SW, I2C, bez reset pinu
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);

//-- DEFINICE URCENE K NASTAVENI PROGRAMU

//-- Zde se nastavi pocet tiku (sepnuti senzoru magnetem) pro jednu celou otocku jednoho pedalu
//-- Tedy otocka treba leveho pedalu o 360 stupnu
#define FULLSENSORTICKS 8.0
//-- Cas, v sekundach, pro detekci neaktivity
//-- Jde o casovy usek, po ktery se kontroluje aktivita na zaklade poctu tiku
//-- Pokud za tento cas nebude splnena alespon jedna otocka pedalu (FULLSENSORTICKS), prestane se pocitat
#define INACTIVETIMESECOND 4
//-- Silne diskutabilni hodnota MET ovlivnujici pocet spalenych kalorii
//-- Pokud jde o elliptical, mela by byt hodnota mezi 4.6-5.7,, coz ovsem prilis nesedi.
//-- Je to tedy lepsi brat jako silu odporu stroje samotneho, kde mi hodnota vysla prave nejlepe u cisla 7
//-- Hodnotu jsem porovnaval dle sve vahy a delky cviceni podle tabulek z netu (z jejich prumeru)
#define MET 7.0
//-- Vaha v kilogramech cloveka, ktery bude stroj pouzivat
#define YOURKG 78.0



//-- Pomocne promenne | NEMENIT !
unsigned int deltaSecond = 0;
unsigned long nowTime = 0;
unsigned long deltaTime = 0;
unsigned long deltaTotalTime = 0;
unsigned long deltaSensor = 0;
float deltaPD = 0;
String totalTime;
String totalKCAL;
String totalKJ;
String totalPD;
String helpHour;
String helpMin;
String helpSec;
//-- BurnedCaloriesPerMinute = BCPM
float BCPM = (float)(((float)MET*(float)YOURKG*(float)3.5)/(float)200);
bool isMove = false;

//-- Volano priblizne kazdou sekundu
void TheApproxSecond()
{
  deltaSecond++;
  if ( deltaSecond >= INACTIVETIMESECOND )
  {
    deltaSecond = 0;
    if ( ((float)deltaSensor/(float)FULLSENSORTICKS) >= (float)1.0 )
    {
      deltaSensor = 0;
      isMove = true;
    }
    else
    {
      deltaSensor = 0;
      isMove = false;
    }
  }
  if ( isMove )
  {
    deltaTotalTime++;
  }
  totalTime = "";
  totalKCAL = "CA: ";
  totalKJ   = "kJ: ";
  totalPD   = "PD: ";
  totalKCAL += String((BCPM/(float)60)*(float)deltaTotalTime).toFloat();
  totalKJ += String((((BCPM/(float)60)*((float)deltaTotalTime/(float)1000)*(float)4184))).toFloat();
  totalPD += String(deltaPD/(float)FULLSENSORTICKS).toFloat();
  helpHour = String(hour(deltaTotalTime),DEC);
  helpMin = String(minute(deltaTotalTime),DEC);
  helpSec = String(second(deltaTotalTime),DEC);
  if ( helpHour.length() < 2 )
    totalTime += "0";
  totalTime += helpHour;
  totalTime += ":";
  if ( helpMin.length() < 2 )
    totalTime += "0";
  totalTime += helpMin;
  totalTime += ":";
  if ( helpSec.length() < 2 )
    totalTime += "0";
  totalTime += helpSec;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso16_tn);
  u8g2.setFontPosTop();
  u8g2.drawStr(28,0,totalTime.c_str());
  u8g2.setFont(u8g2_font_pxplusibmvga9_mr);
  u8g2.setFontPosTop();
  u8g2.drawStr(15,22,totalKCAL.c_str());
  u8g2.drawStr(15,37,totalKJ.c_str());
  u8g2.drawStr(15,52,totalPD.c_str());
  u8g2.sendBuffer();
}

//-- Tato funkce "bere" ostatni volny cas smycky loop()
//-- Prave zde se testuje zda-li byl sepnut kontakt na stroji
//-- Pokud je detekovano sepnuti, dojde k zacykleni a ceka se na rozepnuti. 
//-- Jakmile k tomu dojde, bere se to jako splneni jednoho tiku
void SensorTime()
{
  if ( analogRead(A0) > 220 )
  {
    while ( analogRead(A0) > 200 )
    {
      }
    deltaSensor++;
    if (isMove)
      deltaPD += (float)deltaSensor;
  }
}

//-- Nastaveni programu & desky
void setup() 
{
  Serial.begin(9600);
  pinMode(A0, INPUT_PULLUP);
  analogWrite(A0,LOW);
  u8g2.begin();
  u8g2.clear();
  //----
  //----
  deltaTime = millis();
}

//-- A JEDEME
void loop() 
{
  nowTime = millis();
  if ( (nowTime-deltaTime) >= 1000 )
  {
    deltaTime = nowTime;
    TheApproxSecond();     
  }
  else
  {
    SensorTime();
  }
}
