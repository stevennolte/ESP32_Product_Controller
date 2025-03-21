#include <Arduino.h>
#include <Arduino.h>
#include "ESPconfig.h"
#include "ESPWifi.h"
#include "myLED.h"
#include "Wire.h"
#include <ESPAsyncWebServer.h>
#include "ESP32OTAPull.h"
#include "ESPudp.h"

#include "driver/temp_sensor.h"


// TwoWire twoWire = TwoWire(0);
// TwoWire twoWire1 = TwoWire(0);

// Variables to store the state of the switches
bool foldOuterWingsState = true;
bool foldCenterWingsState = false;
bool raiseWingsState = false;


ESPconfig espConfig;
MyLED myLED(&espConfig);
ESPWifi espWifi(&espConfig);
ESPudp espUdp(&espConfig);

std::vector<String> debugVars;
AsyncWebServer server(80);

auto& progData = espConfig.progData;
auto& progCfg = espConfig.progCfg;
auto& progState = espConfig.progData.state;
auto& wifiCfg = espConfig.wifiCfg;

#pragma region Webserver

void handleFirmwareUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    Serial.printf("Update Start: %s\n", filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // Start with max available size
      Update.printError(Serial);
    }
  }
  
  // Write the received data to the flash memory
  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  }

  // If the upload is complete
  if (final) {
    if (Update.end(true)) { // True to set the size correctly
      Serial.printf("Update Success: %u bytes\n", index + len);
      request->send(200, "text/html", "Update complete! Rebooting...");
      delay(1000);
      ESP.restart();
    } else {
      Update.printError(Serial);
      request->send(500, "text/html", "Update failed.");
    }
  }
}


void updateDebugVars() {
  debugVars.clear(); // Clear the list to update it dynamically
  debugVars.push_back("Program: " + String(NAME));
  debugVars.push_back("Timestamp since boot [s]: " + String((float)(millis())/1000.0));
  debugVars.push_back("Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
  float tempReading;
  temp_sensor_read_celsius(&tempReading);
  debugVars.push_back("Temp: " + String(tempReading));
  debugVars.push_back("Version: " + String(VERSION));
  debugVars.push_back("Wifi SSID: " + WiFi.SSID());
  debugVars.push_back("IP Address: " + String(wifiCfg.ips[0])+"."+String(wifiCfg.ips[1])+"."+String(wifiCfg.ips[2])+"."+String(wifiCfg.ips[3]));
  debugVars.push_back("Wifi State: " + String(wifiCfg.state));
  debugVars.push_back("Program State: " + String(progData.state));
  


  
  

  String sipValue = String(wifiCfg.ips[0])+"."+String(wifiCfg.ips[1])+"."+String(wifiCfg.ips[2])+"."+String(wifiCfg.ips[3]);
  int   ArrayLength  =sipValue.length()+1;    //The +1 is for the 0x00h Terminator
  char  CharArray[ArrayLength];
  sipValue.toCharArray(CharArray,ArrayLength);
  // std::string ipValue = sipValue.toCharArray();
  
}

// Function to serve the debug variables as JSON
void handleDebugVars(AsyncWebServerRequest *request) {
  updateDebugVars();  // Update the debug variables just before sending
  DynamicJsonDocument doc(1024);
  JsonArray array = doc.to<JsonArray>();
  
  for (const auto& var : debugVars) {
    array.add(var);
  }
  
  String jsonResponse;
  serializeJson(doc, jsonResponse);
  request->send(200, "application/json", jsonResponse);
}


// Function to serve the file list as JSON
void handleFileList(AsyncWebServerRequest *request) {
  DynamicJsonDocument doc(1024);
  JsonArray array = doc.to<JsonArray>();

  // Open LittleFS root directory and list files
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  
  while (file) {
    JsonObject fileObject = array.createNestedObject();
    fileObject["name"] = String(file.name());
    fileObject["size"] = file.size();
    file = root.openNextFile();
  }

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  request->send(200, "application/json", jsonResponse);
  Serial.println("Sent File List");
}

// Reboot handler
void handleReboot(AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "Rebooting...");
  delay(100); // Give some time for the response to be sent
  ESP.restart();
}

