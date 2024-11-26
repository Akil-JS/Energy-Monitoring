#include "esp32-hal-gpio.h"
#include "DevsbotEnergyLocal.h"

#define DEBUG_devsbot  // Devsbot debug console

#ifdef DEBUG_devsbot
#define debugLogln(x) Serial.println(x)
#define debugLog(x) Serial.print(x)
#else
#define debugLog(...)
#define debugLogln(...)
#endif

DynamicJsonDocument  doc(1024);
SocketIOclient socketIO;
Preferences preferences;


// Devsbot::Devsbot()
// {
//   Serial.printf("constructor is called\n");
//   //WiFi.onEvent(std::bind(&Devsbot::WiFiEvent, this, std::placeholders::_1));
// }

void Devsbot::WiFiEvent(WiFiEvent_t event) 
{
  Serial.printf("[WiFi-event] event: %d\n", event);

  switch (event) 
    {

        //case SYSTEM_EVENT_STA_CONNECTED:
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("WiFi Connected ");
            break;

        //case SYSTEM_EVENT_STA_DISCONNECTED:
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("WiFi disconnected");
            dBot.wifiStatus = 0;
            break;
        //case SYSTEM_EVENT_STA_LOST_IP:
        case ARDUINO_EVENT_WIFI_STA_LOST_IP:
            Serial.println("Lost IP address");
            dBot.wifiStatus = 0;
            break;
       // case SYSTEM_EVENT_STA_GOT_IP:
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.println("Got IP address");
            Serial.print("device connected : ");Serial.println(WiFi.SSID());
            dBot.wifiStatus = 1;
            break;
        // case SYSTEM_EVENT_STA_BSS_RSSI_LOW:devsbot
        //     Serial.println("device RSSI is below thershold");
        //     Serial.print("RSSI value : ");Serial.println(WiFi.RSSI());
        //     dBot.poorWifi = 1;
        //     break;
        default:
            break;
    }
}



/*void Devsbot:: begin(const char* clusterID, const char* authToken)
{
  WiFi.onEvent(std::bind(&Devsbot::WiFiEvent, this, std::placeholders::_1));
  debugLogln("Hello i am devsbot...!\n");
  debugLogln("passing a clusterID and authtoken \n");

  // Initialize the SPIFFS file system
  while(!SPIFFS.begin(true)){}
  debugLogln("SPIFFS was Successfully initialized\n");
  RWToEEPROM(authToken);
  devicePreProvisionData();
  wifiBegin(NULL, NULL);
  //deviceLog("Device connected to:  "  +  WiFi.SSID() + "\n");
  AuthToken(clusterID, authToken);
}

// functiom used for OTA update ,all device in the cluster got updated
void Devsbot:: begin(const char* clusterID, const char* ssid, const char* password) //running
{
  WiFi.onEvent(std::bind(&Devsbot::WiFiEvent, this, std::placeholders::_1));
  debugLogln("Hello i am devsbot...!\n");
  debugLogln("passing clusterID,ssid,password\n");

  // Initialize the SPIFFS file system
  while(!SPIFFS.begin(true)){}
  debugLogln("SPIFFS was Successfully initialized\n");

  ReadOnlyEEPROM();
  devicePreProvisionData();
  userSsid=ssid;
  userPassword=password;
  wifiBegin(ssid, password);
 // deviceLog("Device connected to:  "  +  WiFi.SSID() + "\n");
  AuthToken(clusterID, devsbotAuthToken.c_str()); //devsbotAuthToken readed form EEPROM
}*/


void Devsbot:: begin()
{
  WiFi.onEvent(std::bind(&Devsbot::WiFiEvent, this, std::placeholders::_1));
  debugLogln("Hello i am devsbot...!\n");
  debugLogln("passing only a macid\n");

  // Initialize the SPIFFS file system
  while(!SPIFFS.begin(true)){}
  debugLogln("SPIFFS was Successfully initialized\n");

  devicePreProvisionData();
  //WiFi.onEvent(WiFiEvent);
  wifiBegin(NULL, NULL); //first it will connect to a devsbotdefaultssid ,password
  //deviceLog("Device connected to:  "  +  WiFi.SSID() + "\n");
  AuthToken(NULL);// authotoken api function device got authtoken EEPROM can't open again
}


void Devsbot:: begin(const char* authToken, const char* ssid, const char* password)
{
  WiFi.onEvent(std::bind(&Devsbot::WiFiEvent, this, std::placeholders::_1));
  debugLogln("Hello i am devsbot...!\n");
  debugLogln("passing gatewayid,ssid,password\n");

  // Initialize the SPIFFS file system
  while(!SPIFFS.begin(true)){}
  debugLogln("SPIFFS was Successfully initialized\n");
  
  RWToEEPROM(authToken);
  debugLog("authtoken : ");debugLogln(devsbotAuthToken);
  devicePreProvisionData();
  userSsid=ssid;
  userPassword=password;
  wifiBegin(ssid, password); //connect device to wifi..
  //deviceLog("Device connected to:  "  +  WiFi.SSID() + "\n");
  AuthToken(authToken);
}

void Devsbot:: RWToEEPROM(const char* authToken) //perform both read and write 
{
  bool checkval;
  preferences.begin("devsbot", false);
  if(!preferences.getBool("check",0)) // writing a authtoke to EEPROM once ,if check is not there in namespcae it will return a 0 ,while opening a eeprom for a first time it will return a 0
  {
    debugLogln("writing a authtoken to a EEPROM...\n"); //put devcie log here
    preferences.putBool("check",1); // check variable make a check key value to 1 again if we open a  namespce devsbot it value set to 1
    preferences.putString("authtoken",authToken);
  }
  checkval=preferences.getBool("check",defaultCheckVal);
  devsbotAuthToken=preferences.getString("authtoken","");
  debugLog("authtoken read from RWEEPROM :  ");debugLogln(devsbotAuthToken);
  debugLog("checkval :  ");debugLogln(checkval);
  preferences.end();
}

void Devsbot:: ReadOnlyEEPROM()
{
  preferences.begin("devsbot", true);
  devsbotAuthToken=preferences.getString("authtoken","");
  debugLog("authtoken read from Read only EEPROM :  ");debugLogln(devsbotAuthToken);
  preferences.end();
}

