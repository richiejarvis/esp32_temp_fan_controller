// esp32_temp_fan_controller - Richie Jarvis - richie@helkit.com
// Version: v0.0.1 - 2025-02-24
// Version: v0.0.2 - 2025-02-26
// Github: https://github.com/richiejarvis/
// Version History
// v0.0.1 - Initial Release
// v0.0.2 - Adding WiFi and Web Control

// Check the temperature, and if above the set-point (25 degrees), start the 12v fans via the relay

// A useful alternative to the Pin 12 to GND reset
#define CONFIG_VERSION "018"
#define CONFIG_VERSION_NAME "v0.0.2"

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <Wire.h>
// #include <WebServer.h>
#include "time.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>


using namespace std;

// Setup the Adafruit library
Adafruit_BME280 bme;
Adafruit_Sensor *bme_temp = bme.getTemperatureSensor();


// Transient Variables
// Var to store Current Fan Status 
boolean current_Fan_State = false;
// Var to store the number of seconds to override for from the Web client.
int override_Cancel_Period = 1800;
int override_Counter = 0;


// Here are my Variables
// Constants
// Replace with your network credentials
const char* wifi_ssid = "PapaSmurf";
const char* wifi_passcode = "797ph0NE*$52699!";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  %PLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";


// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "PLACEHOLDER")
  {
  // Is Override Active?
    if (override_Counter > 0) 
    {
      override_Active = "ON";
    } 
    else 
    {
      override_Active = "OFF";
    }


    String html = "";
    html += "<h2>Settings</h2>"
    html += "<p>Fans automatically turn on at " +(String)on_Temp) + " Degrees Centigrade";
    html += "<p>Fans automatically turn on at " +(String)off_Temp) + " Degrees Centigrade";
    html += "<h2>Current Status</h2>"
    html += "<p>Current Temperature: " + (String)temperature + "/<p>";
    html += "<p>Override Status: " + override_Active;
    html += " Override Counter: " +(String)override_Counter + "/<p>";
    html += "<p>Fan State: " + (String)current_Fan_State + "</p>";
    html += "<h4>Override</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"33\" " + outputState(33) + "><span class=\"slider\"></span></label>";
    html += ""
  );
}

   return html;
  }
  return String();
}

String outputState(int output){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
}

// const char index_html[] PROGMEM = R"rawliteral(
// <!DOCTYPE HTML><html>
// <head>
//   <title>ESP Fan Controller</title>
//   <meta name="viewport" content="width=device-width, initial-scale=1">
//   <link rel="icon" href="data:,">
//   <style>
//   html {
//     font-family: Arial, Helvetica, sans-serif;
//     text-align: center;
//   }
//   h1 {
//     font-size: 1.8rem;
//     color: white;
//   }
//   h2{
//     font-size: 1.5rem;
//     font-weight: bold;
//     color: #143642;
//   }
//   .topnav {
//     overflow: hidden;
//     background-color: #143642;
//   }
//   body {
//     margin: 0;
//   }
//   .content {
//     padding: 30px;
//     max-width: 600px;
//     margin: 0 auto;
//   }
//   .card {
//     background-color: #F8F7F9;;
//     box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
//     padding-top:10px;
//     padding-bottom:20px;
//   }
//   .button {
//     padding: 15px 50px;
//     font-size: 24px;
//     text-align: center;
//     outline: none;
//     color: #fff;
//     background-color: #0f8b8d;
//     border: none;
//     border-radius: 5px;
//     -webkit-touch-callout: none;
//     -webkit-user-select: none;
//     -khtml-user-select: none;
//     -moz-user-select: none;
//     -ms-user-select: none;
//     user-select: none;
//     -webkit-tap-highlight-color: rgba(0,0,0,0);
//    }
//    /*.button:hover {background-color: #0f8b8d}*/
//    .button:active {
//      background-color: #0f8b8d;
//      box-shadow: 2 2px #CDCDCD;
//      transform: translateY(2px);
//    }
//    .state {
//      font-size: 1.5rem;
//      color:#8c8c8c;
//      font-weight: bold;
//    }
//   </style>
// <title>ESP Web Server</title>
// <meta name="viewport" content="width=device-width, initial-scale=1">
// <link rel="icon" href="data:,">
// </head>
// <body>
//   <div class="topnav">
//     <h1>ESP WebSocket Server</h1>
//   </div>
//   <div class="content">
//     <div class="card">

//       ws.textAll(String(current_Fan_State), String(temperature), String(override_Counter), String(override_Active), String(on_Temp), String(off_Temp));

