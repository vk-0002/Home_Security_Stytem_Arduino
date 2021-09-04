
#include <EEPROM.h>
#include <ESP8266WiFi.h>
//#include <ESP8266WebServer.h>
#include<ArduinoJson.h>
#include<ESPAsyncTCP.h>
#include<ESPAsyncWebServer.h>

#define HOTSPOT_NAME 0
#define HOTSPOT_PASS 20
#define WIFI_NAME 40
#define WIFI_PASS 60
#define PIN 80
#define USERS 90

//ESP8266WebServer server(80); 

AsyncWebServer server(80);

int deviceStatus=0;

// This function store data in EEPROM(permenant memory )
void set(int address,String data)
{
  int i;
  
  for(i=0 ; i<19 && i<data.length() ; i++)
  {
      EEPROM.write(address+i , data[i]);    //this line write data
  }
  EEPROM.write(address+i , '\0');
  EEPROM.commit();                          //this line save data
  Serial.println("data store "+ data);
}


String getData(int address)
{
  int i;
  String data;
  for(i=0 ; i<20 ; i++)
  {
      char ch =char( EEPROM.read(address+i) );
      if(ch=='\0')
      {
          break;
      }
      data=data+ch;
  }

  Serial.println("data is "+data);
  return data;
}

void hotspotConfiguration()
{
  String hotspotName=getData(HOTSPOT_NAME);
  String hotspotPass=getData(HOTSPOT_PASS);
  
  Serial.print("Configuring access point...");
  WiFi.softAP(hotspotName, hotspotPass);              //Turn on HotSpot
  IPAddress myIP = WiFi.softAPIP();               //get IP addres
  Serial.print("AP IP address: ");
  Serial.println(myIP);
      
}

void wifiConfiguration()
{

  String sta_ssid = getData(WIFI_NAME);
  String sta_password=getData(WIFI_PASS);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(sta_ssid);
  WiFi.begin(sta_ssid, sta_password);     //Conneting to home wifi

  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Invalid password or wifi name"); 
  }
  
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());         //Print IP addres

}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);

  WiFi.mode(WIFI_AP_STA);                         //set ESP on WiFi & HotSpot Mode
  hotspotConfiguration();
  wifiConfiguration();

//  server.on("/", HTTP_POST, handleRoot); 
//  server.on("/deviceInfo",HTTP_GET,deviceInfo);
//  server.on("/addNewUser", HTTP_POST , addNewUser);
//  server.on("/deviceSettings",HTTP_POST,deviceSettings);
//  server.on("/changePin",HTTP_POST,changePin);
//  
//  server.onNotFound(handleNotFound);


  server.on("/",HTTP_GET,[](AsyncWebServerRequest *request)
  {
 
    Serial.println("not ok");
   request->send(200,"text/plane","not get");
  
  });

  server.on("/deviceInfo",HTTP_GET,[]( AsyncWebServerRequest *request ){

    String macAddress=WiFi.macAddress();
    String hotSpotIP= IPtoString( WiFi.softAPIP() );
    String wifiIP=IPtoString( WiFi.localIP() );
    String status=(String) deviceStatus;
    
    String info="{\"deviceStatus\" : \""+ status + "\" ,\"deviceID\" : \""+ macAddress + "\" ,\"deviceHotspotIP\" : \""+ hotSpotIP + "\", \"deviceWiFiIP\" : \"" + wifiIP +"\" , \"deviceType : doreLock\"  }" ;
    Serial.println(info);

    request->send(200,"text/plane", info );  
      
  });

  server.on("/addNewDevice",HTTP_POST,[](AsyncWebServerRequest *request)
  {
   int args =request->args();
   for(int i=0;i<args;i++)
   {
     Serial.printf("ARG[%s]: %s\n",request->argName(i).c_str(),request->arg(i).c_str());  
   }

   Serial.println();

   int i=0;
   DynamicJsonDocument data(1024);
   deserializeJson(data,request->arg(i).c_str());

   String uid=data["uid"];
   Serial.println(uid);

   String deviceId=data["deviceId"];
   Serial.println(deviceId);

   String pin=data["pin"];
   Serial.println(pin);
  
  
   if(uid!=NULL && deviceId!=NULL && pin!=NULL)
   {
      if(deviceId==WiFi.macAddress())
      {
        if(pin==getData(PIN))
        {
          request->send(200,"text/plane","device linked"); 
          Serial.println("ok");
        }
        else
        {
          request->send(200,"text/plane","yedyabhokacya"); 
          Serial.println("yedyabhokacya"); 
        }
      }
      else
      {
        request->send(200,"text/plane","lavdya");
        Serial.println("Device wrong");  
      }  
   }
   else
   {
     request->send(200,"text/plane","MotherFucker");
     Serial.println("argument null");
   }
  
  
 });


 server.on("/deviceSettings",HTTP_POST,[](AsyncWebServerRequest *request)
 {
  
    int args=request->args();
    for(int i=0;i<args;i++)
    {
       Serial.println(request->arg(i).c_str());  
    } 

    int i=0;
    DynamicJsonDocument data(1024);
    deserializeJson(data,request->arg(i).c_str());

    
    
    if(data["ssid"]!=NULL)
    {
      String ssid=data["ssid"];
      set(WIFI_NAME,ssid);
      Serial.println("ssid changed"); 
    }
    
    if(data["ssid_password"]!=NULL)
    {
      String ssid_pass=data["ssid_password"];
      set(WIFI_PASS,ssid_pass);
      Serial.println("ssid pass changed");  
    }

    if(data["ap_name"]!=NULL)
    {
      String ap=data["ap_name"]; 
      set(HOTSPOT_NAME,ap);
      Serial.println("ap changed");  
    }

    if(data["ap_password"]!=NULL)
    {
      String ap_pass=data["ap_password"]; 
      set(HOTSPOT_PASS,ap_pass); 
      Serial.println("ap pass changed"); 
    }


    
    Serial.println("restart wifi settings");
     
    hotspotConfiguration();
    wifiConfiguration();
   
  });


  server.on("/deviceOnOff",HTTP_POST,[](AsyncWebServerRequest *request)
  {
  
      
      int args=request->args();
      for(int i=0;i<args;i++)
      {
         Serial.println(request->arg(i).c_str());  
      } 
  
      int i=0;
      DynamicJsonDocument data(1024);
      deserializeJson(data,request->arg(i).c_str());
  
      String deviceId=data["deviceId"];
      Serial.println(deviceId);
  
      String pin=data["pin"];
      Serial.println(pin);

      
      bool flag=data["flag"];
      Serial.println(flag);

      if(deviceId!=NULL && pin!=NULL && flag!=NULL)
      {
        if(deviceId==WiFi.macAddress())
        {
          if(pin==getData(PIN))
          {

            if(flag==true)
            {
                unlockDevice();
            }
            else
            {
                lockDevice();
            }
            
            request->send(200,"text/plane","device linked"); 
            Serial.println("ok");
          }
          else
          {
            request->send(200,"text/plane","yedyabhokacya"); 
            Serial.println("yedyabhokacya"); 
          }
        }
        else
        {
          request->send(200,"text/plane","lavdya");
          Serial.println("Device wrong");  
        }   
    }
    else
    {
       request->send(200,"text/plane","MotherFucker");
       Serial.println("argument null");
    }  

  }); 


  

  server.begin();


}



