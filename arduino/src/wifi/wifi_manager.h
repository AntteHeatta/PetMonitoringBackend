#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFiEsp.h>
#include <SoftwareSerial.h>

class WiFiManager
{
public:
    WiFiManager(int rxPin, int txPin, const char *ssid, const char *pass);
    void initialize();
    void connect();
    bool isConnected();
    IPAddress getIP();

private:
    SoftwareSerial espSerial;
    const char *ssid;
    const char *pass;
};

#endif