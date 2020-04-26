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
    const std::string _baseTopic = "Moisture/%d/";
    const std::string _min = "Min";
    const std::string _max = "Max";
          std::string _configTopic;

    String* _IDString;

    AsyncMqttClient* _mqttClient;
    Moisture*        _moisture;

    void ReceiveCommands(const char* topic, const char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    void OnMqttConnect(bool sessionPresent);

    void SendConfig(const char* valueName, int value);
    void SendDate(const char* valueName, double value);
    void SendDate(const char* valueName, int value);

    char* SetTopic(const char* topic);
};

static int _ID = 0;

MoistureMQTT::MoistureMQTT(Moisture* moisture, AsyncMqttClient* client){
    char _IDstring[20];
   
    _mqttClient = client;
    _moisture = moisture;
    
    sprintf(_IDstring, _baseTopic.c_str(), _ID++); // "Moisture/{_ID}/"
    Serial.println("Initialize Moisture MQTT Wrapper");
    Serial.println(_IDstring);
    _IDString = new String(_IDstring);

    _configTopic = _IDString->c_str();
    _configTopic += "SetConfig";
 
    using namespace std::placeholders;  // for _1, _2, _3...
  
    _mqttClient->subscribe(_configTopic.c_str(), 1);
    auto con = std::bind(&MoistureMQTT::OnMqttConnect, this, _1);
    _mqttClient->onConnect(con);

    auto rcv = std::bind(&MoistureMQTT::ReceiveCommands, this, _1, _2, _3, _4, _5, _6);
    _mqttClient->onMessage(rcv);
}

MoistureMQTT::~MoistureMQTT(){}

void MoistureMQTT::OnMqttConnect(bool sessionPresent)
{
    SendConfig((char*)_min.c_str(), _moisture->GetCalibration(Moisture::Calibrate::MinValue));
    SendConfig((char*)_max.c_str(), _moisture->GetCalibration(Moisture::Calibrate::MaxValue));
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

void MoistureMQTT::SendDate(const char* valueName, int value){
    char* msg = new char[128];

    String topic = SetTopic("Data/");
    topic.concat(valueName);
    Serial.println(topic);
    

    Serial.print("Send Payload: ");
    sprintf(msg, "%d", value);
    Serial.println(msg);

    _mqttClient->publish(topic.c_str(), 1, true, msg);
}

void MoistureMQTT::SendDate(const char* valueName, double value){
    char* msg = new char[128];

    char* topic = SetTopic("Data/");
    strncpy(topic, valueName, 128);
    Serial.println(topic);
    
    Serial.print("Send Payload: ");
    sprintf(msg, "%f", value);
    Serial.println(msg);

    _mqttClient->publish(topic, 1, true, msg);
}

char* MoistureMQTT::SetTopic(const char* topic){
    Serial.print("Set Topic: ");
    
    char str[128];
    strncpy(str, _IDString->c_str(), 128);
    strncat(str, topic, 128);
    
    Serial.println(str);
    return str;
}

void MoistureMQTT::SendConfig(const char* valueName, int value){
    char* msg = new char[128];

    String topic = SetTopic("GetConfig/");
    topic.concat(valueName);
    Serial.println(topic);
    
    Serial.print("Send Payload: ");
    sprintf(msg, "%d", value);
    Serial.println(msg);

    _mqttClient->publish(topic.c_str(), 1, true, msg);
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
