#include <Arduino.h>
#include <ESPAsyncWebSrv.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>
#include <esp_dmx.h>
#include "website.h"

//WiFi fields
String APSsid = "";
String APPassword = "";
String mainSsid = "";
String mainPassword = "";
bool mainAP = false;
bool wiFiActive = false;
bool wiFiConnecting = false;
IPAddress ipAddress;
IPAddress NMask(255, 255, 255, 0);
long wiFiConnectTimeOut = 0;
long lastConnectAttempt = 0;
long wiFiConnectionTime = 0;

//WebServer fields
AsyncWebServer server(80);
bool keepWebSrv = false;

//LED fields
int numLeds = 120;
const int dataPin = 5;
Adafruit_NeoPixel pixels(numLeds, dataPin, NEO_GRB + NEO_KHZ800);
int segments = 1;
bool identify = false;

//Persistence fields
Preferences preferences;
char* namespaceName = "LEDBar";

//DMX Fields
int transmitPin = 4;
int receivePin = 3;
int enablePin = DMX_PIN_NO_CHANGE;
dmx_port_t dmxPort = 0;
byte data[DMX_PACKET_SIZE];
dmx_packet_t packet;
bool dmxIsConnected = false;
int dmxStartAdress = 0;
int maxDmxStartAdress = 0;

void setup() {

  Serial.begin(115200);
  delay(1000);
  
  configurePersistence();

  configureWiFi();
  configureLEDs();
  configureDMX();
}

void loop() {
  identifyUpdate();
  dmxUpdate();
  webServerUpdate();
}

//WiFi methods:
void configureWiFi() {
  if(mainAP){
    configureAcessPoint();
    return;
  }
  connectToMainSsid();
}

void connectToMainSsid(){
  WiFi.begin(mainSsid, mainPassword);
  wiFiConnectTimeOut = millis() + 60000;
  wiFiConnecting = true;
}

void configureAcessPoint(){
  Serial.println("-Configuring access point");

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(APSsid, APPassword)) {
    Serial.println("--Soft AP creation failed.");
    Serial.println("--Operation of the LED Bar is still possible with the previous configuration.");
    return;
  }
  delay(100);
  WiFi.softAPConfig(ipAddress, ipAddress, NMask);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("--AP IP address: ");
  Serial.println(myIP);
  Serial.println("Access point configuration finished-");
  mainAP = true;
  wiFiReady();
}

//WebServer methods:
void notFound(AsyncWebServerRequest* request) {
  request->send(404, "text/plain", "Not found");
}

void configureWebServer() {
  if(!wiFiActive){
    Serial.println("-Skipping Webserver Configuration because of missing WiFi");
    return;
  }
  Serial.println("-Configuring WebServer");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    if(request->hasParam("identify")){
      AsyncWebServerResponse *response = request->beginResponse(204);
      request->send(response);
      identify = true;
      return;
    }
    Serial.println("-Web Page: root");
    if (request->hasParam("dmxAdress")) {
      handleDmxAdress(request);
    }
    if (request->hasParam("segments")) {
      handleSegments(request);
    }
    handlekeepWebSrv(request);
    
    request->send(200, "text/html", getWebpage());
    Serial.println("done-");
  });

  server.onNotFound(notFound);

  server.begin();
  Serial.println("WebServer configuration finished-");
}

void webServerUpdate() {
  if(wiFiConnecting){
    handleConnectingPhase();
  }

  if(wiFiActive){
    handleWifiActivePhase();
  }
}

void handleConnectingPhase(){

  if(wiFiConnectTimeOut < millis()){
    configureAcessPoint();
    wiFiConnecting = false;
    return;
  }

  if((lastConnectAttempt + 1000) <= millis() && WiFi.status() != WL_CONNECTED){
    Serial.print("Trying to connect to ");
    Serial.println(mainSsid);
    int remainingSec = (wiFiConnectTimeOut-millis())/1000;
    Serial.print(remainingSec);
    Serial.println(" Sec. Remaining");
    lastConnectAttempt = millis();
  }
  
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("WiFi Connected");
    delay(100);
    WiFi.config(ipAddress, ipAddress, NMask);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    wiFiReady();
  }
}

void handleWifiActivePhase(){
  if(mainAP){
    if(!keepWebSrv && millis() > wiFiConnectionTime + 600000){
      Serial.println("Shutting down Web Server and WiFi Access Point");
      server.end();
      WiFi.softAPdisconnect(true);
      wiFiActive = false;
      keepWebSrv = true;
    }
    return;
  }
  if(WiFi.status() != WL_CONNECTED){
      Serial.println("Shutting down Web Server");
      server.end();
      wiFiActive = false;
      keepWebSrv = true;      
  }
}

void wiFiReady(){
  wiFiActive = true;
  wiFiConnecting = false;
  wiFiConnectionTime = millis();
  configureWebServer();
}

String getWebpage(){
  String result = content;
  result.replace("[maxAdress]", String(maxDmxStartAdress));
  result.replace("[dmxAdress]", String(dmxStartAdress));
  result.replace("[segments]", String(segments));
  result.replace("[numLeds]", String(numLeds));
  result.replace("[startAdress]", String(dmxStartAdress));
  int endAdress = dmxStartAdress -1 + 3 * segments;
  result.replace("[endAdress]", String(endAdress));
  if(mainAP){
    result.replace("[keepWebSrvInput]", keepWebServerInput);
    if(keepWebSrv){
      result.replace("[keepWebSrv]", "checked");
    }else{
      result.replace("[keepWebSrv]", "");
    }
  }else{
    result.replace("[keepWebSrvInput]", "");    
  }
  return result;
}

