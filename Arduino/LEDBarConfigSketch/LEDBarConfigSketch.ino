#include <Preferences.h>
#include "configLEDBar4.h"

Preferences preferences;
char* namespaceName = "LEDBar";

void setup() {
  Serial.begin(115200);
  delay(100);

  //Clearing persistent Memory
  preferences.begin(namespaceName, false);
  if(preferences.clear()){
    Serial.print("Clearing of Namespace: ");
    Serial.print(namespaceName);
    Serial.println(" was Successfull!");
  }else{
    Serial.print("Clearing of Namespace: ");
    Serial.print(namespaceName);
    Serial.println(" was not Successfull!");    
  }

  //Setting WiFi Behavior
  preferences.putBool("mainAP", BE_MAIN_ACCESSPOINT);
  preferences.putString("ownSsid", OWN_SSID);
  preferences.putString("ownPassword", OWN_SSID_PASSWORD);
  preferences.putString("mainPassword", MAIN_SSID_PASSWORD);
  preferences.putString("mainSsid", MAIN_SSID);
  preferences.putBytes("ipAddress", IP_ADRESS, 4);


  //Setting LED Count
  preferences.putInt("amountOfLEDs", AMOUNT_OF_LEDS);

  preferences.end();
  Serial.println("Config Loaded.");
  delay(1000);
  Serial.println("Reading back data to check.");
  preferences.begin(namespaceName, true);
  Serial.println(preferences.getBool("mainAP"));
  Serial.println(preferences.getString("ownSsid"));
  Serial.println(preferences.getString("ownPassword"));
  Serial.println(preferences.getString("mainPassword"));
  Serial.println(preferences.getString("mainSsid"));
  byte buf[4];
  preferences.getBytes("ipAddress", buf, 4);
  Serial.println(buf[0]);
  Serial.println(buf[1]);
  Serial.println(buf[2]);
  Serial.println(buf[3]);
}

void loop() {
}
