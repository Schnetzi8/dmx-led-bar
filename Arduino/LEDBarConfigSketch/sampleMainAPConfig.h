//Sample Config for a Main Accesspoint Controller

//Config-WiFi
//Wether or not this Controller should provide the WiFi Network for all other Controllers to connect to for configuration.
const bool BE_MAIN_ACCESSPOINT = true;

//SSID for the Network this controller provides. Either as Main Access Point or in case it needs to create its own fallback Network when the main Access Point is not available
const String OWN_SSID = "MainController";

//Password for OWN_SSID
const String OWN_SSID_PASSWORD = "****";

//SSID to connect to Main Access Point. Is ignored if the Controller is the Main Access Point.
const String MAIN_SSID = "";

//Password for MAIN_SSID. Is equally ignored if the Controller is the Main Access Point
const String MAIN_SSID_PASSWORD = "****";

//IP Adress of the Controller. If this Controller isn't the Main Acces Point, the Address should be between 192.168.4.11 and 192.168.4.254 as the DHCP Server on the Accespoint may assing IP's between 192.168.4.1 and 192.168.4.10 to connecting clients.
byte IP_ADRESS[] = {1, 4, 168, 192};

//Config-LED
//Amount of LEDs on the Bar
const int AMOUNT_OF_LEDS = 120;

//End Config