// File download handler
void handleFileDownload(AsyncWebServerRequest *request) {
  if (request->hasParam("filename")) {
    String filename = request->getParam("filename")->value();
    if (LittleFS.exists("/" + filename)) {
      request->send(LittleFS, "/" + filename, "application/octet-stream");
    } else {
      request->send(404, "text/plain", "File not found");
    }
  } else {
    request->send(400, "text/plain", "Filename not provided");
  }
}

// File upload handler
void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    // Open file for writing
    Serial.printf("UploadStart: %s\n", filename.c_str());
    request->_tempFile = LittleFS.open("/" + filename, "w");
  }
  if (len) {
    // Write the file content
    request->_tempFile.write(data, len);
  }
  if (final) {
    // Close the file
    request->_tempFile.close();
    Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);
    request->send(200, "text/plain", "File Uploaded");
  }
}

void handleMomentaryCommand(AsyncWebServerRequest *request) {
  Serial.println("got Momentary Command");
  // if (request->hasParam("button") && request->hasParam("action")) {
  //   String button = request->getParam("button")->value();
  //   String action = request->getParam("action")->value();

  //   Serial.printf("Momentary Command: Button=%s, Action=%s\n", button.c_str(), action.c_str());

  //   // Add your logic here to handle the momentary button actions
  //   if (action == "start") {
  //     // Start the action for the button
  //     Serial.printf("Starting action for button: %s\n", button.c_str());
  //   } else if (action == "stop") {
  //     // Stop the action for the button
  //     Serial.printf("Stopping action for button: %s\n", button.c_str());
  //   } else {
  //     Serial.println("Unknown action received for momentary command.");
  //   }

  //   request->send(200, "text/plain", "Momentary command processed");
  // } else {
  //   request->send(400, "text/plain", "Invalid parameters");
  // }
}
void handleToggleAPMode(AsyncWebServerRequest *request) {
  static bool apModeState = false;
  apModeState = !apModeState;
  wifiCfg.apMode = apModeState ? 1 : 0;
  Serial.printf("AP Mode State: %s\n", apModeState ? "ON" : "OFF");
  request->send(200, "text/plain", apModeState ? "AP_Mode is ON" : "AP_Mode is OFF");
}

// Function to handle toggle switch commands
void handleToggleCommand(const String& command, const String& action) {
  if (command == "foldOuterWings") {
      foldOuterWingsState = (action == "on");
      Serial.printf("Fold Outer Wings: %s\n", foldOuterWingsState ? "ON" : "OFF");
      // Add your logic here to control hardware for "Fold Outer Wings"
  } else if (command == "foldCenterWings") {
      foldCenterWingsState = (action == "on");
      Serial.printf("Fold Center Wings: %s\n", foldCenterWingsState ? "ON" : "OFF");
      // Add your logic here to control hardware for "Fold Center Wings"
  } else if (command == "raiseWings") {
      raiseWingsState = (action == "on");
      Serial.printf("Raise Wings: %s\n", raiseWingsState ? "ON" : "OFF");
      // Add your logic here to control hardware for "Raise Wings"
  } else {
      Serial.println("Unknown command received.");
  }
}
#pragma endregion

