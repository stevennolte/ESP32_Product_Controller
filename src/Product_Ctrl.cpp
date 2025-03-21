#include "Product_Ctrl.h"


Product_Ctrl::Product_Ctrl(ESPconfig* vars, CANBUS* canbus) : meter() {
    espConfig = vars;
}

void Product_Ctrl::begin(){
    meter.begin(espConfig->gpioDefs.FLOW_PIN, espConfig->flowCfg.flowCalNumber);  
    meter.setTresholds(espConfig->flowCfg.maxFlow, espConfig->flowCfg.maxFlow);
    xTaskCreate(
        taskHandler,   // Task function
        "TaskC",       // Name of the task
        4096,          // Stack size (in words)
        this,          // Pass the current instance as the task parameter
        1,             // Priority of the task
        NULL           // Task handle (not needed)
    );
}

void Product_Ctrl::taskHandler(void *param){
    Product_Ctrl* instance = (Product_Ctrl*)param;
    instance->continuousLoop();
}

void Product_Ctrl::continuousLoop(){
    while (true){
        switch(espConfig->progData.state){
            case 0:
                break;
            case 1:
                break;
            case 2:
                break;
        }
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
}

