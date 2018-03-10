/*
 * Name: StannaButton
 * Author: ARMIN
 * Version V3
 *
 * GPIO2 is now responsible for the power supply of the ESP8266 itself. As soon as the chip gets booted into the setup routine
 * it must be put to high so that the external transistor+relay keeps the power supply alive. If the chip should go to deeptest sleep
 * GPIO2 should be put to low.
 * 
 * GPIO0 detects the button presses that shall be consumed as business lgic part. Getting a high flank signals a pressed key.
 * 
 * The ESP8266 is used to interpret button presses and to send them via HTTP GET to a predefined station.
 * TODO: Define button press events like click, double click, long press, ...
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define USE_SERIAL 1
#define USE_WIFI    0

#define _SSID "xxx"
#define _PASS "xxx"

const String SERVER_IP = "192.168.43.10";
const String SERVER_PORT = "6666";

const String CMD_MUTE = "mute";
const String CMD_UNMUTE = "unmute";

const bool STATE_1 = false;
const bool STATE_2 = true;

const int BUTTON_PIN = 0;
const int POWER_PIN = 2;

const int POWER_OFF_INTERVAL = 30000; // 30 seconds till power off on now action

bool _button_pressed = false;
bool _state = STATE_1; // the external true state does not matter, only the state transition does.
bool _main_power = true; // while true chip shall run

int _last_action_time = 0;
#if USE_SERIAL
const int LIGHT_INTERVAL = 1000;
int _last_light_time = 0;
#endif

void setup() 
{
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);
  pinMode(BUTTON_PIN, INPUT);
  
#if USE_SERIAL
  Serial.begin(115200);
  while (!Serial); // wait active
#endif

#if USE_WIFI
  WiFi.begin(_SSID, _PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(5);
   }
#endif 
   _last_action_time = millis();
}

/*
 * Checks for state changes (button pressed for a short amount of time).
 * On change sends an http get request
 */
void loop() 
{
    _main_power = millis() - _last_action_time >= POWER_OFF_INTERVAL;
    if(!_main_power)
    {
      digitalWrite(POWER_PIN, LOW); // power off, global exit
    }
#if USE_SERIAL
    if(_main_power && millis() - _last_light_time > LIGHT_INTERVAL)
    {
      for(int i = 0; i < 10; i++)
        Serial.println("messagemessagemessagemessagemessage");
      _last_light_time = millis();
    }
#endif
    bool button_pressed = digitalRead(BUTTON_PIN);
    if(button_pressed != _button_pressed) 
    {
      _button_pressed = button_pressed;
      if(!button_pressed) // released button
      {
        publishStateChange();
      }
    }
//#if USE_SERIAL
//    Serial.println(button_pressed);
//#endif
    delay(16); // roughly 60 Hz
}

void publishStateChange()
{
  _last_action_time = millis();
  String command;
  if(_state == STATE_1)
  {
    _state = STATE_2;
    command = CMD_MUTE;
  }
  else
  {
    _state = STATE_1;
    command = CMD_UNMUTE;    
  }
#if USE_WIFI
  HTTPClient http;
  String request = "http://" + SERVER_IP + ":" + SERVER_PORT + "/oinkservice/" + command;
  http.begin(request);
  int response = http.GET();
#if USE_SERIAL
  Serial.println("Sent " + request + " with result "+ response);
#endif // USE_SERIAL
#endif // USE_WIFI
}

