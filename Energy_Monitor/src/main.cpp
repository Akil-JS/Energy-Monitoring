#include <DevsbotEnergyLocal.h>
#include<ArduinoJson.h>
#include<ModbusMaster.h>
//#include <SoftwareSerial.h>


#define DBOT_GATEWAY_ID "wUKHsBHuuw"

//connecting to a office Wifi
// #define WIFI_SSID "NTSOTA"
// #define WIFI_PASSWORD "ntsota@12345"


#define WIFI_SSID "IoT NA3S"
#define WIFI_PASSWORD "iot@na3s"

//pin defnition of modbus
#define MAX485_DE      19//Driver output Enable pin DE Active HIGH
#define MAX485_RE_NEG  18 //Receiver output Enable pin RE Active LOW


void sendValueToVirtualPin(String,byte);


ModbusMaster node;


unsigned long previousmillis=0,previousmillis1=0;
unsigned long currentmillis;

int modbusTimeOut=2000;

//register address corresponds to Selec meter.
// uint16_t regAddr[] = {0x1A, 0x00, 0x14, 0x16, 0x18, 0x0E};
// uint16_t noRegToRead[]={2,2,2,2,2,2};
// const char* vPins[] = {"V0", "V1", "V3", "V4", "V5", "V6"};  // virtual pins you may change here.
// byte noParam[]={1,1,1,1,1,1};


//register address corresponds to Elmesure meter.
// uint16_t regAddr[]={0x9C,0x9E,0x84,0x94,0x74,0x64,0x15E,0x96,0x98,0x9A,0x76,0x78,0x7A,0x66,0x68,0x6A,0xF2,0xF4,0xF6,0x8E,0x90,0x92};
// uint16_t noRegToRead[]={2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
// const char* vPins[] = {"V0", "V1", "V3", "V4", "V5", "V6","V7","V8","V12","V11","V9","V2"};  // virtual pins you may change here.
// byte noParam[]={1,1,1,1,1,1,1,3,3,3,3,3};

//register address corresponds to Selec  meter.
// uint16_t regAddr[]={0x38,0x60,0x0E,0x16,0x36,0x2A,0x10,0x12,0x14,0x30,0x32,0x34,0x18,0x1A,0x1C,0x54,0x56,0x58,0x00,0x02,0x04};//fr,Kwh,vLL,Iavg,pfavg,kwTotal,co2,Ir,Iy,Ib,pfr,pfy,pfb,KWr,KWy,KWb,kwhR,kwhy,kwhb,Vrn,Vyn,Vbn
// uint16_t noRegToRead[]={2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
// const char* vPins[] = {"V0", "V1", "V3", "V4", "V5", "V6","V8","V12","V11","V9","V2"};  // virtual pins you may change here.
// byte noParam[]={1,1,1,1,1,1,3,3,3,3,3};


//register address corresponds to Schneider electric
// uint16_t regAddr[] = {3109,3203,3025,3009,3191,3059,2999,3001,3003,3077,3079,3081,3053,3055,3057,3027,3029,3031};// {Fr,wh,VLL,Iavg,pfIEC,totalPower,Ir,Iy,Ib,pfr,pfy,pfb,pr,py,pb,vrn,vyn,vbn}
// uint16_t noRegToRead[]={2,4,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
// const char* vPins[] = {"V0", "V1", "V3", "V4", "V5", "V6","V8","V12","V11","V2"};// virtual pins you may change here.
// byte noParam[]={1,1,1,1,1,1,3,3,3,3};


//register address corresponds to Schneider electric
uint16_t regAddr[] = {156,158,132,148,116,100,150,152,154,118,120,122,102,103,106,134,136,138};// {Fr,wh,VLL,Iavg,pfIEC,totalPower,Ir,Iy,Ib,pfr,pfy,pfb,pr,py,pb,vrn,vyn,vbn}
//{Fr,wh,VLL,Iavg,pfIEC,totalPower,Ir,Iy,Ib,pfr,pfy,pfb,pr,py,pb,vrn,vyn,vbn}

