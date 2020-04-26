#include <stdio.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h> 
// #include "sdkconfig.h"
#include <Arduino.h>
#include <WiFi.h>
#include <driver/adc.h>
//  - framework-arduinoespressif32 3.10004.200129 (1.0.4)
//  - framework-espidf 3.30300.190916 (3.3.0)
//  - tool-esptoolpy 1.20600.0 (2.6.0)
//  - tool-mkspiffs 2.230.0 (2.30)
//  - toolchain-xtensa32 2.50200.80 (5.2.0)

#include "MoistureMQTT.hpp"

Moisture*       moisture[2];
MoistureMQTT*   moistureMqtt[2];

int motor[2];

void CalibrateSensors(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total){
    char  msg[128];
    Serial.println(topic);
    Serial.println(payload);

    String pyl =  String(payload).substring(0, len);
    Serial.println(pyl);


    if(String(topic) == "Moisture/Calibrate/0"){
        if (String(pyl) == "Min")
            moisture[0]->SetCalibration(Moisture::Calibrate::MinValue);
        if (String(pyl) == "Max")
            moisture[0]->SetCalibration(Moisture::Calibrate::MaxValue);
        if (String(pyl) == "GetConfig"){
            sprintf(msg, "%d", moisture[0]->GetCalibration(Moisture::Calibrate::MinValue));
            mqttClient.publish("Moisture/0/Config/Min", 1, true, msg);
            sprintf(msg, "%d", moisture[0]->GetCalibration(Moisture::Calibrate::MaxValue));
            mqttClient.publish("Moisture/0/Config/Max", 1, true, msg);
        }
    }
    
    if(String(topic) == "Moisture/Calibrate/1"){
        if (String(pyl) == "Min")
            moisture[1]->SetCalibration(Moisture::Calibrate::MinValue);
        if (String(pyl) == "Max")
            moisture[1]->SetCalibration(Moisture::Calibrate::MaxValue);
        if (String(pyl) == "GetConfig"){
            sprintf(msg, "%d", moisture[1]->GetCalibration(Moisture::Calibrate::MinValue));
            mqttClient.publish("Moisture/1/Config/Min", 1, true, msg);
            sprintf(msg, "%d", moisture[1]->GetCalibration(Moisture::Calibrate::MaxValue));
            mqttClient.publish("Moisture/1/Config/Max", 1, true, msg);
        }
    }
}

void SubscribeMqtt(bool sessionPresent){
    mqttClient.subscribe("Moisture/Calibrate/0", 1);
    mqttClient.subscribe("Moisture/Calibrate/1", 1);
    mqttClient.onMessage(CalibrateSensors);
}

void setup() {
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    // WiFi.mode(WIFI_STA);
    Serial.begin(115200);
    delay(100);

    setupMQTT();

    Serial.println("Build Sensors");
    moisture[0] = new Moisture(ADC1_CHANNEL_6);
    moisture[1] = new Moisture(ADC1_CHANNEL_7);

   Serial.println("Setup Pumps");
    motor[0] = 26;
    motor[1] = 27;


    for (int i = 0; i < 2; i++)
        pinMode(motor[i], OUTPUT);

    mqttClient.onConnect(SubscribeMqtt);

    Serial.println("Build MQTT Wrapper");
    moistureMqtt[0] = new MoistureMQTT(moisture[0], &mqttClient);
    moistureMqtt[1] = new MoistureMQTT(moisture[1], &mqttClient);
}

void loop() {
    // char  msg[128];
    // char  mqtt[128];
    
    for (int i = 0; i < 2; i++)
    {
         moistureMqtt[i]->SendData();
         moistureMqtt[i]->SendMoisture(); 
         moistureMqtt[i]->SendVoltage(); 

        // if (moist <= 0.4){
        //     sprintf(msg, "On");
        //     sprintf(pub, "%s/Motor", mqtt);
        //     mqttClient.publish(pub, 1, true, msg);
        //     digitalWrite(motor[i], HIGH);
        // }
        // else{
        //     sprintf(msg, "Off");
        //     sprintf(pub, "%s/Motor", mqtt);
        //     mqttClient.publish(pub, 1, true, msg);
        //     digitalWrite(motor[i], LOW);
        // }
    }
    delay(3000);
}