void setup() {
  progData.state = 0;
  myLED.startTask();
  progData.state = 2;

  // Start USB Serial Port
  Serial.begin(115200);
  delay(5000);   // Wait for the usb to connect so you can see the outputs at startup
  Serial.println("Starting up...");
  espConfig.progData.confRes = espConfig.loadConfig();
  // Start Wifi AP and Webserver for diagnostics
  // espConfig.wifiCfg.state = espWifi.makeAP();
  wifiCfg.state = espWifi.connect();
  Serial.println("Wifi State: " + String(espConfig.wifiCfg.state));
  #pragma region Server Setup
        // Serve the main HTML page
        #pragma region Page Handlers
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
          Serial.println("getting Home file");
          request->send(LittleFS, "/product.html");
        });
        server.on("/product.html", HTTP_GET, [](AsyncWebServerRequest *request){
          Serial.println("getting Home file");
          request->send(LittleFS, "/product.html");
        });
        server.on("/settings.html", HTTP_GET, [](AsyncWebServerRequest *request){
          Serial.println("getting settings file");
          request->send(LittleFS, "/settings.html");
        });
        server.on("/debug.html", HTTP_GET, [](AsyncWebServerRequest *request){
          Serial.println("getting index file");
          request->send(LittleFS, "/debug.html");
        });
        server.on("/boom.html", HTTP_GET, [](AsyncWebServerRequest *request){
          Serial.println("getting boom file");
          request->send(LittleFS, "/boom.html");
        });
        #pragma endregion

        #pragma region Request Handlers
        server.on("/getDebugVars", HTTP_GET, handleDebugVars);
        server.on("/getFiles", HTTP_GET, handleFileList);
        server.on("/download", HTTP_GET, handleFileDownload);
        server.on("/reboot", HTTP_GET, handleReboot);
        
        #pragma endregion

        #pragma region Post Handlers
        server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {}, handleFileUpload);
        server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {}, handleFirmwareUpload);

        server.on("/toggleAPMode", HTTP_POST, handleToggleAPMode); // Add this line
        // Define the endpoint to handle toggle switch commands
    server.on("/foldOuterWings/on", HTTP_POST, [](AsyncWebServerRequest *request) {
      handleToggleCommand("foldOuterWings", "on");
      request->send(200, "text/plain", "Fold Outer Wings ON");
  });

  server.on("/foldOuterWings/off", HTTP_POST, [](AsyncWebServerRequest *request) {
      handleToggleCommand("foldOuterWings", "off");
      request->send(200, "text/plain", "Fold Outer Wings OFF");
  });

  server.on("/foldCenterWings/on", HTTP_POST, [](AsyncWebServerRequest *request) {
      handleToggleCommand("foldCenterWings", "on");
      request->send(200, "text/plain", "Fold Center Wings ON");
  });

  server.on("/foldCenterWings/off", HTTP_POST, [](AsyncWebServerRequest *request) {
      handleToggleCommand("foldCenterWings", "off");
      request->send(200, "text/plain", "Fold Center Wings OFF");
  });

  server.on("/raiseWings/on", HTTP_POST, [](AsyncWebServerRequest *request) {
      handleToggleCommand("raiseWings", "on");
      request->send(200, "text/plain", "Raise Wings ON");
  });

  server.on("/raiseWings/off", HTTP_POST, [](AsyncWebServerRequest *request) {
      handleToggleCommand("raiseWings", "off");
      request->send(200, "text/plain", "Raise Wings OFF");
  });
  server.on("/momentary/leftFlipIn", HTTP_POST, handleMomentaryCommand);
        #pragma endregion
        
        
        // Start server
        server.begin();
      #pragma endregion
      temp_sensor_start();
      
    
  
  
  progState = 1;
}

void debugPrint(){
  Serial.printf("Timestamp since boot [ms]: %lu", millis());
  Serial.printf(" progName: %s", espConfig.progCfg.name);
  Serial.printf(" progState: %lu", progState);
  Serial.printf(" confRes: %lu", espConfig.progData.confRes);
  Serial.printf(" wifiRes: %lu", espConfig.wifiCfg.state);
  float reading;
  temp_sensor_read_celsius(&reading);
  Serial.printf(" temp: %f", reading);
  Serial.println();
  // Serial.println(twoWire.requestFrom(0x22, 0x01));
  // Serial.printf("Mag x: %.2f mT, y: %.2f mT, z: %.2f mT, Temp: %.2f °C\n", espConfig.magData.x, espConfig.magData.y, espConfig.magData.z, (espConfig.magData.t*1.8)+32);
  // Serial.println();
  // Serial.println(espConfig.progCfg.name);
  // Serial.println();
}

void loop(){
  
  // please note that the value of status should be checked and properly handler
  
  delay(1000);
  debugPrint();
}
 

