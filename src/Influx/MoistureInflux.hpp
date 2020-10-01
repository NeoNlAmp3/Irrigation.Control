#pragma once
#define INFLUXDB_CLIENT_DEBUG_ENABLE
#include <WiFiMulti.h>
#include <InfluxDbCloud.h>
#include <InfluxDbClient.h>
#include <Logon/Logon.hpp>
#include <Logon/InfluxLogon.hpp>
#include <Moisture.hpp>

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
WiFiMulti wifiMulti;

class MoistureInfluxConnector {

    Point _dataPoint = Point("Moisture");;
    Moisture* _sensor;

    char _IdString[128];
    const char* _baseTopic  = "%d";

public:
    MoistureInfluxConnector(Moisture* sensor);
    ~MoistureInfluxConnector();

    void SendDataPoint();
};

static int _influxID = 0;
MoistureInfluxConnector::MoistureInfluxConnector(Moisture* sensor) {
    sprintf(_IdString, _baseTopic, _influxID++); // "Moisture/{_ID}/"
    _dataPoint.addTag("ID",_IdString);
    _sensor = sensor;

    if (_influxID != 1)
        return;
    WiFi.mode(WIFI_STA);

    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
    // Check server connection
    Serial.println("Validate Connection ");
    if (client.validateConnection()) {
        Serial.print("Connected to InfluxDB: ");
        Serial.println(client.getServerUrl());
    } else {
        Serial.print("InfluxDB connection failed: ");
        Serial.println(client.getLastErrorMessage());
    }
};

MoistureInfluxConnector::~MoistureInfluxConnector() {

};

void MoistureInfluxConnector::SendDataPoint() {
    _dataPoint.clearFields();
    _dataPoint.addField("Voltage",  _sensor->GetVoltage());
    _dataPoint.addField("Value",    _sensor->GetData());
    _dataPoint.addField("Moisture", _sensor->GetData());

    // Print what are we exactly writing
    Serial.print("Writing: ");
    Serial.println(_dataPoint.toLineProtocol());

    // Write point
    if (!client.writePoint(_dataPoint)) {
        Serial.print("InfluxDB write failed: ");
        Serial.println(client.getLastErrorMessage());
    }

}