void Devsbot:: wifiBegin(const char* ssid, const char* password)
{
  
  Serial.print("userssid: ");Serial.println(ssid);Serial.print("userpassword: ");Serial.println(password);
  if(deviceSsid=="" && devicePassword=="")
  {
    while(wifiStatus!=1)
    {
      debugLog("trying for userwifi ");debugLogln(deviceSsid);
      if(ssid != NULL)
      {
        debugLog("Connecting to user Wi-Fi...\n");
        WiFi.begin(ssid, password);
        wifiConnectionChecking();
      }
      if(wifiStatus!=1 && deviceSsid != "") //got from preprovision
      {
        debugLog("Connecting to devsbot webzone Wi-Fi...\n");
        WiFi.begin(deviceSsid.c_str(), devicePassword.c_str());
        wifiConnectionChecking();
      }

      if(wifiStatus!=1  && devsbotDefaultSsid != NULL)
      {
        debugLog("Connecting to devsbot default Wi-Fi...\n");
        WiFi.begin(devsbotDefaultSsid, devsbotDefaultPassword);
        wifiConnectionChecking();
      }
    }
  }
  else
  {
    debugLog("trying for webzone wifi ");debugLogln(deviceSsid);
    while(wifiStatus!=1)
    {
      if(wifiStatus!=1 && deviceSsid != "") //got from preprovision
      {
        debugLog("Connecting to devsbot webzone Wi-Fi...\n");
        WiFi.begin(deviceSsid.c_str(), devicePassword.c_str());
        wifiConnectionChecking();
      }

      if(wifiStatus!=1 && ssid != NULL)
      {
        debugLog("Connecting to user Wi-Fi...\n");
        WiFi.begin(ssid, password);
        wifiConnectionChecking();
      }
      if(wifiStatus!=1  && devsbotDefaultSsid != NULL)
      {
        debugLog("Connecting to devsbot default Wi-Fi...\n");
        WiFi.begin(devsbotDefaultSsid, devsbotDefaultPassword);
        wifiConnectionChecking();
      }
    }
  }
}



void Devsbot ::wifiConnectionChecking()
{
  Serial.printf("wifi connection cheking begin..\n");
  uint64_t wifiStartMillis = millis();
  uint64_t wifiEndMillis = wifiStartMillis + wifiWating;
  if(wifiStatus != 1)
  {
    while(wifiStartMillis <= wifiEndMillis)
    {
      wifiStartMillis = millis();
      if (wifiStatus == 1)
      {
        debugLog("Device connected to: ");debugLogln(WiFi.SSID());
        delay(100);
        //vTaskDelay(200/portTICK_PERIOD_MS);
        debugLog("device local IP : ");debugLogln(WiFi.localIP());
        deviceLog("Device connected to:  " + String(WiFi.SSID()));
        break;
      }
    }
  }
  Serial.printf("wifi connection End..\n");
}


/*void Devsbot:: wifiConnectionChecking()
{
  uint64_t wifiStartMillis = millis();
  uint64_t wifiEndMillis = wifiStartMillis + 3000;

  if(WiFi.status() != WL_CONNECTED)
  {
    while(wifiStartMillis <= wifiEndMillis)
    {
      wifiStartMillis = millis();
      if (WiFi.status() == WL_CONNECTED)
      {
        debugLog("Device connected to: ");debugLogln(WiFi.SSID());
        ip = WiFi.localIP();
        while(ip==IPAddress(0,0,0,0))
        {
          ip = WiFi.localIP();
          //Serial.printf("trying get local ip\n");
        }
        debugLog("device local IP : ");debugLogln(ip);
        break;
      }
    }
  }

  if (WiFi.status() == WL_CONNECTED) 
  {
    DNStoIP();
    debugLog(" => Wi-Fi Connected");debugLogln(WiFi.SSID());
    deviceLog("Wi-Fi Connected");deviceLog(WiFi.SSID());
  }
  
}*/

void Devsbot::EEPROMfirmwareVersion()
{
  preferences.begin("version", false); //prefernece version .
  if(!preferences.getBool("tempVal",0))
  {
    Serial.printf("put a firmVersion to EEPROM\n");
    preferences.putBool("tempVal",1);
    preferences.putFloat("firmVersion",1.0);
  }
  deviceFirmwareVersion=preferences.getFloat("firmVersion",defaultFirmwareVersion); // it will return a defaultFirmwareVersion version if there is no key of firmwareVersion
  Serial.printf("read a firmware Version from a EEPROM");
  debugLog("device Frimware version : ");debugLogln(deviceFirmwareVersion);
  preferences.end();
}


void Devsbot:: AuthToken(const char* authToken) // authtoken is sent form a macro hardcoded
{
  EEPROMfirmwareVersion();
  // devsbotClusterID= String(clusterID);
  // Serial.println("Cluster ID : " + devsbotClusterID + "\n");
  if(authToken == NULL || SPIFFS.exists("/Devsbot_Document.json"))
  {
    if(SPIFFS.exists("/Devsbot_Document.json"))
    {
      debugLogln("Device file available...!\n");
      deviceProvision();
      deviceProvisionData();
      wifiAfterProvision();
      widgetBegin();
      socketIOConnection();
      
    }
    else // if authtoken is null device in developer mode .we need to pass the macid .from application we get a authtoken 
    {
      debugLogln("Device in developer mode...!\n");      
      debugLogln("Device provisioning initiates...!\n");
      AuthTokenAPI(); // get authtoken if cluster id alone given
      deviceProvision();
      deviceProvisionData();
      ESP.restart();
    }
  }

  else // authtoken is given in device ,first time device is provisioning
  {
    debugLogln("Authtoken exist...!");
    deviceProvision();
    deviceProvisionData();
    ESP.restart();
  }
}

void Devsbot::reconnectingToWifi()
{
  debugLogln("reconnecting to Wifi...");
  WiFi.disconnect(true);//delete a old wifi config
  delay(100);
  debugLogln("device disconnected form a network");
  if(deviceSsid=="" && devicePassword=="")
  {
    debugLog("trying for userwifi ");debugLogln(deviceSsid);
    if(userSsid!= NULL)
    {
      debugLog("Connecting to user Wi-Fi...\n");
      WiFi.begin(userSsid, userPassword);
      wifiConnectionChecking();
    }
    if(wifiStatus!=1 && deviceSsid != "") //got from preprovision
    {
      debugLog("Connecting to devsbot webzone Wi-Fi...\n");
      WiFi.begin(deviceSsid.c_str(), devicePassword.c_str());
      wifiConnectionChecking();
    }

    if(wifiStatus!=1  && devsbotDefaultSsid != NULL)
    {
      debugLog("Connecting to devsbot default Wi-Fi...\n");
      WiFi.begin(devsbotDefaultSsid, devsbotDefaultPassword);
      wifiConnectionChecking();
    }

  }
  else
  {
    debugLog("trying for webzone wifi ");debugLogln(deviceSsid);
    if(wifiStatus!=1 && deviceSsid != "") //got from preprovision
    {
      debugLog("Connecting to devsbot webzone Wi-Fi...\n");
      WiFi.begin(deviceSsid.c_str(), devicePassword.c_str());
      wifiConnectionChecking();
    }

    if(wifiStatus!=1 && userSsid != NULL)
    {
      debugLog("Connecting to user Wi-Fi...\n");
      WiFi.begin(userSsid,userPassword);
      wifiConnectionChecking();
    }
    if(wifiStatus!=1  && devsbotDefaultSsid != NULL)
    {
      debugLog("Connecting to devsbot default Wi-Fi...\n");
      WiFi.begin(devsbotDefaultSsid, devsbotDefaultPassword);
      wifiConnectionChecking();
    }
  }
  if(sioDisconnectFlag==0)
  {
    sioDisconnectStart=millis();
    Serial.printf("sioDisconnectStart millis updated");
  }
}