//       <h2>Current Status</h2>

//       <p class="state">Fan State: <span id="state">%current_Fan_State%</span></p>
//       <p class="state">Temperature: <span id="state">%temperature%</span></p>      
//       <p class="state">Override Active: <span id="state">%override_Active%</span></p>
//       <p class="state">On Temperature: <span id="state">%on_Temp%</span></p>
//       <p class="state">Off Temperature: <span id="state">%off_Temp%</span></p>
//       <p class="state">Override Seconds Remaining: <span id="state">%override_Active%</span></p>

//       <p><button id="button" class="button">Toggle Override</button></p>
//     </div>
//   </div>
// <script>
//   var gateway = `ws://${window.location.hostname}/ws`;
//   var websocket;
//   window.addEventListener('load', onLoad);
//   function initWebSocket() {
//     console.log('Trying to open a WebSocket connection...');
//     websocket = new WebSocket(gateway);
//     websocket.onopen    = onOpen;
//     websocket.onclose   = onClose;
//     websocket.onmessage = onMessage; // <-- add this line
//   }
//   function onOpen(event) {
//     console.log('Connection opened');
//   }
//   function onClose(event) {
//     console.log('Connection closed');
//     setTimeout(initWebSocket, 2000);
//   }
//   function onMessage(event) {
//     var state;
//     if (event.data == "1"){
//       state = "ON";
//     }
//     else{
//       state = "OFF";
//     }
//     document.getElementById('state').innerHTML = state;
//   }
//   function onLoad(event) {
//     initWebSocket();
//     initButton();
//   }
//   function initButton() {
//     document.getElementById('button').addEventListener('click', toggle);
//   }
//   function toggle(){
//     websocket.send('toggle');
//   }
// </script>
// </body>
// </html>
// )rawliteral";

// // Set listening port to 80
// WiFiServer server(80);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// // Variable to store the HTTP request
// String header;

// Variable to store the temperature
int temperature = 0;
int prev_Temperature = 0;
int on_Temp = 26;
int off_Temp = 20;
boolean prev_State = false;

// Relay board control pins
const int relay1Pin = 16;  // Controls Relay 1 - Left Hand relay when relays are nearest to me
const int relay2Pin = 17;  // Not currently used

// Simple Logging Output...
void debugOutput(String textToSend)
{
  Serial.println();
  Serial.println(textToSend);
}

// Set Fan Mode On
boolean startFans()
{
    // Turn Relay on!
    digitalWrite(relay1Pin, HIGH);
    prev_State = current_Fan_State;
    return true;
}

