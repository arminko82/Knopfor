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
#define USE_WIFI   0

#define _SSID "xxx"
#define _PASS "xxx"

const String SERVER_IP = "192.168.43.10";
const String SERVER_PORT = "6666";

const String CMD_MUTE = "mute";
const String CMD_UNMUTE = "unmute";

const int WORK_FREQUENCY = 10; //Hz

const bool STATE_1 = false;
const bool STATE_2 = true;

const int BUTTON_PIN = 0;
const int POWER_PIN = 2;

const int POWER_OFF_INTERVAL = 10000; //  milliseconds till power off on no action

bool _button_pressed = false;
bool _state = STATE_1; // the external true state does not matter, only the state transition does.
bool _main_power = true; // while true chip shall run

int _last_action_time = 0;

void setup()
{
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);
    pinMode(BUTTON_PIN, INPUT);

#if USE_SERIAL
    Serial.begin(115200);
    while (!Serial); // wait active
    log("Serial Started");
#endif

#if USE_WIFI
    WiFi.begin(_SSID, _PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(5);
    }
    log("WIFI Started");
#endif
    _last_action_time = millis();
}

/*
* Checks for state changes (button pressed for a short amount of time).
* On change sends an http get request
*/
void loop()
{
    auto ellapsed = millis() - _last_action_time;
    _main_power = ellapsed <= POWER_OFF_INTERVAL;
    log(String(ellapsed));
    if(!_main_power)
    {
        log("Powering down");
        digitalWrite(POWER_PIN, LOW); // power off, global exit
    }
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
    delay(1000 / WORK_FREQUENCY);
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
    log("Sent " + request + " with result "+ response);
#endif // USE_WIFI
}

// Logs in format sprintf(logString, "%10lu: %s", millis(), msg);
void log(String msg)
{
#if USE_SERIAL
    char logString[100];
    sprintf(logString, "%10lu: %s", millis(), msg.c_str());
    Serial.println(logString);
#endif // USE_SERIAL
}
