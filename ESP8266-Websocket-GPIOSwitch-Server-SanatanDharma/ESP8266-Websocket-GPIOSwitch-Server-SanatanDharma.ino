/*
 * ESP8266 Web server with Web Socket to control an LED.
 *
 * The web server keeps all clients' LED status up to date and any client may
 * turn the LED on or off.
 *
 * For example, clientA connects and turns the LED on. This changes the word
 * "LED" on the web page to the color red. When clientB connects, the word
 * "LED" will be red since the server knows the LED is on.  When clientB turns
 * the LED off, the word LED changes color to black on clientA and clientB web
 * pages.
 *
 * References:
 *
 * https://github.com/Links2004/arduinoWebSockets
 * 
 * nodeUSB Flash Config
 * 15:56:33.377 -> Flash ide  size: 4194304 bytes
15:56:33.378 -> Flash ide speed: 40000000 Hz
15:56:33.380 -> Flash ide mode:  DIO
15:56:33.381 -> Flash Chip configuration ok.
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266WebServer.h>
#include <string.h>
#include <ArduinoOTA.h>
#define MAX_STRING_LEN  32

// Function to return a substring defined by a delimiter at an index
char* subStr (char* str, char *delim, int index) {
  char *act, *sub, *ptr;
  static char copy[MAX_STRING_LEN];
  int i;

  // Since strtok consumes the first arg, make a copy
  strcpy(copy, str);

  for (i = 1, act = copy; i <= index; i++, act = NULL) {
     //Serial.print(".");
     sub = strtok_r(act, delim, &ptr);
     if (sub == NULL) break;
  }
  return sub;

}

const char *WIFI_SSID = "****";
const char *WIFI_PASS = "****";
//Strange nodemcu numbering
int GPIO1 = 5;    
int GPIO2 = 4; 
int GPIO5 = 14;
int GPIO6 = 12;
//HOLDS the GPIO states   
char str[9] = {'C','H','K','A','-'}; //CHKA-1010
//Holds the integer values of GPIO reads
int val1=0;
int val2=0;
int val5=0;
int val6=0;

ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(8000);

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=2.7">

  <meta  content="text/html; charset=utf-8">
<style>
  * {
    box-sizing: border-box;
}
  
  [class*="col-"] {
    float: left;
    padding: 15px;
}
/* For mobile phones: */
[class*="col-"] {
    width: 100%;
}
@media only screen and (min-width: 1024px) {
    /* For desktop: */
    .col-1 {width: 8.33%;}
    .col-2 {width: 16.66%;}
    .col-3 {width: 25%;}
    .col-4 {width: 33.33%;}
    .col-5 {width: 41.66%;}
    .col-6 {width: 50%;}
    .col-7 {width: 58.33%;}
    .col-8 {width: 66.66%;}
    .col-9 {width: 75%;}
    .col-10 {width: 83.33%;}
    .col-11 {width: 91.66%;}
    .col-12 {width: 100%;}
}
  .switch {
  position: relative;
  display: inline-block;
  width: 60px;
  height: 34px;
}
.svrlight{
  background-color: red;
  }
.switch input {display:none;}
.pwrlight{
  position: relative;
  height: 26px;
  width: 26px;
  right: 20px;
  bottom: 4px;
  border-radius: 10%;
  }
.pwrlight:active{
  background-color: red;
  border: none;
  }
.slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: #ccc;
  -webkit-transition: .4s;
  transition: .4s;
}

.slider:before {
  position: absolute;
  content: "";
  height: 26px;
  width: 26px;
  left: 4px;
  bottom: 4px;
  background-color: white;
  -webkit-transition: .4s;
  transition: .4s;
}

input:checked + .slider {
  background-color: #2196F3;
}

input:focus + .slider {
  box-shadow: 0 0 1px #2196F3;
}

input:checked + .slider:before {
  -webkit-transform: translateX(26px);
  -ms-transform: translateX(26px);
  transform: translateX(26px);
}

/* Rounded sliders */
.slider.round {
  border-radius: 34px;
}

.slider.round:before {
  border-radius: 50%;
}
.buttonpressedeffect {
  display: inline-block;
  padding: 15px 25px;
  font-size: 24px;
  cursor: pointer;
  text-align: center;
  text-decoration: none;
  outline: none;
  color: #fff;
  background-color: #4CAF50;
  border: none;
  border-radius: 15px;
  box-shadow: 0 9px #999;
}

