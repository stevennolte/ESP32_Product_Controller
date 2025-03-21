#ifndef ESPCONF_H
#define ESPCONF_H
#include "Arduino.h"
#include "LittleFS.h"
#include "ArduinoJson.h"
#include "Version.h"

class ESPconfig
{
public:
    uint8_t loadConfig();
    uint8_t updateIP();
    uint8_t updateServer();
    uint8_t getStrapping();

    class GPIO_Definitions{
        public:
            uint8_t LED_PIN = 48;
            uint8_t SDA_PIN = 41;
            uint8_t SCL_PIN = 42;
            uint8_t SDA_H_PIN = 5;
            uint8_t SCL_H_PIN = 6;
            uint8_t CAN_RX = 1;
            uint8_t CAN_TX = 2;
            uint8_t FLOW_PIN = 34;
            
            GPIO_Definitions(){}
    };
    GPIO_Definitions gpioDefs;

    class I2C_Definitions{
        public:
            uint8_t ADS_ADDRESS = 0x48;
            I2C_Definitions(){}
    };
    I2C_Definitions i2cDefs;

    class ProgramConfig {
        public:
            char name[64];
            String name2;
            uint8_t version[3];
            uint8_t ledBrht;
            
            ProgramConfig(){}
    };
    ProgramConfig progCfg;
    
    class ProgramData {
        public:
            uint8_t state;
            uint8_t adsState;
            uint8_t confRes;
            ProgramData(){}
    };
    ProgramData progData;

    class WifiConfig {
        public:
            const char* ssids[4] = {"FERT", "SSEI"};
            const char* passwords[4] = {"Fert504!", "Nd14il!la"};
            uint8_t ips[4];
            uint8_t state;
            uint8_t apMode;
            WifiConfig(){}
    };
    WifiConfig wifiCfg;


    class OTAConfig {
        public:
            uint8_t state;
            uint8_t port;
            uint8_t ipAddr;
            char basePath[64];
            OTAConfig(){}
    };
    OTAConfig otaCfg;

    class FlowMeterConfig {
        public:
            uint8_t state;
            uint16_t flowCalNumber = 200;
            uint16_t minFlow = 1;
            uint16_t maxFlow = 100;
            FlowMeterConfig(){}
    };
    FlowMeterConfig flowCfg;

    ESPconfig(/* args */);
    
    
};




#endif