void handleDmxAdress(AsyncWebServerRequest* request) {
  Serial.print("--DMX start-Adress: ");
  int input = request->getParam("dmxAdress")->value().toInt();
  if(input > 0 && input <=maxDmxStartAdress){
  dmxStartAdress = input;
  setPerInt(namespaceName, "dmxStartAdress", dmxStartAdress);
  }
  Serial.print(dmxStartAdress);
  Serial.println("--");
}

void handleSegments(AsyncWebServerRequest* request) {
  Serial.print("--Segments: ");
  int input = request->getParam("segments")->value().toInt(); 
  if(input <= numLeds, input > 0){
    segments = input; 
    setPerInt(namespaceName, "segments", segments);
    maxDmxStartAdress = 512 + 1 - 3 * segments;
    if(dmxStartAdress > maxDmxStartAdress){
      dmxStartAdress = maxDmxStartAdress;
    }
  }
  Serial.print(segments);
  Serial.println("--");
}

void handlekeepWebSrv(AsyncWebServerRequest* request) {
  Serial.print("--Keep Web Server: ");
  if(request->hasParam("segments")){
  keepWebSrv = false;    
  }
  if (request->hasParam("keepWebSrv")) {
    keepWebSrv = true;
  }
  Serial.print(keepWebSrv);
  Serial.println("--");
}

//LED methods:
void configureLEDs() {
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(2000);
  pixels.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
}

void setColor(int r, int g, int b) {
  for (int i = 0; i < numLeds; i++) {
    pixels.setPixelColor(i, r, g, b);
  }
  pixels.show();
}

void setPixelsColor(int startIndex, int endIndex, int r, int g, int b){
  for(int i = startIndex; i <= endIndex; i++){
    pixels.setPixelColor(i, r, g, b);
  }
}

void identifyUpdate(){
  if(identify){
    Serial.println("Identifying!");
    setColor(255,0,0);
    delay(300);
    setColor(0,255,0);
    delay(300);
    setColor(0,0,255);
    delay(300);
    identify = false;
  }
}
//Persistence Methods
void configurePersistence() {
  preferences.begin(namespaceName, true);
  //Read Config
  //WiFi
  mainAP = preferences.getBool("mainAP", 0);
  APSsid = preferences.getString("ownSsid", "ERROR");
  APPassword = preferences.getString("ownPassword", "ERROR");
  mainSsid = preferences.getString("mainSsid", "ERROR");
  mainPassword = preferences.getString("mainPassword", "ERROR");
  byte buffer[4];
  preferences.getBytes("ipAddress", buffer, 4);
  ipAddress = IPAddress(buffer[3], buffer[2], buffer[1], buffer[0]);

  //LED
  numLeds = preferences.getInt("amountOfLEDs", 42);

  dmxStartAdress = preferences.getInt("dmxStartAdress", 1);
  segments = preferences.getInt("segments", 1);
  maxDmxStartAdress = 512 + 1 - 3 * segments;

  preferences.end();
}

void setPerInt(char* nameOfspace, char* key, int data) {
  preferences.begin(nameOfspace, false);
  preferences.putInt(key, data);
  preferences.end();
}

//DMX Methods
void configureDMX() {
  pinMode(2, OUTPUT);
  digitalWrite(2, 1);
  dmx_config_t config = DMX_CONFIG_DEFAULT;
  dmx_personality_t personalities[] = {
    { 1, "Default Personality" }
  };
  int personality_count = 1;
  dmx_driver_install(dmxPort, &config, personalities, personality_count);
  dmx_set_pin(dmxPort, transmitPin, receivePin, enablePin);
}

void dmxUpdate() {
  receiveDMXData();
  processDMXData(); 
}

void receiveDMXData(){
  if (dmx_receive(dmxPort, &packet, DMX_TIMEOUT_TICK)) {
    if (!packet.err) {
      /* If this is the first DMX data we've received, lets log it! */
      if (!dmxIsConnected) {
        Serial.println("DMX is connected!");
        dmxIsConnected = true;
      }

      /* Don't forget we need to actually read the DMX data into our buffer so
        that we can print it out. */
      dmx_read(dmxPort, data, packet.size);
    } else {
      /* Oops! A DMX error occurred! Don't worry, this can happen when you first
        connect or disconnect your DMX devices. If you are consistently getting
        DMX errors, then something may have gone wrong with your code or
        something is seriously wrong with your DMX transmitter. */
      Serial.println("A DMX error occurred.");
      switch (packet.err) {
        case DMX_ERR_TIMEOUT:
          Serial.printf("The driver timed out waiting for the packet.\n");
          /* If the provided timeout was less than DMX_TIMEOUT_TICK, it may be
          worthwhile to call dmx_receive() again to see if the packet could be
          received. */
          break;

        case DMX_ERR_IMPROPER_SLOT:
          Serial.printf("Received malformed byte at slot %i.\n", packet.size);
          /* A slot in the packet is malformed. Data can be recovered up until 
          packet.size. */
          break;

        case DMX_ERR_UART_OVERFLOW:
          Serial.printf("The DMX port overflowed.\n");
          /* The ESP32 UART overflowed. This could occur if the DMX ISR is being
          constantly preempted. */
          break;
      }
    }
  }
}

void processDMXData(){
  int LedsPerSegment = numLeds/segments;
  for(int segment=0; segment<segments; segment++){
    int startIndex = LedsPerSegment * segment;
    int endIndex = startIndex + LedsPerSegment - 1;
    int dmxAdressR = segment * 3 + dmxStartAdress;
    int dmxAdressG = dmxAdressR + 1;
    int dmxAdressB = dmxAdressG + 1;
    setPixelsColor(startIndex, endIndex, data[dmxAdressR], data[dmxAdressG], data[dmxAdressB]);
  }
  pixels.show();
}