.buttonpressedeffect:hover {background-color: #3e8e41}

.buttonpressedeffect:active {
  background-color: #3e8e41;
  box-shadow: 0 5px #666;
  transform: translateX(4px);
 }
 
 .buttonhover {
  display: inline-block;
  border-radius: 4px;
  background-color: #f4511e;
  border: none;
  color: #FFFFFF;
  text-align: center;
  font-size: 28px;
  padding: 20px;
  width: 200px;
  transition: all 0.5s;
  cursor: pointer;
  margin: 5px;
}

.buttonhover span {
  cursor: pointer;
  display: inline-block;
  position: relative;
  transition: 0.5s;
}

.button span:after {
  content: '\00bb';
  position: absolute;
  opacity: 0;
  top: 0;
  right: -20px;
  transition: 0.5s;
}

.buttonhover:hover span {
  padding-right: 25px;
}

.button:hover span:after {
  opacity: 1;
  right: 0;
}

</style>
<script language="javascript" type="text/javascript">

 var boolConnected=false;
  function doConnect()
  {
      if (!(boolConnected)){
      /*websocket = new WebSocket(document.myform.url.value);*/
/*
         websocket = new WebSocket('ws://192.168.1.106:8000/');
*/

         
    websocket = new WebSocket('ws://' + window.location.hostname + ':8000/'); 
    boolConnected=true;
    websocket.onopen = function(evt) { onOpen(evt) };
    websocket.onclose = function(evt) { onClose(evt) };
    websocket.onmessage = function(evt) { onMessage(evt) };
    websocket.onerror = function(evt) { onError(evt) };
  }

  }
  function onOpen(evt)
  {
    console.log("connected\n");
      /* if response from server - change the color to green*/
      document.getElementById("svrlight").style.backgroundColor="LawnGreen";
      /* Server is online - let's check the gpio status*/
      doSend("CHKA-X");
  }

  function onClose(evt)
  {
    console.log("disconnected\n");
      /* if server disconnected - change the color to red*/
      document.getElementById("svrlight").style.backgroundColor="red";
      /*set the gpio array to blank*/
      window['gpio']=[];
      boolConnected=false;
  }

  function onMessage(evt)
  {

      
     /* Data returned from ESP will be in the form of ON-1 OFF-1 ON-2 ON-5 OFF-2 OFF-5 CHKA-1010
      CHKA-1010 4 digits correspond to gpio reads of pin 1,2,5,6
    evt.data.slice(-1) = "1" or "2" or "5" or "6"
    evt.data.slice(0,-2)="ON" or "OFF" 
      evt.data.slice(0,-5)="CHKA" 
      
      
      
      */
      
      
    console.log("response: " + evt.data + '\n');

    var status=evt.data.slice(0,-2);
    var statuschk=evt.data.slice(0,-5); /*"CHKA-1010" becomes "CHKA" */
    if(status=="ON"){
            var id=evt.data.slice(-1);
        document.getElementById("pwrlight"+id).style.backgroundColor="red";
        document.getElementById("sw"+id).checked=true;
        } 
        else if (status=="OFF"){
            var id=evt.data.slice(-1);
            document.getElementById("pwrlight"+id).style.backgroundColor="black";
            document.getElementById("sw"+id).checked=false;
        } 
    else if (statuschk=="CHKA"){
      /* store gpio staus in an array named gpio+i*/
      window['gpio']=[];
      window['gpio'][1]=evt.data.slice(-4,-3); /*"CHKA-1010" becomes "1" */
      window['gpio'][2]=evt.data.slice(-3,-2); /*"CHKA-1010" becomes "0" */
      window['gpio'][5]=evt.data.slice(-2,-1); /*"CHKA-1010" becomes "1" */
      window['gpio'][6]=evt.data.slice(-1); /*"CHKA-1010" becomes "0" */
      /*call the set initial switch state function*/
      initialswitchstate();
      
    }

    
    
  
  }

  function onError(evt)
  {

  console.log('error: ' + evt.data + '\n');
  websocket.close();


  }

  function doSend(message)
  {
    console.log("sent: " + message + '\n');

    websocket.send(message);
  }

function closeWS(){
    
   /* Probable bug in arduino websocket - hangs if not closed properly, specially by a phone browser entering a powersaving mode*/
    websocket.close();
    boolConnected=false;
}

/*    On android - when page loads - focus event isn't fired so websocket doesn't connect*/
   
   window.addEventListener("focus",doConnect, false);
   
    
 window.addEventListener("blur",closeWS, false);
 window.addEventListener('load', function() {
    foo(true); 
     /*After page loading blur doesn't fire until focus has fired at least once*/
     
    /* window.focus();*/
},{once:true}, false);

/*window.addEventListener('blur', function() {
    foo(false);
}, {once:true}, false); */


function foo(bool) {
    if (bool){
doConnect();
    } else {
        
      
 /*   Probable bug in arduino websocket - hangs if not closed properly, specially by a phone browser entering a powersaving mode
 */       websocket.close();    
    }
}
    
</script>
<script type="text/javascript">

function initialswitchstate(){
  
  var allswitches=document.getElementsByTagName("input"); /* get all elements by tag name input*/
    for(k=0;k<allswitches.length;k++){

    
      if(window['gpio'][allswitches[k].id.slice(-1)] == 1){
        /*ON*/
        document.getElementById("pwrlight"+allswitches[k].id.slice(-1)).style.backgroundColor="red";
        allswitches[k].checked=true;
        } else {
          /*OFF*/
          document.getElementById("pwrlight"+allswitches[k].id.slice(-1)).style.backgroundColor="black";
          allswitches[k].checked=false;
          
          };
      
    
    };

  
  
  };
function queryServer1(id)
{
 /* 1,2,5,6 (for gpio 5,4,14,12) - works. 0 (GPIO 16 doesn't work).*/
 /*sw1 pwrlight1 sw2 pwrlight2 sw5 pwrlight5 sw6 pwrlight6  */
  var payload;
  if (document.getElementById("sw"+id).checked==true){
        payload="ON-"+id;}
        else {
        
        payload="OFF-"+id;
        };
 
  doSend(payload);
 
};
</script>
<title>BedRoom</title></head>
<body>


<div class="col-6">    

  <button class="pwrlight" id = "pwrlight1"></button>
<label class="switch">
  <input type="checkbox" id="sw1" onclick='queryServer1("1")'>
  <div class="slider round"></div>
</label><br>
<button class="pwrlight" id = "pwrlight2"></button>
<label class="switch">
  <input type="checkbox" id="sw2" onclick='queryServer1("2")'>
  <div class="slider round"></div>
</label><br>




  <button class="pwrlight" id = "pwrlight5"></button>
<label class="switch">
  <input type="checkbox" id="sw5" onclick='queryServer1("5")'>
  <div class="slider round"></div>
</label><br>
<button class="pwrlight" id = "pwrlight6"></button>
<label class="switch">
  <input type="checkbox" id="sw6" onclick='queryServer1("6")'>
  <div class="slider round"></div>
</label><br>



    <button class="svrlight" id = "svrlight" >BedRoom</button>
    </div>
</body>

</html>

)rawliteral";