uint16_t noRegToRead[]={2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
const char* vPins[] = {"V0", "V1", "V3", "V4", "V5", "V6","V8","V12","V11","V2"};// virtual pins you may change here.
byte noParam[]={1,1,1,1,1,1,3,3,3,3};





const byte size=sizeof(regAddr)/sizeof(regAddr[0]);
const byte sizevPins=sizeof(vPins)/sizeof(vPins[0]);
byte keyValThree=0;
char* values[]={"R","Y","B"};


void postTransmission()   //Set up call back function9
{
  digitalWrite(MAX485_RE_NEG, LOW);
  digitalWrite(MAX485_DE, LOW);
}

void preTransmission()  //Set up call back function
{
  digitalWrite(MAX485_RE_NEG, HIGH);
  digitalWrite(MAX485_DE, HIGH);
}


void setup()
{
  Serial.begin(115200);
  Serial2.begin(9600,SERIAL_8N1); //serial 2: RX2 and TX2 in Arduino Mega, SERIAL_8E1 8bit data,evenparity,1stopbit
  dBot.begin(DBOT_GATEWAY_ID,WIFI_SSID,WIFI_PASSWORD);
  //dBot.begin();

  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);

 // Init in receive mode
  digitalWrite(MAX485_RE_NEG, LOW);
  digitalWrite(MAX485_DE, LOW);

  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  byte i=0;
  while(i<sizevPins)
  {
    if(noParam[i++]==3)
      keyValThree++;
  }
  Serial.print("keyValthree : ");Serial.println(keyValThree);
}


float powerfactorcalculation(float floatValue) //power factor calculation for each phase as per IEEE
{
  float pfVal=0.0;

  if (floatValue > 1)
  {
    pfVal = 2 - (floatValue);
  }
  else if (floatValue < -1)
  {
    pfVal = -2-(floatValue);
    //PF is leading
  }

  return pfVal;

}