void Devsbot::wifiAfterProvision()
{ 
  debugLogln("wifiAfterProvision begin");
  if(deviceSsid!=devsbotSsid || devicePassword!=devsbotPassword)
  {
    debugLogln("devicessid and devsbotssid is not same");
    deviceLog("devicessid and devsbotssid is not same");
    WiFi.disconnect(); //disconnected from previously connected network/
    //while(wifiStatus!=0){}
    //vTaskDelay(500/portTICK_PERIOD_MS);
    delay(100);
    while(wifiStatus==1)
    {
      debugLogln("disconnecting form a previous connected network");
    }
    debugLogln("device disconnected from previously connected network");
    deviceSsid=devsbotSsid; //got a value form provision webzone. 
    devicePassword=devsbotPassword;
    if(userSsid!=NULL && userPassword!=NULL)
    {
      wifiBegin(userSsid,userPassword);
    }
    else
    {
      wifiBegin(NULL,NULL);
    }
  }
  else
  {
    debugLogln("devicessid and devsbotssid is same continue with connected network");
  }
  debugLogln("wifiAfterProvision End");
}



void Devsbot:: AuthTokenAPI()
{
  byte authtokenCnt=0;
  debugLogln("AuthTokenAPI begin");
  //String authTokenRequest= "cluster_id=" + devsbotClusterID + "&device_mac_id=" + WiFi.macAddress();
  String authTokenRequest= "mac_id=" + WiFi.macAddress();
  debugLogln("Auth token request data: "+ authTokenRequest+ "\n");
  Serial.println(AuthTokenApiURL + authTokenRequest);
  while(1)
  {
    http.begin(AuthTokenApiURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    apiResponseCode=http.POST(authTokenRequest);
    //vTaskDelay(100/portTICK_PERIOD_MS);
    delay(100);
    if(apiResponseCode!=200)
    {
      authtokenCnt++;
      debugLog("Authtokenapi response code :  " );debugLogln(apiResponseCode);
      http.end();
      delay(2000);

      if(wifiStatus==0)
        reconnectingToWifi();
      
      if(authtokenCnt==apiHitCnt)
      {
        debugLogln("fail to get Authtoken");
        ESP.restart();
      }
    }
    else if(apiResponseCode==200)
    {
      break;
    }
    //vTaskDelay(100/portTICK_PERIOD_MS);
    delay(100);
  }
  apiResponse= http.getString();
  http.end();

  DeserializationError error = deserializeJson(doc, apiResponse);   

  if(!error)
  {
    String status= doc["status"];
    if(status=="201")
    {
      String authToken = doc["cluster_api_id"]; //here gataway ID
      //devsbotAuthToken= authToken;
      RWToEEPROM(authToken.c_str());
      debugLogln("Authentication token: "+ devsbotAuthToken+"\n");
    }

    else
    {
      debugLogln("Authentication token not received...!\n");
      //deviceLog("Authentication token not received...!\n");
      ESP.restart();
    }
  }
  else
  {
    debugLogln("API response file error\n");
    deviceLog("deserialization error in authtokenapi ");
  }
  debugLogln("AuthTokenAPI End");
}

void Devsbot:: deviceProvision()
{
  byte provisionCnt=0;
  debugLogln("deviceProvision begin");
  int responseStatus;
  debugLogln("Device provision starts...!\n");debugLog("Authtoken => " + devsbotAuthToken + "\n");
  deviceLog("Device provision starts...!\n");deviceLog("Authtoken : " + devsbotAuthToken + "\n");


  if(devsbotAuthToken.length() == 10 )
  {
    String deviceProvisionAPI = deviceProvisionURL + "?gateway_api_Id=" + String(devsbotAuthToken); //may be here it will take it as null
    //String deviceProvisionAPI = deviceProvisionURL + "?gateway_api_Id=" + String(devsbotAuthToken) + "&device_mac_Id=" + WiFi.macAddress(); //may be here it will take it as null
    debugLogln("Device provision request => "+ deviceProvisionAPI + "\n");   

    
    
    while(1)
    {
      http.begin(deviceProvisionAPI);
      apiResponseCode=http.GET();
      //vTaskDelay(100/portTICK_PERIOD_MS);
      delay(100);
      if(apiResponseCode!=200)
      {
        provisionCnt++;
        debugLog("deviceProvision response code : ");debugLogln(apiResponseCode);
        http.end();
        delay(2000);

        if(wifiStatus==0)
          reconnectingToWifi();
        
        if(provisionCnt==apiHitCnt)
        {
          debugLogln("fail to get a provision data device restart");
          ESP.restart();
        }
      }
      else if(apiResponseCode==200)
      {
        break;
      }
      //vTaskDelay(100/portTICK_PERIOD_MS);
      delay(100);
    }

    apiResponse= http.getString();
    debugLog("apiresponse String : ");debugLogln(apiResponse);
    DeserializationError error = deserializeJson(doc, apiResponse);
    if(!error)
    {
      responseStatus=doc["status"].as<int>();

      if(responseStatus!=200)
      {
        http.end();
        debugLogln("device not provisioned response status is not 200 read a old spiff data");
        debugLogln("device provision API response : " + String(apiResponse) + "\n");
        deviceLog("device not provisioned response status is not 200 read a old spiff data");
        deviceLog("device provision API response :  " +  apiResponse + "\n");
        return;
      }
      if (SPIFFS.exists("/Devsbot_Document.json"))
        SPIFFS.remove("/Devsbot_Document.json"); 
      File file = SPIFFS.open("/Devsbot_Document.json", "w");
      while (!file){}
      file.print(apiResponse);
      file.close();
      while(!SPIFFS.exists("/Devsbot_Document.json")){}
      debugLogln("Device Provisioned...!\n");
      deviceLog("Device Provisioned...!\n");
      http.end();   
    }
    else
    {
      debugLogln("apiResponse data file error\n");
      deviceLog("DeserializationError in deviceProvision" + String(error.f_str()));
    }
  }
  else
  {
    debugLog("Error in parsing a devsbotAuthToken  length : ");debugLogln(devsbotAuthToken.length());
    deviceLog("Error in parsing a devsbotAuthToken  length : " +  devsbotAuthToken + "\n");
  }
  debugLogln("deviceProvision End");
}


void Devsbot:: deviceProvisionData()
{
  debugLogln("deviceProvisionData begin");
  if (SPIFFS.exists("/Devsbot_Document.json"))
  {
    File file = SPIFFS.open("/Devsbot_Document.json", "r");

    while (file.available())
    {
      provisionData= file.readStringUntil('\n');
    }    
    file.close();

    debugLogln("Read device provision data=> "+ provisionData+"\n");
    DeserializationError error = deserializeJson(doc, provisionData);

    if (!error)
    {
      // if(devsbotAuthToken=="") //authtoken alredy readed form EEPROM
      // {
      //   String authToken = doc["device_auth_token"];
      //   devsbotAuthToken = authToken;
      //   debugLogln("Devsbot AuthToken=> "+ devsbotAuthToken+"\n");
      // }

      // String version = doc["Version"];
      // devsbotFirmwareVersion= version.toFloat();
      // debugLogln("Devsbot Firmware Version=> "+ String(devsbotFirmwareVersion)+"\n");

      String WidgetVersion = doc["widget_version"];
      devsbotWidgetVersion= WidgetVersion.toFloat();
      debugLogln("Devsbot Widget Version=> "+ String(devsbotWidgetVersion)+ "\n");

      String postSsid=doc["SSID"];
      devsbotSsid=postSsid;
      debugLogln("Devsbot SSID=> "+ String(devsbotSsid)+ "\n");
      
      String postPassword=doc["password"];
      devsbotPassword=postPassword;
      debugLogln("Devsbot password=> "+ String(devsbotPassword)+ "\n");

      String heartBeat=doc["heartbeat"];
      heartBeatInterval=heartBeat.toInt();
      debugLogln("heartBeatInterval=> "+ String(heartBeatInterval)+ "\n");

      instantaneousReading= doc["live_data"];
      debugLogln("instantaneousReading=> "+ String(instantaneousReading)+ "\n");

      String dataInterval=doc["data_interval"];
      completeJsonReading=dataInterval.toInt();
      debugLogln("completeJsonReading=> "+ String(completeJsonReading)+ "\n");
    }
    else
    {
      debugLogln("Provision data file error\n");
    }

  }
  debugLogln("deviceProvisionData End");
}

void Devsbot:: devicePreProvisionData()
{
  if (SPIFFS.exists("/Devsbot_Document.json"))
  {
    File file = SPIFFS.open("/Devsbot_Document.json", "r");

    while (file.available())
    {
      provisionData= file.readStringUntil('\n');
    }    
    file.close();

    debugLogln("Read device pre provision data=> "+ provisionData+"\n");

    DeserializationError error = deserializeJson(doc, provisionData);
    
    if (!error)
    {
      if(devsbotAuthToken=="") //authtoken alredy readed form EEPROM
      {
        debugLogln("read authtoken from EEPROM");
        ReadOnlyEEPROM();
        // debugLogln("Authtoken not read from EEPROM \n");
        // String authToken = doc["device_auth_token"];
        // devsbotAuthToken = authToken;
        // debugLogln("Device AuthToken=> "+ devsbotAuthToken+"\n");
      }


      // String version = doc["Version"];
      // deviceFirmwareVersion= version.toFloat();
      // debugLogln("Device Firmware Version=> "+ String(deviceFirmwareVersion)+"\n");

      String WidgetVersion= doc["widget_version"];
      deviceWidgetVersion= WidgetVersion.toFloat();
      debugLogln("Device Widget Version=> "+ String(deviceWidgetVersion)+ "\n");

      String preSsid=doc["SSID"];
      deviceSsid=preSsid;
      debugLogln("deviceSsid=> "+ String(deviceSsid)+ "\n");

      String prePassword=doc["password"];
      devicePassword=prePassword;
      debugLogln("devicePassword=> "+ String(devicePassword)+ "\n");

    }
    else
    {
      debugLogln("Devsbot provisiondata file error\n");
    }

  }
  else
  {
    debugLogln("preprovision No file in spiff \n");
  }
 
}

/*void Devsbot:: firmwareVersionVerification()
{
  if(devsbotFirmwareVersion> 1.0)
  {
    debugLogln("Device OTA Firmware updating\n");
    deviceLog("Device OTA Firmware updating");
    firmwareUpdate();
  }

  else
  {
    debugLogln("Device Restart\n");
    deviceLog("Device Restart");
    ESP.restart();
  }
}*/

void Devsbot::deviceFirmwareUpdate()
{
  if (devsbotFirmwareVersion > deviceFirmwareVersion)
  {
    debugLogln("The device is going to update the firmware\n");
    deviceLog("The device is going to update the firmware");
    firmwareUpdate();
  }

  // else if (devsbotFirmwareVersion == deviceFirmwareVersion)
  // {
  //   debugLogln("The firmware version is the same and the device is going to initialize a devspot connection\n");
  //   deviceLog("The firmware version is the same and the device is going to initialize a devspot connection");
  // }

  else if (devsbotFirmwareVersion < deviceFirmwareVersion)
  {
    debugLogln("The device is going to rollback the firmware\n");
    deviceLog("The device is going to rollback the firmware");
    firmwareUpdate();
  }
}

void Devsbot:: widgetBegin()
{
  debugLogln("Widget begin=> Devsbot widget version are checking...!\n");

  if (devsbotWidgetVersion> 0)
  {
    widgetAPI();
    widgetPinInitialize();
  }
}

void Devsbot:: widgetAPI()
{
  debugLogln("widgetAPI begin");
  String devsbotWidgetAPI = devsbotWidgetURL + "?cluster_id=" + devsbotAuthToken;
  debugLogln(devsbotWidgetAPI+"\n");

  
    http.begin(devsbotWidgetAPI);
    apiResponseCode=http.GET();
    //vTaskDelay(100/portTICK_PERIOD_MS);
    delay(100);
    if(apiResponseCode!=200)
    {
      debugLog("widgetAPI response code : "); debugLogln(apiResponseCode);
      deviceLog("widgetAPI response code" + String(apiResponseCode));
    }
    else if(apiResponseCode==200)
    {
      apiResponse = http.getString();
      debugLogln("Widget API response=> "+ apiResponse+"\n");
    }
    delay(100);
    http.end();
  

  SPIFFS.remove("/Devsbot_Widget.json"); 
  File file = SPIFFS.open("/Devsbot_Widget.json", "w");
  while (!file){}
  file.print(apiResponse);
  file.close();

  while(!SPIFFS.exists("/Devsbot_Widget.json")){}
  debugLogln("Device widget data updated...!\n");
  deviceLog("Device widget data updated\n");
  debugLogln("widgetAPI End");
}

void Devsbot:: widgetPinInitialize()
{
  debugLogln("widgetPinInitialize begin");
  digitalInputPin= "";
  digitalInputPullupPin= "";
  analogInputPin= "";

  while(!SPIFFS.exists("/Devsbot_Widget.json")){}
  File file = SPIFFS.open("/Devsbot_Widget.json", "r");

  while (file.available())
  {
    widgetData= file.readStringUntil('\n');
  }    
  file.close();
  debugLogln("Read devsbot widget data=> "+ widgetData+"\n");

  DeserializationError error = deserializeJson(doc, widgetData);

  if (!error)
  {
    for (JsonVariant elem : doc.as<JsonArray>()) 
    {    
      String devsbotDatastreamName = elem["datastream_name"];
      String devsbotWidgetPinMode = elem["pinmode"];

      if (devsbotDatastreamName == "Digital" && devsbotWidgetPinMode == "INPUT")
      {
        String dI = elem["pin"];
        digitalInputPin= dI;

        char setPin[digitalInputPin.length() + 1];
        digitalInputPin.toCharArray(setPin, digitalInputPin.length() + 1);
        const char* setDigitalInput = strtok(setPin, ",");
        while (setDigitalInput != NULL)
        {
          String digitalPin= String(setDigitalInput);
          pinMode(digitalPin.toInt(), INPUT);
          debugLogln("Digital Input=> "+ digitalPin+"\n");
          setDigitalInput = strtok(NULL, ",");
        }
      }

      if (devsbotDatastreamName == "Digital" && devsbotWidgetPinMode  == "INPUT_PULLUP")
      {
        String dIP = elem["pin"];
        digitalInputPullupPin= dIP;

        char setPin[digitalInputPullupPin.length() + 1];
        digitalInputPullupPin.toCharArray(setPin, digitalInputPullupPin.length() + 1);
        const char* setDigitalInputPullup = strtok(setPin, ",");
        while (setDigitalInputPullup != NULL)
        {
          String digitalPullupPin= String(setDigitalInputPullup);
          pinMode(digitalPullupPin.toInt(), INPUT_PULLUP);
          debugLogln("Digital Input Pullup=> "+ digitalPullupPin+"\n");
          setDigitalInputPullup = strtok(NULL, ",");
        }
      }

      if (devsbotDatastreamName == "Analog" && devsbotWidgetPinMode  == "INPUT")
      {
        String aI = elem["pin"];
        analogInputPin= aI;

        char setPin[analogInputPin.length() + 1];
        analogInputPin.toCharArray(setPin, analogInputPin.length() + 1);
        const char* setAnalogInput = strtok(setPin, ",");
        while (setAnalogInput != NULL)
        {
          String analogPin= String(setAnalogInput);
          pinMode(analogPin.toInt(), INPUT);
          debugLogln("Analog Input=> "+ analogPin+"\n");
          setAnalogInput = strtok(NULL, ",");
        }
      }

    }
  }
  else
  {
    debugLogln("Widget data file error\n");
    deviceLog("deserilization error in widgetpininitialization");
  }
  debugLogln("widgetPinInitialize End");
}



void Devsbot::firmwareUpdate()
{
  String devsbotOTA= devsbotOTAUpdateURL+ "?cluster_api_Id=" + devsbotAuthToken + "&version="+ String(devsbotFirmwareVersion, 1);
  debugLogln("\nOTA Request=> "+ devsbotOTA+ "\n");


  httpota.begin(devsbotOTA);
  int httpCode = httpota.GET();
  if (httpCode != HTTP_CODE_OK) 
  {
    Serial.printf("Failed to fetch firmware: %s\n", httpota.errorToString(httpCode).c_str());
    deviceLog("fail to fetch a firmware response code : " + String(httpCode));
    httpota.end();
    return;
  }
 
  
  int contentLength = httpota.getSize();
  bool canBegin = Update.begin(contentLength);

  if (!canBegin) 
  {
    Serial.println("Not enough space to begin OTA");
    httpota.end();
    return;
  }

  WiFiClient* stream = httpota.getStreamPtr();
  size_t written = 0;

  const int chunkSize = 1024;
  uint8_t buff[chunkSize] = {0};

  while (httpota.connected() && written < contentLength) 
  {
    Serial.println("inside a while of httpota");
    size_t available = stream->available();
    Serial.print("available");Serial.println(available);
    if (available) 
    {
      size_t len = stream->readBytes(buff, ((available > sizeof(buff)) ? sizeof(buff) : available));
      Serial.print("len : ");Serial.println(len);
      size_t writtenThisChunk = Update.write(buff, len);
      Serial.print("writtenThisChunk");Serial.println(writtenThisChunk);
      if (writtenThisChunk != len) 
      {
        Serial.println("Failed to write the received chunk to flash");
        httpota.end();
        Update.end();
        break;
      }
      written += writtenThisChunk;
      Serial.printf("Written: %u/%u bytes\n", written, contentLength);
      Serial.println();
    }
    vTaskDelay(10/portTICK_PERIOD_MS); // Small delay to allow other tasks
  }

  if (Update.end(true)) 
  {
    Serial.println("OTA update completed successfully");
    if (Update.isFinished()) 
    {
      firmwareVersionSend();
      Serial.println("Update successfully finished. Rebooting...");
      ESP.restart();
    } 
    else 
    {
      Serial.println("Update not finished? Something went wrong!");
      ESP.restart();
    }
  } 
  else 
  {
    Serial.printf("Error Occurred. Error #: %u\n", Update.getError());
    ESP.restart();
  }

  httpota.end();
}




void Devsbot:: firmwareVersionSend()
{
  String firmwareVersion = "device_auth_token=" + String(devsbotAuthToken) + "&version=" + String(devsbotFirmwareVersion, 1);
  byte firmversionCnt=0;
  while(1)
  {
    http.begin(firmwareVersionURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    apiResponseCode=http.POST(firmwareVersion);
    delay(100);
    if(apiResponseCode!=200)
    {
      firmversionCnt++;
      debugLog("firmwareVersionSend response code ");debugLogln(apiResponseCode);
      http.end();
      delay(2000);

      if(wifiStatus==0)
        reconnectingToWifi();

      if(firmversionCnt==apiHitCnt)
      {
        debugLogln("fail to post a firmwareVersion");
        return;
      }
     
    }
    else if(apiResponseCode==200)
    {
      break;
    }
    delay(100);
  }
  apiResponse = http.getString();
  debugLogln("Firmware version API data=> "+ apiResponse+"\n");
  http.end();

  //Store a devsbotFrimware version to a EEPROM.when a device restarts it will get a deviceFrimwareVersion form EEPROM .
  preferences.begin("version", false);
  preferences.putFloat("firmVersion",devsbotFirmwareVersion);
  preferences.end();
}

void Devsbot::socketIOConnection()
{
  debugLogln("socketIOConnection begin");
  debugLogln("Devsbot connection initializing\n");
  //socketIO.begin("192.168.0.118",3002,"/niraltek/socket.io/?EIO=4");
  //socketIO.begin("68.183.85.221",80,"/niraltek/socket.io/?EIO=4");
  socketIO.beginSSL("energy.devsbot.com",443,"/socket.io/?EIO=4");
  //socketIO.beginSSL("sem-demo.devsbot.com",443,"/niraltek/socket.io/?EIO=4");
  //socketIO.beginSSL("sem-demo.devsbot.com",443,"/socket.io/?EIO=4");
  socketIO.onEvent(socketIOEvent);
  debugLogln("socketIOConnection End");
}


void Devsbot::socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length)
{
  debugLogln("socketIOEvent begin");
  debugLogln("Devsbot connection status=> "+ String(type)+"\n");
  static String dbOutputPin;

  switch (type)
  {
    case sIOtype_DISCONNECT:  //49
      sIOConnectionStatus = type;
      debugLogln("Device disconnected with Devsbot API\n");
      dBot.deviceLog("Device disconnected with Devsbot API\n");
      dBot.sioDisconnect=1;
      break;

    case sIOtype_CONNECT:   // 48
      sIOConnectionStatus = type;
      debugLogln("Device connected with Devsbot API\n");
      dBot.deviceLog("Device connected with Devsbot API\n");
      socketIO.send(sIOtype_CONNECT, "/");
      dBot.sioDisconnectFlag=1;
      dBot.sioDisconnect=0;
      
      break;

    case sIOtype_EVENT:  //50
      {
        String dbPayloadContent = String((const char*)payload);
        dbPayloadContent.remove(0, 2);
        dbPayloadContent.remove((dbPayloadContent.length() - 2), 2);
        dbPayloadContent.replace("\\", "");

        DeserializationError error = deserializeJson(doc, dbPayloadContent);

        if (error)
        {
          debugLog(F("deserializeJson() failed: socketio"));
          debugLogln(error.f_str());
          return;
        }

        String dbMethod = doc["Method"];
        String dbPin = doc["Pin"];
        String dbPinmode = doc["Pinmode"];
        String dbPayload = doc["Payload"];

        if(doc.containsKey("Version"))
        {
          debugLogln("contains key of version");
          dBot.devsbotFirmwareVersion=doc["Version"];
        }

        debugLogln("Method=> "+ dbMethod+ "\n");
        debugLogln("Pin=> "+ dbPin+ "\n");
        debugLog("Payload=> ");debugLogln(dbPayload+"\n");
        

        if (dbMethod == "Digital")
        {
          if(dbPin != dbOutputPin)
          {
            pinMode(dbPin.toInt(), OUTPUT);
            dbOutputPin= dbPin;
          }
          digitalWrite(dbPin.toInt(), dbPayload.toInt());
          debugLogln("Digital OUTPUT\n");
        }

        else if (dbMethod == "Analog")
        {
          analogWrite(dbPin.toInt(), dbPayload.toInt());
          debugLogln("Analog OUTPUT\n");
        }

        else if(dbMethod == "OTAUpdate")
        {
          debugLogln("firmware update");
          dBot.deviceFirmwareUpdate();
        }

        else if (dbMethod == "Restart")
        {
          debugLogln("Device restart\n");
          dBot.deviceLog("Device restart\n");
          ESP.restart();
        }

      }
  }
  debugLogln("socketIOEvent End");
}


bool Devsbot:: checkServerConnection(uint16_t port) 
{
  WiFiClient clientServer;
  uint64_t checkServerStart=millis();
  while (!clientServer.connect(hostname,port)) 
  {
    if (millis() - checkServerStart > 10000) 
    { // 10 seconds timeout
      return 0;
    }
    delay(100); // Small delay to prevent overwhelming the server with connection attempts
  }
  clientServer.stop(); // Close the connection if it was successful
  return 1;
}



// bool Devsbot:: checkServerConnection(uint16_t port) 
// {
//   uint64_t checkServerStart=millis();
//   uint64_t checkServerEnd=checkServerStart + 10000;
//   WiFiClient clientServer;
//   bool tempCheckServer;
//   tempCheckServer=clientServer.connect(hostname,port);
//   //tempCheckServer=client.connect("192.168.0.119",port);
//   if(tempCheckServer!=1)
//   {
//     while(checkServerStart<=checkServerEnd)
//     {
//       checkServerStart=millis();
//       tempCheckServer=clientServer.connect(hostname,port);
//       if(tempCheckServer==1)
//         break;
//       vTaskDelay(100/portTICK_PERIOD_MS);
//     }
//   }
//   return tempCheckServer;
// }

void Devsbot:: Loop()
{
  //debugLogln("devsbotLoop begin");
   
  socketIO.loop();
  delay(200);
  // vTaskDelay(200/portTICK_PERIOD_MS);


 if ((sIOConnectionStatus == 48)) // ascii value of '0' is 48 means sIOtype_CONNECT
  {
    //Serial.print("socketio is connected : ");Serial.println(socketIO.isConnected());
    devsbotAuthentication(); //join channel
    devsbotStatus();
    widgetUpdate();
    sensorInput();
  }
  else
  {
    debugLogln("sIOConnection is not 48");
    //Serial.print("socketio is connected : ");Serial.println(socketIO.isConnected());
  }

  if(sioDisconnect==1  && sioDisconnectFlag==1)
  {
    sioDisconnectFlag=0;
    sioDisconnectStart=millis();
    Serial.print("sioDisconnectStart in sec : ");Serial.println(sioDisconnectStart/1000);
  }

  if(sioDisconnect==1)
  {
    uint64_t currentLoopTime=millis();
    //Serial.print("currentLoopTime in sec : ");Serial.println(currentLoopTime/1000);
    Serial.print("time difference : ");Serial.println((currentLoopTime-sioDisconnectStart)/1000);
    if((currentLoopTime-sioDisconnectStart>=30000))
    {
      debugLogln("....sIOConnection callback failed device restart...\n");
      ESP.restart();
    }
  }

  if(wifiStatus==0)
  {
    debugLogln("device disconnected or lost its IP");
    reconnectingToWifi();
  }

  //debugLogln("devsbotLoop End\n");
}

/*!
 *    @brief Device verification to establish connection and initiate communication between device and devsbot app
*/
void Devsbot:: devsbotAuthentication()
{
  //debugLogln("devsbotAuthentication begin");
  uint64_t dbNow = millis();

  if (dbNow - dbMessageTimestamp > 2000)
  { 
    dbMessageTimestamp = dbNow;


    debugLogln("Device firmware version=> "+ String(deviceFirmwareVersion,1)+ "\n");
  
    
    JsonArray array = doc.to<JsonArray>();

    array.add("joinchannel");

    JsonObject param1 = array.createNestedObject();
    //param1["devicelink"] = devsbotAuthToken;
    param1["clusterid"] = devsbotAuthToken;
    String dbInitalAuthentication;
    serializeJson(doc, dbInitalAuthentication);

    debugLogln("Device authentication data=> "+ dbInitalAuthentication+ "\n");
    bool tempConnectivity=checkServerConnection(443);
    debugLog("checkInternetConnectivity : ");debugLogln(tempConnectivity);
    if(tempConnectivity==1)
    {
      if(socketIO.sendEVENT(dbInitalAuthentication))
      {
        debugLogln("dbInitalAuthentication was success");
      }
      else
      {
        debugLogln("dbInitalAuthentication was failed");
      }
    }
    else
    {
      debugLogln("devsbotAuthentication response code is -1\n");
      if(wifiStatus==0)//internet connection is lost
        reconnectingToWifi();
    }
 }
 // debugLogln("devsbotAuthentication End\n");

}

/*!
 *    @brief The device will send the live status to the devsbot app and get the widget version from the devsbot app server
*/
void Devsbot::devsbotStatus()
{
  byte statusCnt=0;
 // debugLogln("devsbotStatus begin");

  uint64_t devsbotStatus_now = millis();

  if ((devsbotStatus_now - devsbotStatus_Time > (heartBeatInterval*1000)) || (aliveState==true))
  {
    aliveState=false;
    int8_t wifiSignal=WiFi.RSSI();
    String devsbotAliveData = "device_auth_token=" + devsbotAuthToken + "&status_Id=1" + "&wifi=" + String(wifiSignal);
    debugLogln(devsbotAliveData);
    devsbotStatus_Time = devsbotStatus_now;
    
    while(1)
    {
      Serial.println(devsbotDeviceStatusURL + devsbotAliveData);
      http.begin(devsbotDeviceStatusURL);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      Serial.print("devsbotAliveData");Serial.println(devsbotAliveData);
      apiResponseCode=http.POST(devsbotAliveData);
      delay(100);
      if(apiResponseCode!=200) // may be -1,500,
      {
        statusCnt++;
        debugLog("devsbotStatus response code :");debugLogln(apiResponseCode);
        http.end();
        delay(2000);

        if(wifiStatus==0)
          reconnectingToWifi();

        if(statusCnt==apiHitCnt)
        {
          debugLogln("fail to post a devsbotStatus");
          return;
        }
        
        
        
      }
      else if(apiResponseCode==200)
      {
        break;
      }
   
    }
    apiResponse = http.getString();

    debugLogln("Device alive status API data=> "+ apiResponse+ "\n");
    http.end();
    DeserializationError error = deserializeJson(doc, apiResponse);

    if (error)
    {
      debugLog(F("deserializeJson() failed: devsbotstatus"));
      debugLogln(error.f_str());
      return;
    }

    int statusJson=doc["status"].as<int>();
    Serial.print("status key value in json response : ");Serial.println(statusJson);
    if(statusJson==201)
    {
      // if(doc.containsKey("FirmwareVersion"))
      // {
      //   devsbotFirmwareVersion=doc["FirmwareVersion"].as<float>();
      //   Serial.println("contains firmware key");
      // }

     
      devsbotFirmwareVersion=doc["device_version"].as<float>();
      debugLogln("devsbotFirmwareVersion=> " + String(devsbotFirmwareVersion)+"\n");


      devsbotWidgetVersion = doc["widget_version"];
      debugLogln("Devsbot widget version=> " + String(devsbotWidgetVersion)+"\n");

      String devsbotheartbeat=doc["heartbeat"];
      devsbotheartBeatInterval=devsbotheartbeat.toInt();

      String devsbotdatainterval=doc["data_interval"];
      devsbotDataInterval=devsbotdatainterval.toInt();

      devsbotliveData=doc["live_data"];

      numSlave=doc["slave_id_count"];

      JsonArray slaveId = doc["slave_id"];
      //Serial.print("no of slave : ");Serial.println(slaveId.size());
  
      for (int i = 0; i < slaveId.size(); i++) 
      {
        slaveIdArray[i] = slaveId[i]; // Assuming values are within the byte range (0-255)
        //Serial.println(slaveIdArray[i]); // Printing the values to the serial monitor
      }

    }
    else
    {
      debugLogln("device authtoken : "+ String(devsbotAuthToken)  +" device status API response : " + String(apiResponse) + "\n");
      deviceLog("device status API response : " +  apiResponse + "\n");
      debugLogln("status key value in json response is not equal to 201\n");
      deviceLog("status key value in json response is not equal to 201 and response status :  " + String(statusJson));
      deviceLog("device authtoken : "+ devsbotAuthToken + "\n");
      //vTaskDelay(100/portTICK_PERIOD_MS);
      delay(100);
    }
  }
   // debugLogln("devsbotStatus End\n");

}


void Devsbot:: widgetUpdate()
{
 // debugLogln("widgetUpdate begin");
  if(devsbotFirmwareVersion!=deviceFirmwareVersion)
  {
    deviceLog("devicefirmware and devsbotfirmware version is not same");
    debugLogln("devicefirmware and devsbotfirmware version is not same");
    deviceFirmwareUpdate();
  }

  if(deviceWidgetVersion != devsbotWidgetVersion)
  {
    debugLogln("Widget data update starts...!\n");
    deviceLog("Widget data update starts...!\n");
    widgetAPI();
    widgetPinInitialize();
    widgetVersionEdit();
  }
  if(heartBeatInterval!=devsbotheartBeatInterval)
  {
    heartBeatInterval=devsbotheartBeatInterval;
    debugLogln("updated heartBeatInterval=> " + String(heartBeatInterval)+"\n");
    deviceLog("updated heartBeatInterval :  " + String(heartBeatInterval) + "\n");
  }
  if(instantaneousReading!=devsbotliveData)
  {
    instantaneousReading=devsbotliveData;
    debugLogln("updated instantaneousReading=> " + String(instantaneousReading)+"\n");
    deviceLog("updated instantaneousReading=> " + String(instantaneousReading)+"\n");
  }
  if(completeJsonReading!=devsbotDataInterval)
  {
    completeJsonReading=devsbotDataInterval;
    debugLogln("updated completeJsonReading=> " + String(completeJsonReading)+"\n");
    deviceLog("updated completeJsonReading=> " + String(completeJsonReading)+"\n");
  }
 // debugLogln("widgetUpdate End\n");
}

void Devsbot:: widgetVersionEdit()
{
  deviceWidgetVersion= devsbotWidgetVersion;
}

void Devsbot::sensorInput()
{
 // debugLogln("sensorInput begin");

  if (deviceWidgetVersion == devsbotWidgetVersion)
  {
    if(digitalInputPin != NULL)
    {
      debugLogln("Digital Sensor Pins=> "+ digitalInputPin+"\n");

      char dataPin[digitalInputPin.length() + 1];
      digitalInputPin.toCharArray(dataPin, digitalInputPin.length() + 1);
      const char* digitalSensorInput = strtok(dataPin, ",");

      while (digitalSensorInput != NULL)
      {
        String digitalPin= String(digitalSensorInput);
        jsonInputSend("digitalinput", digitalPin, String(digitalRead(digitalPin.toInt())));
        //debugLogln("Digital Input data=> "+ String(digitalRead(digitalPin.toInt())));
        digitalSensorInput = strtok(NULL, ",");
      }
    }

    if(digitalInputPullupPin != NULL)
    {
      debugLogln("Digital Pullup Sensor Pins=> "+ digitalInputPullupPin+"\n");

      char dataPin[digitalInputPullupPin.length() + 1];
      digitalInputPullupPin.toCharArray(dataPin, digitalInputPullupPin.length() + 1);
      const char* digitalSensorInputPullup = strtok(dataPin, ",");

      while (digitalSensorInputPullup != NULL)
      {
        String digitalPullupPin= String(digitalSensorInputPullup);
        jsonInputSend("digitalinput", digitalPullupPin, String(digitalRead(digitalPullupPin.toInt())));
        //debugLogln("Digital Input Pullup data=> "+ String(digitalRead(digitalPullupPin.toInt())));
        digitalSensorInputPullup = strtok(NULL, ",");
      }   
    }

    if(analogInputPin != NULL)
    {
      debugLogln("Analog Sensor Pins=> "+ analogInputPin+"\n");

      char dataPin[analogInputPin.length() + 1];
      analogInputPin.toCharArray(dataPin, analogInputPin.length() + 1);
      const char* analogSensorInput = strtok(dataPin, ",");

      while (analogSensorInput != NULL)
      {
        String analogPin= String(analogSensorInput);
        jsonInputSend("analoginput", analogPin, String(analogRead(analogPin.toInt())));
        //debugLogln("Analog Input data=> "+ String(analogRead(analogPin.toInt())));
        analogSensorInput = strtok(NULL, ",");
      }  
    }
  }
  delay(100);
 // debugLogln("sensorInput End\n");
}


/*!
 * @brief Send custom sensor data and other required data to devsbot server for monitoring purpose
 * @param dbVirtualPin, A widget's virtual pin is required to send data to the devsbot server
 * @param dbVirtualData, Device tracking integer data is sent to the devsbot app and displayed in a widget on the dashboard
*/
void Devsbot::sendVirtualWrite(byte dbVirtualPin, int dbVirtualData) // only send numerical value in virtualpin and send data in virtualData.
{
  jsonInputSend("virtualinput", ("V" + String(dbVirtualPin)), String(dbVirtualData));
}

/*!
 * @brief Send custom sensor data and other required data to devsbot server for monitoring purpose
 * @param dbVirtualPin, A widget's virtual pin is required to send data to the devsbot server
 * @param dbVirtualData, Device tracking string data is sent to the devsbot app and displayed in a widget on the dashboard
*/
void Devsbot::sendVirtualWrite(byte dbVirtualPin, String dbVirtualData) // only send numerical value in virtualpin and send data in virtualData.
{
  jsonInputSend("virtualinput", ("V" + String(dbVirtualPin)), dbVirtualData);
}

// void Devsbot::sentEnergyMeterData(String EnergyString)
// {
//   jsonInputSend("MeterJsonData",EnergyString);  //meterjsondata is event handler 
// }

void Devsbot::sentEnergyMeterData(String EnergyString)
{
  byte meterdataCount=0;
  //debugLogln("sentEnergyMeterData begin");
  String meterString = "device_auth_token=" + devsbotAuthToken + "&device_value=" + EnergyString;

  while(1)
  {
    http.begin(energymeterJsonURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    apiResponseCode=http.POST(meterString);
    delay(100);
    if(apiResponseCode!=200)
    {
      meterdataCount++;
      debugLog("sentEnergyMeterData response code ");debugLogln(apiResponseCode);
      debugLogln("post the meter data again\n");
      http.end();
      delay(2000);

      if(wifiStatus==0)
        reconnectingToWifi();

      if(meterdataCount==apiHitCnt)//0,1,2,3,4,5
      {
        debugLogln("fail to post a meterdata");
        return;
      }
    }
    else if(apiResponseCode==200)
    {
      break;
    }
  }
  //vTaskDelay(100/portTICK_PERIOD_MS);
  delay(100);

  apiResponse = http.getString();
  debugLog("sentEnergyMeterData : ");debugLogln(apiResponse);
  http.end();


    /*DeserializationError error = deserializeJson(doc, apiResponse);

    if (error)
    {
      debugLog(F("deserializeJson() failed: sentEnergyMeterData"));
      debugLogln(error.f_str());
      return;
    }

    int jsonstatus = doc["status"].as<int>();

    if(jsonstatus==201)
    {
      debugLogln("energy meter json String posted sucessfully");
    }
    else
    {
      debugLogln(" fail in sending energy meter json String apiresponse : " + apiResponse);
      deviceLog(" fail in sending energy meter json String apiresponse : " + apiResponse);
    }*/
  //debugLogln("sentEnergyMeterData End");
}

/*!
 *    @brief Here device will send information about input method, input pin number and tracking data to the devsbot server
 *    @param dbInputMethod, The input type is useful for identifying which data came from the device to the devsbot server
 *    @param dbInputPin, The input pin description is useful to match the widget pin description on the dashboard in the devsbot app
 *    @param dbDeviceValue, The tracking data will be sent to the devsbot server and will view the data on the dashboard
*/
void Devsbot::jsonInputSend(String dbInputMethod, String dbInputPin, String dbDeviceValue)
{
  String devsbotInput;
  JsonArray array1 = doc.to<JsonArray>();

  array1.add(dbInputMethod);
  JsonObject param2 = array1.createNestedObject();

  param2["devicelink"] = devsbotAuthToken;
  param2["input_pin"] = dbInputPin;
  param2["device_value"] = dbDeviceValue;

  serializeJson(doc, devsbotInput);
  debugLogln(devsbotInput+"\n");

  socketIO.sendEVENT(devsbotInput);
}

void Devsbot::jsonInputSend(String MeterJsonData,String Energyjson )
{
  String jsonString;
  JsonArray array1 = doc.to<JsonArray>();

  array1.add(MeterJsonData);
  JsonObject param2 = array1.createNestedObject();

  param2["devicelink"] = devsbotAuthToken;
  param2["device_value"] = Energyjson;

  serializeJson(doc, jsonString);
  debugLogln(jsonString+"\n");

  socketIO.sendEVENT(jsonString);
  debugLogln(" Energymeter data sent successfully ");
}





/*!
 *    @brief The device logs data to the server to know what is happening with the device while it is running
 *    @param dbDeviceLogData, It contains information about the operational information of the device while the device is running
*/

void Devsbot:: deviceLog(String dbDeviceLogData)
{
  String devsbotData = "gateway_api_Id=" + devsbotAuthToken + "&device_log=" + dbDeviceLogData;
  byte logCnt=0;
  while(1)
  {
    http.begin(devsbotDeviceLogURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
    int deviceLogapiResponseCode=http.POST(devsbotData);
    if(deviceLogapiResponseCode!=200)
    {
      logCnt++;
      debugLogln("deviceLog response code " + String (deviceLogapiResponseCode));
      http.end();
      delay(2000);

      if(wifiStatus==0)
        reconnectingToWifi();

      if(logCnt==apiHitCnt)
      {
        debugLogln("fail to post a deviceLog ");
        return;
      }
    }
    else if(deviceLogapiResponseCode==200)
    {
      break;
    }
  }
  String deviceLogapiResponse = http.getString();
  debugLogln("deviceLogapiResponse  : " + deviceLogapiResponse);
  http.end();
}




/*void Devsbot:: deviceLog(String dbDeviceLogData)
{
  String devsbotData = "device_auth_token=" + devsbotAuthToken + "&device_log=" + dbDeviceLogData;
  http.begin(devsbotDeviceLogURL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
  int deviceLogapiResponseCode=http.POST(devsbotData);
  if(deviceLogapiResponseCode!=200)
  {
    debugLogln("deviceLog response code " + String (deviceLogapiResponseCode));
  }
  else if(deviceLogapiResponseCode==200)
  {
    String deviceLogapiResponse = http.getString();
    debugLogln("deviceLogapiResponse  : " + deviceLogapiResponse);
  }
  http.end();
}*/


/*void Devsbot:: deviceLog(String dbDeviceLogData)
{
  String devsbotData = "device_auth_token=" + devsbotAuthToken + "&device_log=" + dbDeviceLogData;
  
  while(1)
  {
    http.begin(devsbotDeviceLogURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
    int deviceLogapiResponseCode=http.POST(devsbotData);
    if(deviceLogapiResponseCode==200)
    {
      break;
    }
    else if(deviceLogapiResponseCode==-1)
    {
      debugLogln("deviceLog response code is -1 \n");
      reconnectingToWifi();
    }
    vTaskDelay(200/portTICK_PERIOD_MS);
  }
  String deviceLogapiResponse = http.getString();
  http.end();
}*/

void Devsbot:: deviceLogOTA(String dbDeviceLogOTAData)
{
  // String devsbotData = "device_auth_token=" + devsbotAuthToken + "&device_log=" + dbDeviceLogOTAData;
  // client.setInsecure();
  // http.begin(devsbotDeviceLogURL);
  // http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  // while(1)
  // {
  //   apiResponseCode=http.POST(devsbotData);
  //   if(apiResponseCode==200)
  //   {
  //     break;
  //   }
  //   else if(apiResponseCode==-1)
  //   {
  //     debugLogln("something went wrong during a OTA update");
  //     ESP.restart();
  //   }
  // }
  // apiResponse = http.getString();
  // http.end();
}


void Devsbot::deviceContinue()
{
  socketIO.loop();
  devsbotStatus();
}



/*!
 *    @brief Custom devsbot delay function to maintain stable connection between devsbot server and device
*/
void Devsbot::devsbotDelay(uint64_t dbMilliSeconds)
{
  uint64_t dbPauseNow = millis();
  uint64_t dbPauseDelay = dbPauseNow + dbMilliSeconds;
  while (dbPauseNow <= dbPauseDelay)
  {
    dbPauseNow = millis();
    socketIO.loop();
    devsbotStatus();
  }
}

Devsbot dBot;