#include <string>
#include "MQTT.hpp"
#include "Moisture.hpp"

class MoistureMQTT{

public:
    MoistureMQTT();
    MoistureMQTT(Moisture* moisture, AsyncMqttClient* mqttClient);
    ~MoistureMQTT();

    void SendMoisture();
    void SendData();
    void SendVoltage();

private:
    const char* _baseTopic  = "Moisture/%d/";
    const char* _min        = "Min";
    const char* _max        = "Max";

    char _IdString[128];

    AsyncMqttClient* _mqttClient;
    Moisture*        _moisture;

    void ReceiveCommands(const char* topic, const char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    void OnMqttConnect(bool sessionPresent);

    void SendConfig(const char* valueName, int value);
    void SendDate(const char* valueName, double value);
    void SendDate(const char* valueName, int value);

    void SetTopic(char *str, const char* topic);
    void Publish(const char* topic, const char* msg);
};

static int _ID = 0;

MoistureMQTT::MoistureMQTT(Moisture* moisture, AsyncMqttClient* client){
    char _configTopic[128];

    _mqttClient = client;
    _moisture = moisture;
    
    Serial.println("Initialize Moisture MQTT Wrapper");

    sprintf(_IdString, _baseTopic, _ID++); // "Moisture/{_ID}/"
    strncpy(_configTopic, _IdString, 128);
    strncat(_configTopic, "SetConfig", 128);
 
    using namespace std::placeholders;  // for _1, _2, _3...
    _mqttClient->subscribe(_configTopic, 1);
    auto con = std::bind(&MoistureMQTT::OnMqttConnect, this, _1);
    _mqttClient->onConnect(con);

    auto rcv = std::bind(&MoistureMQTT::ReceiveCommands, this, _1, _2, _3, _4, _5, _6);
    _mqttClient->onMessage(rcv);
}

MoistureMQTT::~MoistureMQTT(){}

void MoistureMQTT::OnMqttConnect(bool sessionPresent)
{
    SendConfig(_min, _moisture->GetCalibration(Moisture::Calibrate::MinValue));
    SendConfig(_max, _moisture->GetCalibration(Moisture::Calibrate::MaxValue));
}

void MoistureMQTT::SendMoisture(){
    SendDate((char*)"Moisture" , _moisture->GetMoisture());
}

void MoistureMQTT::SendData(){
    SendDate((char*)"RawData", _moisture->GetData());
}

void MoistureMQTT::SendVoltage(){
    SendDate((char*)"Voltage", _moisture->GetVoltage());
}

void MoistureMQTT::SetTopic(char *str, const char* topic){
    strncpy(str, _IdString, 128);
    strncat(str, topic, 128);
}

void MoistureMQTT::Publish(const char* topic, const char* msg){
    Serial.print(topic);
    Serial.print(" = ");
    Serial.println(msg);

    _mqttClient->publish(topic, 1, true, msg);
}

void MoistureMQTT::SendDate(const char* valueName, int value){
    char msg[128];
    char topic[128];

    SetTopic(topic, "Data/");
    strncat(topic, valueName, 128);
    
    sprintf(msg, "%d", value);
        
    Publish(topic, msg);
}

void MoistureMQTT::SendDate(const char* valueName, double value){
    char msg[128];
    char topic[128];

    SetTopic(topic, "Data/");
    strncat(topic, valueName, 128);

    sprintf(msg, "%f", value);
        
    Publish(topic, msg);
}

void MoistureMQTT::SendConfig(const char* valueName, int value){
    char msg[128];
    char topic[128];

    SetTopic(topic, "GetConfig/");
    strncat(topic, valueName, 128);
    
    sprintf(msg, "%d", value);

    Publish(topic, msg);
}   

void MoistureMQTT::ReceiveCommands(const char* topic, const char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total){
    String pyl =  String(payload).substring(0, len);

    if(String(topic) == "Moisture/Calibrate/0"){
        if (String(pyl) == "Min")
            _moisture->SetCalibration(Moisture::Calibrate::MinValue);
        if (String(pyl) == "Max")
            _moisture->SetCalibration(Moisture::Calibrate::MaxValue);
    };
}