// Set Fan Mode Off
boolean stopFans()
{
    // Turn Relay off!
    digitalWrite(relay1Pin, LOW);
    prev_State = current_Fan_State;
    return false;
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) {
      current_Fan_State = !current_Fan_State;
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%lu connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%lu disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (current_Fan_State){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  return String();
}

// // Update any connected client
// void notifyClients() {
//   String override_Active = "";
//   // Is Override Active?
//   if (override_Counter > 0) 
//   {
//     override_Active = "ON";
//   } 
//   else 
//   {
//     override_Active = "OFF";
//   }
//   ws.textAll(
//     String
//     (
//       (String)current_Fan_State)
//     // ), 
//     // String(
//     //   (String)temperature
//     // ), 
//     // String(
//     //   (String)override_Counter
//     // ),
//     // String(
//     //   (String)override_Active),
//     // String((String)on_Temp),
//     // String((String)off_Temp)
//   );
//   debugOutput("INFO: Letting Clients Know");
// }


// Setup everything...
void setup() 
{
  Serial.begin(115200);
  debugOutput("INFO: Starting esp32_temp_fan_controller v" + (String)CONFIG_VERSION_NAME);
  /* Initialise the sensor */
  // This part tries I2C address 0x77 first, and then falls back to using 0x76.
  // If there is no I2C data with these addresses on the bus, then it reports a Fatal error, and stops
  if (!bme.begin(0x77, &Wire)) {
    debugOutput("INFO: BME280 not using 0x77 I2C address - checking 0x76");
    if (!bme.begin(0x76, &Wire)) {
      debugOutput("FATAL: Could not find a valid BME280 sensor, check wiring!");
      while (1) delay(10);
    }
  }
  pinMode(relay1Pin,OUTPUT);
  pinMode(relay2Pin,OUTPUT);
  debugOutput("INFO: BME280 & Relay Initialisation completed");
    // Connect to Wi-Fi network with SSID and password
  debugOutput("INFO: WiFI Connecting");
  WiFi.begin(wifi_ssid, wifi_passcode);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  debugOutput("INFO: WiFI Connected");
  // Print local IP address and start web server
  debugOutput("INFO: IP Address: " + WiFi.localIP());
  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Start server
  server.begin();
  debugOutput("INFO: Webserver Started");
  debugOutput("INFO: Initialisation Completed");
}

//  This is where we do stuff again and again...
void loop() 
{
  // Check current temp & if the sensor is working
  if (checkFanStatus()) 
  {
    // // Webserver Listening
    // webserverListen();
    // Count down 1 second if override is active
    if (override_Counter > 0){
      // Override is active
      override_Counter--;
    } else if (override_Counter = 0) {
      override_Counter = -1;
      debugOutput("INFO: Leaving Override Mode");  
    } else {
      // Cancel Override
      override_Counter = 0;
    }
  } 
  delay(1000);
  // ws.cleanupClients();
}

// void webserverListen() 
// {
//   WiFiClient client = server.accept(); // Listen for incoming clients
//   if (client) { 
//     // If a new client connects,
//     debugOutput("INFO: Client Connected");
//     String currentLine = ""; // make a String to hold incoming data from the client
//     while (client.connected()) 
//     { // loop while the client's connected
//       if (client.available()) 
//       { // if there's bytes to read from the client,
//         char c = client.read(); // read a byte, then
//         Serial.write(c); // print it out the Serial monitor
//         header += c;
//         if (c == '\n') 
//         { // if the byte is a newline character
//         // if the current line is blank, you got two newline characters in a row.
//         // that's the end of the client HTTP request, so send a response:
//           if (currentLine.length() == 0) 
//           {
//             // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
//             // and a content-type so the client knows what's coming, then a blank line:
//             client.println("HTTP/1.1 200 OK");
//             client.println("Content-type:text/html");
//             client.println("Connection: close");
//             client.println("<head>");
//             client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
//               if (override_Counter > 0) 
//               {
//                 client.println("<meta http-equiv=\"refresh\" content=\"60;url=/\">");
//               }
//             // client.println("<link rel=\"icon\" href=\"data:,\">");
//             client.println("<style>html { font-family: Sans-serif; display: inline-block; margin: 0px auto; text-align: center;}");
//             client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
//             client.println(".override_on_text { font-variant: small-caps; font-weight: bold font-size: 30px; margin: 2px; cursor: pointer; }");
//             client.println(".override_on_color { background-color: rgba(255, 0, 0, 0.3); }");            
//             client.println(".override_off_text { font-variant: normal; font-size: 30px; margin: 2px; cursor: pointer; }");
//             client.println(".override_off_color { background-color: rgba(0, 255, 0, 0.3); }");            
//             client.println(".button2 {background-color: #555555; }");
//             client.println(".text-decoration: none; font-weight: bold; font-size: 40px; margin: 2px; cursor: pointer; }");
//             client.println("</style>");
//             client.println("</head>");
//             // Web Page Heading
//             client.println("<h1>ESP32 Web Server</h1>");
//             // Get current Override Mode
//             if (override_Counter > 0)
//             {
//               // Set minutes left on counter
//               int minutes_left_on_override = override_Counter / 60;
//               client.println("<p><div class=\"override_on_text\">");
//               client.println("Currently in Override Mode.  Number of Minutes left: ");
//               client.println((String)minutes_left_on_override);
//               client.println("</div></p>");
//             } 
//             else 
//             {
//               client.println("<p><div class=\"override_off_text>\"");
//               client.println("Override Currently Off");
//               client.println("</div></p>");
//               client.println("<p>On  Temperature: " + (String)on_Temp + "</p>");
//               client.println("<p>Off Temperature: " + (String)off_Temp + "</p>");      
//             }
//             // Get Current Fan State and convert to English
//             // Setup a string var to hold the English version
//             String current_Fan_State;
//             if (current_Fan_State) 
//             {
//               current_Fan_State = "ON";
//             } else {
//               current_Fan_State = "OFF";
//             }
//             // Display current State and temperature
//             client.println("<p>Current Fan State   " + current_Fan_State + "</p>");
//             client.println("<p>Current Temperature " + (String)temperature + "</p>");
//             // Display Override Buttons
//             if (current_Fan_State) 
//             {
//               // Display OFF Override button
//               client.println("<p><a href=\"/fans/off\"><button class=\"button\">Turn Fans OFF</button></a></p>");
//               debugOutput("INFO: Starting Override Mode - Fans locked ON for 30 minutes");
//             } else {
//               // Display ON Override button
//               client.println("<p><a href=\"/fans/on\"><button class=\"button button2\">Turn Fans ON</button></a></p>");
//               debugOutput("INFO: Starting Override Mode - Fans locked ON for 30 minutes");
//             }
//             // This bit checks whether a button has been pushed
//             // Turns the fans on
//             if (header.indexOf("GET /fans/on") >= 0) 
//             {
//               // Set the override timer
//               override_Counter = override_Cancel_Period;               
//               // Force Fans On
//               current_Fan_State = startFans();
//               // Redirect to root after button press
//               client.println("<head>");
//                 client.println("<meta http-equiv=\"refresh\" content=\"0;url=/\">");
//               client.println("</head>");
//             } 
//             else if (header.indexOf("GET /fans/off") >= 0) 
//             {
//               // Set the override timer
//               override_Counter = override_Cancel_Period;               
//               // Force Fans Off
//               current_Fan_State = stopFans();
//               // Redirect to root
//               client.println("<head>");
//                 client.println("<meta http-equiv=\"refresh\" content=\"0;url=/\">");
//               client.println("</head>");
//             }
//             // Clear the header variable
//             header = "";
//             // Close the connection
//             client.stop();
//           }
//         }
//       }
//     }
//   }
// }

String override_Active()
{
  // Is override active?
  if (override_Counter != 0)
  {
    // Override isn't active
    return "No ";
  }
  else 
  {
    // Override isn't active
    return "YES";
  }
}

boolean checkFanStatus() 
{
  boolean status = false;
  sensors_event_t temp_event;
  bme_temp->getEvent(&temp_event);
  // Store the values from the BME280 in local vars
  temperature = (int)temp_event.temperature;
  // Store whether the sensor was connected
  // Sanity check to make sure we are not underwater, or in space!
  if (temperature < -40 || temperature > 60) 
  {
    debugOutput("ERROR: Sensor Failure");
    // Set whether the Sensor is working
    status = false;
  } 
  else 
  {
    // Set whether the Sensor is working
    status = true;
    // Has the state changed?
    if (prev_State != current_Fan_State) 
    {
      // Is override active?
      if (override_Counter <= 0)
      {
        // Override isn't active
        // Are the fans on or off?
        if (current_Fan_State) 
        {
          // Fans are OFF
          if (temperature > on_Temp) {
            // START THE FANS!
            debugOutput("INFO: Starting Fans");
            current_Fan_State = startFans();
          }
        } 
        else 
        {
          // Fans are ON
          if (temperature < off_Temp ) 
          {
              debugOutput("INFO: Stopping Fans");
              current_Fan_State = stopFans();
          }
        }
      }
      // Store the previous temperature so we aren't displaying it when it hasn't changed
      if (temperature != prev_Temperature) {
        debugOutput(checkMode());
      } else {
        // Store the temperature from last time so we only output it once if it hasn't changed
        prev_Temperature = temperature;
      }
    }
  } 
  return status;
}

// Check current status of fans and override mode
String checkMode()
{
  String text = "";
  Serial.print("INFO: ");
  // Is override Mode active? 
  if (override_Counter > 0)
  {
    // text = text + "Override Mode: ON  Seconds Left: " + (String)override_Counter;
    Serial.print("Override Mode: ON  Seconds Left: " + (String)override_Counter);
  }
  else if (override_Counter <= 0)
  {
    // Is the temperature high enough?
    if (temperature >= on_Temp)
    {
      // text = text + "Override Mode: OFF Fans turn OFF at " + (String)off_Temp + " Temperature:  " + (String)temperature;
      Serial.print("Override Mode: OFF Fans turn OFF at " + (String)off_Temp + " Temperature: " + (String)temperature);
    }
    // Is the temperature low enough?
    else if (temperature <= off_Temp)
    {
      // text = text + "Override Mode: OFF Fans turn ON at "  + (String)on_Temp +  " Temperature:  " + (String)temperature;
      Serial.print("Override Mode: OFF Fans turn ON at "  + (String)on_Temp +  " Temperature: " + (String)temperature);
    }
  }

  if (current_Fan_State)
  {
    // Fans are on currently
    // text = text + "INFO: Current Fan Mode: ON  ";
    Serial.print(" Current Fan Mode: ON  ");
  } 
  else 
  {
    // Fans are off currently
    Serial.print(" Current Fan Mode: OFF ");
  }
  Serial.println();
  return text;
}
