#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <ESP8266WiFi.h>

class WiFiManager
{
public:
    WiFiManager(const char *ssid, const char *pass);
    void initialize();
    void connect();
    bool isConnected();
    IPAddress getIP();

private:
    const char *ssid;
    const char *pass;
};

#endif