void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      //itoa( cm, str, 10 );
      //  webSocket.sendTXT(num, str, strlen(str));
      }
      break;
    case WStype_TEXT:
     {
      Serial.printf("[%u] get Text: %s\r\n", num, payload);

      char *split1,*split2 ;
     char *mystring = (char *)payload;
      split1=subStr(mystring, "-", 1);
      split2=subStr(mystring, "-", 2);
      
      if (strcmp(split1,"CHKA") == 0)
      {//Send GPIO statuses to all the clients
        //char str[9] = {'C','H','K','A','-'}; //CHKA-1010 -- Declared above
          val1=digitalRead(GPIO1)+48;// 48 is the ascii code for '0' - basically converting integer 0 into ASCII 0 
          val2=digitalRead(GPIO2)+48;
          val5=digitalRead(GPIO5)+48;
          val6=digitalRead(GPIO6)+48;
          str[5]=(char) val1;
          str[6]=(char) val2;
          str[7]=(char) val5;
          str[8]=(char) val6;
          webSocket.broadcastTXT(str, strlen(str));
      }
      else if (strcmp(split1,"ON") == 0) 
      {//ON-1 ON-2 ON-5 ON-6
       //Turn on the GPIO and then broadcast to all the clients
           if (strcmp(split2,"1") == 0){digitalWrite(GPIO1, HIGH);}
           if (strcmp(split2,"2") == 0){digitalWrite(GPIO2, HIGH);}
           if (strcmp(split2,"5") == 0){digitalWrite(GPIO5, HIGH);}
           if (strcmp(split2,"6") == 0){digitalWrite(GPIO6, HIGH);}
           webSocket.broadcastTXT(payload, length);
      } 
      else if (strcmp(split1,"OFF") == 0)
      {//OFF-1 OFF-2 OFF-5 OFF-6
       //Turn off the GPIO and then broadcast to all the clients
           if (strcmp(split2,"1") == 0){digitalWrite(GPIO1, LOW);}
           if (strcmp(split2,"2") == 0){digitalWrite(GPIO2, LOW);}
           if (strcmp(split2,"5") == 0){digitalWrite(GPIO5, LOW);}
           if (strcmp(split2,"6") == 0){digitalWrite(GPIO6, LOW);}
          webSocket.broadcastTXT(payload, length);
       }

     }
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      hexdump(payload, length);

      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}

