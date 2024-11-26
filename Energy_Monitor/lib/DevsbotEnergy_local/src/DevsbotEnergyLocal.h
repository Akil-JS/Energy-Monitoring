//gateway concept 
//change authtoken----> gateWayId
//remove devsbotClusterID(global variable),clusterID(function parameter)
#ifndef DEVSBOTENERGYLOCAL_H
#define DEVSBOTENERGYLOCAL_H

#include <Arduino.h>
#include<Preferences.h>

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "FS.h"

#elif defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#include<Update.h>
#include <HTTPUpdate.h>
#include "SPIFFS.h"
#endif

#include <WiFiClientSecure.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>
#include <ArduinoJson.h>


class Devsbot
{
  public:
          const float defaultFirmwareVersion=1.0; // not a valid firmware version
          const bool defaultCheckVal=0;
          float deviceFirmwareVersion;
          float devsbotFirmwareVersion;

          String devicePassword= "";
          String deviceSsid= "";


          const char* userSsid= NULL;
          const char* userPassword= NULL;

          byte apiHitCnt=3;
          
          void begin();
          void begin(const char* authToken); //connecting to default ssid and password
          void begin(const char* authToken, const char* ssid, const char* password);
          void wifiBegin(const char* ssid, const char* password);
          void Loop();
          void sendVirtualWrite(byte dbVirtualPin, int dbVirtualData);
          void sendVirtualWrite(byte dbVirtualPin, String dbVirtualData);
          void devsbotDelay(uint64_t dbMilliSeconds);
          void sentEnergyMeterData(String EnergyString);
          void editAndStoreJson();
          bool instantaneousReading; //live_data
          int completeJsonReading=60;//data_interval
          uint8_t numSlave; //number of slave by default is 1
          byte slaveIdArray[32];
          void deviceContinue();
          void wifiAfterProvision();
          void RWToEEPROM(const char* authToken);
          void ReadOnlyEEPROM();
          bool checkInternetConnectivity();
          bool checkServerConnection(IPAddress ip, uint16_t port);
          bool checkServerConnection(uint16_t port);
          void reconnectingToWifi();
          void WiFiEvent(WiFiEvent_t event);
          void EEPROMfirmwareVersion();
          bool wifiStatus=0;
          bool poorWifi=0;
          bool sioDisconnect=1;//by default socket io is disconnected
          bool sioDisconnectFlag=1;
          uint64_t sioDisconnectStart=0;

          

  private:
          HTTPClient http;
          HTTPClient httpota;
          WiFiClientSecure OTAclient; //not used for a OTA 
          WiFiClientSecure client;
          static inline byte sIOConnectionStatus;

        
        //  uint64_t sioDisconnectEnd=0;

          
          String devsbotPassword= "";
          String devsbotSsid= "";
          
          const char* devsbotDefaultSsid= "Niraltek";
          const char* devsbotDefaultPassword= "Niraltek@12345";
          

          String devsbotClusterID;
          String devsbotAuthToken;
          
          //uint16_t apiResponseCode; 
          int apiResponseCode;
          String apiResponse;
          String provisionData;
          String widgetData;

          int heartBeatInterval;
          bool aliveState=true;
          int devsbotheartBeatInterval;
          bool devsbotliveData;
          int devsbotDataInterval;
          bool connected =false;
          bool gotIp=false;
          IPAddress ip;
          IPAddress serverIP;
          int serverIpResult=0;
          bool dnstoip=0;
          bool devsbotauthenticationcheck;
          const int wifiWating=15000;
          
          //float deviceFirmwareVersion=1.0;
          

          int deviceWidgetVersion;
          int devsbotWidgetVersion;

          unsigned long dbMessageTimestamp = 0;
          unsigned long devsbotStatus_Time = 0;
         
          

          String digitalInputPin;
          String digitalInputPullupPin;
          String analogInputPin;

