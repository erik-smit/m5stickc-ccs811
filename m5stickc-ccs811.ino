#include <M5StickC.h>
#include "DFRobot_CCS811.h"

String ccs811_baseline_filename = "/M5Stack/ccs811_baseline";
DFRobot_CCS811 CCS811(&Wire, /*IIC_ADDRESS=*/0x5A);
RTC_TimeTypeDef RTC_TimeStruct;
unsigned long lastBaselineWriteHour = 0;
bool baselineSet = false;

void setup(void)
{
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
    
  Wire.begin(0, 26);
  Serial.begin(115200);
  /*Wait for the chip to be initialized completely, and then exit*/
  while(CCS811.begin() != 0){
    M5.Lcd.setCursor(0, 32, 1);
    M5.Lcd.println("failed to init chip, please check if the chip connection is fine");
    delay(1000);
  }

  /* SPIFFS */
  M5.Lcd.setCursor(0, 40, 1);
  if(SPIFFS.begin()){
    M5.Lcd.println("SPIFFS Started.");
  } else {
    M5.Lcd.println("SPIFFS failed to Start.");
    M5.Lcd.println("SPIFFS format start...");
    SPIFFS.format();
    M5.Lcd.println("SPIFFS format finish");
    if(SPIFFS.begin()){
        M5.Lcd.println("SPIFFS Started.");
    } else {
        M5.Lcd.println("SPIFFS still failed to Start.");
    }
  }

  /* RTC */
  RTC_TimeStruct.Hours   = 0;
  RTC_TimeStruct.Minutes = 0;
  RTC_TimeStruct.Seconds = 0;
  M5.Rtc.SetTime(&RTC_TimeStruct);

  M5.Lcd.fillScreen(BLACK);  
}

void loop() {
    M5.update(); // need to call update()
    M5.Lcd.setCursor(0, 2, 1);
    double discharge = M5.Axp.GetIdischargeData() / 2.0;
    M5.Lcd.printf("I    : %.2f mA\r\n",discharge);

    M5.Rtc.GetTime(&RTC_TimeStruct);
    M5.Lcd.printf(
      "Time : %02d:%02d:%02d\n", 
      RTC_TimeStruct.Hours, 
      RTC_TimeStruct.Minutes, 
      RTC_TimeStruct.Seconds);

    if (CCS811.checkDataReady() == true){
        M5.Lcd.printf("eCO2 : %.4d ppm\r\n",CCS811.getCO2PPM());
        M5.Lcd.printf("TVOC : %.4d ppb\r\n",CCS811.getTVOCPPB());        
    }

    //delay cannot be less than measurement cycle
    delay(1000);
    
    // AN000370: Do not save or restore the baseline while the sensor is still in the process of warming up.
    if(!baselineSet && RTC_TimeStruct.Hours == 0 && RTC_TimeStruct.Minutes > 20) {
      loadCCS811Baseline();
      baselineSet = true;
    }

    if (RTC_TimeStruct.Hours > lastBaselineWriteHour) {
      dumpCCS811Baseline();
      lastBaselineWriteHour = RTC_TimeStruct.Hours;
    }
    
    if (M5.BtnA.wasPressed()) {
      dumpCCS811Baseline();
    }
    if (M5.BtnB.wasPressed()) {
      loadCCS811Baseline();
    }
}

void timePrefix(String text) {
  M5.Lcd.printf(
    "%02d:%02d:%02d %s", 
    RTC_TimeStruct.Hours, 
    RTC_TimeStruct.Minutes, 
    RTC_TimeStruct.Seconds, 
    text.c_str());
}

void loadCCS811Baseline() {
  M5.Lcd.fillScreen(BLACK);  
  M5.Lcd.setCursor(0, 40, 1);
  if (SPIFFS.exists(ccs811_baseline_filename)){
    File CBF = SPIFFS.open(ccs811_baseline_filename, FILE_READ);
    size_t len = 2;
    uint16_t sbuf;
    CBF.readBytes((char *)&sbuf, len);
    CCS811.writeBaseLine(sbuf);
    timePrefix("baseline loaded.\n");
    M5.Lcd.printf("Value: %04x\r\n", __builtin_bswap16(sbuf));
  } else {
    timePrefix("baseline not found.\n");
  }
}

void dumpCCS811Baseline() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 40, 1);  
  File CBF = SPIFFS.open(ccs811_baseline_filename, FILE_WRITE);
  uint16_t baseline;
  baseline = CCS811.readBaseLine();
  CBF.write(baseline & 0xff);
  CBF.write(baseline >> 8 & 0xff);
  timePrefix("baseline written.\n");
  M5.Lcd.printf("Value: %04x\r\n", __builtin_bswap16(baseline));
}