void handleRoot()
{
  server.send_P(200, "text/html", INDEX_HTML);
}
void handleON(){
  digitalWrite(GPIO2, HIGH);
  webSocket.broadcastTXT("ON-2", 4);
  server.send(200);
  
  }
void handleOFF(){
  digitalWrite(GPIO2, LOW);
  webSocket.broadcastTXT("OFF-2", 5);
  server.send(200);
  
  }
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void connect() {

  // Connect to Wifi.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

//  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // WiFi fix: https://github.com/esp8266/Arduino/issues/2186
 // WiFi.persistent(false);
 // WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.config(IPAddress(192,168,1,205), IPAddress(192,168,1,2), IPAddress(255,255,255,0),IPAddress(192,168,1,2));
  unsigned long wifiConnectStart = millis();

  while (WiFi.status() != WL_CONNECTED) {
    // Check to see if
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Failed to connect to WiFi. Please verify credentials: ");
      delay(10000);
    }

    delay(500);
    Serial.println("...");
    // Only try for 5 seconds.
    if (millis() - wifiConnectStart > 15000) {
      Serial.println("Failed to connect to WiFi");
      return;
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
 // Serial.println(millis()-currentmillis); //=506
  
  Serial.println();

  // Serial.println(millis()-currentmillis); //=1434
   
}

void setup()
{ 


  Serial.begin(115200);
  delay(10);
 // set all the GPIOs to OUTPUT
  pinMode(GPIO1, OUTPUT);
  pinMode(GPIO2, OUTPUT);
  pinMode(GPIO5, OUTPUT);
  pinMode(GPIO6, OUTPUT);
//set all the GPIOs to LOW

  digitalWrite(GPIO1, LOW);
  digitalWrite(GPIO2, LOW);
  digitalWrite(GPIO5, LOW);
  digitalWrite(GPIO6, LOW);
  

  Serial.println();
  Serial.println();
  Serial.println();

 connect();
  
  server.on("/", handleRoot);
  server.on("/ON", handleON);
  server.on("/OFF", handleOFF);
  server.onNotFound(handleNotFound);

  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);


/* ************OTA********************* */

// Port defaults to 8266
// ArduinoOTA.setPort(8266);

// Hostname defaults to esp8266-[ChipID]
// ArduinoOTA.setHostname("myesp8266");

// No authentication by default
// ArduinoOTA.setPassword((const char *)"123");

ArduinoOTA.onStart([]() {
Serial1.println("Start");
});
ArduinoOTA.onEnd([]() {
Serial1.println("\nEnd");
});
ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
Serial1.printf("Progress: %u%%\r", (progress / (total / 100)));
});
ArduinoOTA.onError([](ota_error_t error) {
Serial1.printf("Error[%u]: ", error);
if (error == OTA_AUTH_ERROR) Serial1.println("Auth Failed");
else if (error == OTA_BEGIN_ERROR) Serial1.println("Begin Failed");
else if (error == OTA_CONNECT_ERROR) Serial1.println("Connect Failed");
else if (error == OTA_RECEIVE_ERROR) Serial1.println("Receive Failed");
else if (error == OTA_END_ERROR) Serial1.println("End Failed");
});
ArduinoOTA.begin();

/****************************************************/  


}

void loop()
{
  webSocket.loop();
  server.handleClient();
  ArduinoOTA.handle();

}