void readModbusJson(uint8_t slaveNum=1)
{
  //Serial.printf("readModbusJson begin\n");
  const size_t capacity = JSON_ARRAY_SIZE(slaveNum) + slaveNum * (JSON_OBJECT_SIZE(sizevPins+2) + keyValThree* JSON_OBJECT_SIZE(3));
  Serial.printf("capacity :%d \n",capacity);
  DynamicJsonDocument docJson1(capacity + 1024);
  JsonObject meterData;
  JsonObject phaseQty;
  JsonArray metersArray;

  uint8_t result;
  float floatValue;
  byte slaveFailCount=0;
  byte count=0;
  bool timeOutFlag=0;
  bool cnt=true;
  uint16_t data[2];
  metersArray = docJson1.to<JsonArray>();
  for (byte slave = 0; slave < slaveNum; slave++)
  {
    Serial.printf("slave : %d\n",dBot.slaveIdArray[slave]);
    //Serial.printf("slave : %d\n",slave);
    node.begin(dBot.slaveIdArray[slave],Serial2);
    //node.begin(slave,Serial2);
    dBot.deviceContinue();
    for(byte i=0,j=0;j<sizevPins,i<size;i++) //sizevPins=6,size=6
    {
      //result = node.readHoldingRegisters(regAddr[i],noRegToRead[i]); // Read two registers starting at address 0x0000
      result = node.readHoldingRegisters(regAddr[i],noRegToRead[i]); // Read two registers starting at address 0x0000
      //vTaskDelay(200/portTICK_PERIOD_MS);
      delay(50);

      uint64_t checkStartTimer = millis();
      uint64_t checkEndTimer = checkStartTimer + modbusTimeOut;
      while (result!=node.ku8MBSuccess)
      {
        checkStartTimer=millis();
        Serial.print("Error reading registers: ");
        Serial.println(result);
        //result = node.readHoldingRegisters(regAddr[i],noRegToRead[i]); // Read two registers starting at address 0x0000
        result = node.readInputRegisters(regAddr[i],noRegToRead[i]); // Read two registers starting at address 0x0000
        //vTaskDelay(200/portTICK_PERIOD_MS);
        delay(50);
        if(checkEndTimer<=checkStartTimer)
        {
          timeOutFlag=1;
          slaveFailCount++;
          count=0; //if 3ph values are read from a slaves device ,in that instance a time out error occured and program move to nxt slave 
          cnt=true;// in that instance while reading a 3ph qty  cnt=false ,count=1.
          Serial.printf("failcount: %d\n",slaveFailCount);
          if(slaveFailCount==slaveNum)
          {
            dBot.sentEnergyMeterData("[]");
            docJson1.clear();
            return;
          }
          break;
        }
      }

      if(timeOutFlag==1)
      {
        timeOutFlag=0;
        break;
      }
      if (i == 0)
      {
        meterData=metersArray.createNestedObject();
        meterData["slave_id"]=dBot.slaveIdArray[slave];
        //meterData["slave_id"]=slave;
      }

      if(noRegToRead[i]==4)
      {
        uint64_t value=0;
        for(byte index=0;index<4;index++)
          value=value + ((uint64_t)node.getResponseBuffer(index) << (48-16*index));
        
        meterData[vPins[j]]=value;
      }

      if(noRegToRead[i]==2)
      {
        Serial.printf("val of i: %d,val of j: %d \n",i,j);
      // Serial.print("metermemory usage: ");Serial.println(metersArray.memoryUsage());

        //schneider electric data conversion
        // uint32_t intVal = ((uint32_t)node.getResponseBuffer(0) << 16) + node.getResponseBuffer(1);
        // memcpy(&floatValue, &intVal, sizeof(floatValue));

        // elmeasure data conversion and selec
        data[0]=node.getResponseBuffer(0x00);
        data[1]=node.getResponseBuffer(0x01);
        floatValue=*((float *)data);

        // if(regAddr[i]==3059)
        //   floatValue=floatValue*1000;

        // if(regAddr[i]==3077 || regAddr[i]==3079  || regAddr[i]==3079)
        //   floatValue=powerfactorcalculation(floatValue);

        //selec reg conversion as per application 
        if((regAddr[i]==0x60) || (regAddr[i]==0X2A) || (regAddr[i]==0X18) || (regAddr[i]==0X1A) || (regAddr[i]==0X1C) || (regAddr[i]==0X54) ||(regAddr[i]==0X56) ||(regAddr[i]==0X58) )
        {
          floatValue=floatValue*1000;
        }
        
        Serial.print("noParam : ");Serial.println(noParam[j]);
        if(noParam[j]==3)
        {

          if(cnt==true)
          {
            cnt=false;
            phaseQty=meterData[vPins[j]].to<JsonObject>();
          }
          //Serial.printf("count : %d\n",count);
          phaseQty[values[count++]]=String(floatValue,2);
          
          if(count < sizeof(values)/sizeof(values[0])) //fix the != to <
          {
            // Serial.printf("count : %d\n",count);
            // Serial.printf("continue\n");
            continue;
          }
          else
          {
            Serial.printf("count is 0 and cnt is true\n");
            count=0;
            cnt=true;
          }
        }

        else
        {
          Serial.print("vpins : ");Serial.println(vPins[j]);
          meterData[vPins[j]]=String(floatValue,2);
        }
      }
      j++;
    }
   
  }


  String jsonString;
  serializeJson(docJson1,jsonString);
  //Serial.print("On the whole memory usage: ");Serial.println(docJson1.memoryUsage());
  Serial.println("json String : " + jsonString);
  Serial.println();
  dBot.sentEnergyMeterData(jsonString);
  // Serial.print("Total measurejson");Serial.println(measureJson(docJson1));
  // Serial.print("On the whole memory usage: ");Serial.println(docJson1.memoryUsage());
  docJson1.clear();
  //Serial.printf("readModbusJson End\n");
  Serial.println();
} 


void loop()
{
  byte i=0;
  Serial.printf("main loop\n\n");
  dBot.Loop();
 
  if(millis()>=5000)
  {
    currentmillis=millis();
    Serial.print("currentmillis-previousmillis : ");Serial.println((currentmillis-previousmillis)/1000);
    // if(currentmillis-previousmillis>=(1000*dBot.completeJsonReading))
    if(currentmillis-previousmillis>=(1000*10))
    {
      uint64_t premillis=millis();
      readModbusJson(dBot.numSlave);
      //readModbusJson(13);
     // uint64_t postmillis=millis();
      //Serial.print("time for data acquire from multiple slaves : ");Serial.println((postmillis-premillis)/1000);
      previousmillis=currentmillis+(millis()-premillis);
      //Serial.print("difference of millis-currentmillis : ");Serial.println(previousmillis/1000);
    }

  }
  else
  {
    Serial.printf("config a stable connection\n");
  }
}