          // const char* hostname = "192.168.0.118";
          // String AuthTokenApiURL= "http://192.168.0.118:3002/device/first/provision";  //post in while loop
          // String deviceProvisionURL= "http://192.168.0.118:3002/device/provision/data"; //get in while loop
          // String devsbotWidgetURL= "http://192.168.0.118:3002/cluster/widget/pin"; //not in while 
          // String devsbotOTAUpdateURL = "http://192.168.0.118:3002/OTAfile/download";
          // String firmwareVersionURL = "http://192.168.0.118:3002/device/update/version";// post in while
          // String devsbotDeviceStatusURL = "http://192.168.0.118:3002/device/status"; //post in while
          // String devsbotDeviceLogURL = "http://192.168.0.118:3002/iot/device/post"; //post in while
          // String energymeterJsonURL="http://192.168.0.118:3002/meterjsonData";//not in while

          //  const char* hostname = "sem-demo.devsbot.com";
          //  String AuthTokenApiURL= "https://sem-demo.devsbot.com/req/device/first/provision";
          //  String deviceProvisionURL= "https://sem-demo.devsbot.com/req/device/provision/data";
          //  String devsbotWidgetURL= "https://sem-demo.devsbot.com/req/cluster/widget/pin";
          //  String devsbotOTAUpdateURL = "https://sem-demo.devsbot.com/req/OTAfile/download";
          //  String firmwareVersionURL = "https://sem-demo.devsbot.com/req/device/update/version";
          //  String devsbotDeviceStatusURL = "https://sem-demo.devsbot.com/req/device/status"; 
          //  String devsbotDeviceLogURL = "https://sem-demo.devsbot.com/req/iot/device/post";
          //  String energymeterJsonURL="https://sem-demo.devsbot.com/req/meterjsonData";

           const char* hostname = "sem-demo.devsbot.com";
           String AuthTokenApiURL= "https://sem-demo.devsbot.com/api/device/first/provision";
           String deviceProvisionURL= "https://sem-demo.devsbot.com/api/device/provision/data";
           String devsbotWidgetURL= "https://sem-demo.devsbot.com/api/cluster/widget/pin";
           String devsbotOTAUpdateURL = "https://sem-demo.devsbot.com/api/OTAfile/download";
           String firmwareVersionURL = "https://sem-demo.devsbot.com/api/device/update/version";
           String devsbotDeviceStatusURL = "https://sem-demo.devsbot.com/api/device/status"; 
           String devsbotDeviceLogURL = "https://sem-demo.devsbot.com/api/iot/device/post";
           String energymeterJsonURL="https://sem-demo.devsbot.com/api/meterjsonData";
        
          // const char* hostname = "energy.devsbot.com";
          //  String AuthTokenApiURL= "https://energy.devsbot.com/api/device/first/provision";
          //  String deviceProvisionURL= "https://energy.devsbot.com/api/device/provision/data";
          //  String devsbotWidgetURL= "https://energy.devsbot.com/api/cluster/widget/pin";
          //  String devsbotOTAUpdateURL = "https://energy.devsbot.com/api/OTAfile/download";
          //  String firmwareVersionURL = "https://energy.devsbot.com/api/device/update/version";
          //  String devsbotDeviceStatusURL = "https://energy.devsbot.com/api/device/status"; 
          //  String devsbotDeviceLogURL = "https://energy.devsbot.com/api/iot/device/post";
          // String energymeterJsonURL="https://energy.devsbot.com/api/meterjsonData";
      
          
          void wifiConnectionChecking();
          void AuthToken(const char* authToken);
          void AuthTokenAPI();
          void deviceProvision();
          void deviceProvisionData();
          void devicePreProvisionData();
          void firmwareVersionVerification();     
          void firmwareUpdate();
          void widgetBegin();
          void widgetAPI();
          void widgetPinInitialize();
          static void updateStarted();
          static void updateProgress(int cur, int total);
          static void updateFinished();
          static void updateError(int err);
          void firmwareVersionSend();
          void deviceFirmwareUpdate();
          void socketIOConnection();
          static void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length);
          void devsbotAuthentication();
          void devsbotStatus();
          void widgetUpdate();
          void widgetVersionEdit();
          void sensorInput();
          void jsonInputSend(String dbInputMethod, String dbInputPin, String dbDeviceValue);
          void jsonInputSend(String MeterJsonData,String Energyjson);
          void deviceLog(String dbDeviceLogData);
          void deviceLogOTA(String dbDeviceLogOTAData);
};

extern Devsbot dBot;
#endif