String IPtoString(IPAddress address)
{
    return String() + address[0] + "." + address[1] + "." + address[2] + "." + address[3];  
}

bool unlockDevice()
{
    // unlock device logic

    return true;
}
bool lockDevice()
{
    // lock device logic

    return true;
}

//void deviceSettings()
//{
//  if(server.hasArg("ssid") && server.hasArg("ssid_password") && server.hasArg("ap_name") && server.hasArg("ap_password") )
//  {
//    if(server.arg("ssid")!=NULL)
//    {
//      set(WIFI_NAME,server.arg("ssid")); 
//    }
//    
//    if(server.arg("ssid_password")!=NULL)
//    {
//      set(WIFI_PASS,server.arg("ssid_password"));  
//    }
//
//    if(server.arg("ap_name")!=NULL)
//    {
//      set(HOTSPOT_NAME,server.arg("ap_name"));  
//    }
//
//    if(server.arg("ap_password")!=NULL)
//    {
//      set(HOTSPOT_PASS,server.arg("ap_password")); 
//    }
//
//    hotspotConfiguration();
//    wifiConfiguration();
//    Serial.println("restart wifi settings");
//    
//  }
//  else
//  {
//    server.send(200,"text/plane","MotherFucker"); 
//    Serial.println("invalid request"); 
//  }  
//}
//
//void changePin()
//{
//  if(server.hasArg("pin"))
//  {
//    if(server.arg("pin")==getData(PIN))
//    {
//      set(PIN,server.arg("pin"));
//    }
//    else
//    {
//       server.send(200,"text/plane","lavdya"); 
//       Serial.println("invalid pin");   
//    }
//  }
//  else
//  {
//    server.send(200,"text/plane","MotherFucker"); 
//    Serial.println("invalid request");   
//  }  
//}
//
//void handleRoot()
//{
//  //server.send(200,"text/html"," <form action=\"/addNewUser\" method=\"post\"><label for=\"fname\">First name:</label><input type=\"text\" id=\"uid\" name=\"uid\"><br><br>label for=\"lname\">Last name:</label><input type=\"text\" id=\"deviceId\" name=\"deviceId\"><br><br><input type=\"submit\" value=\"Submit\"></form> ");  
//
//   Serial.println(server.hasArg("1"));
//   server.send(200,"text/plane","okk");
//
//}
//
//void handleNotFound()
//{
//   server.send(404, "text/plain", "404: Not found"); 
//}
//
void loop() 
{
  //server.handleClient();
}
