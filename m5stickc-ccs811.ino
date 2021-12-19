#include <M5StickC.h>
#include "DFRobot_CCS811.h"

String file_name = "/M5Stack/notes.txt";
DFRobot_CCS811 CCS811(&Wire, /*IIC_ADDRESS=*/0x5A);

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
  
}

void loop() {
    M5.Lcd.setCursor(0, 2, 1);
    double discharge = M5.Axp.GetIdischargeData() / 2.0;
    M5.Lcd.printf("I    :%.2f mA\r\n",discharge);

    if(CCS811.checkDataReady() == true){
        M5.Lcd.printf("eCO2 :%.4d ppm\r\n",CCS811.getCO2PPM());
        M5.Lcd.printf("TVOC :%.4d ppb\r\n",CCS811.getTVOCPPB());        
    }

    //delay cannot be less than measurement cycle
    delay(1000);
}
