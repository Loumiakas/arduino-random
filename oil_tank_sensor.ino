#include <ESP8266WiFi.h>
#include <HCSR04.h>
#include <MQTT.h>
/*
 *  Network constants
 */
const char *    NTWK_SSID = "<NTWK_SSID>";
const char *    NTWK_PASS = "<NTWK_PASS>";
/*
 *  MQTT constants
 */
const char *    MQTT_HOST = "<MQTT_HOST>";
const char *    MQTT_USER = "<MQTT_USER>";
const char *    MQTT_PASS = "<MQTT_PASS>";
/*
 *  Pin definitions
 */
const uint8_t   TRIG_PIN = 14;      // D5
const uint8_t   ECHO_PIN = 12;      // D6
/*
 *  Fuel tank dimensions
 */
const uint8_t   TANK_HEIGHT = 105;
const uint8_t   TANK_LENGTH = 165;
const uint8_t   TANK_WIDTH  = 65;
/*
 *  Other constants
 */
const uint8_t   WIFI_MQTT_TRY  = 10;            // Attempts before going into deep-sleep
const uint64_t  SLEEP_DURATION = 21600000000;   // 6 hours
/*
 *  Global variables
 */
WiFiClient      net;
MQTTClient      client;
HCSR04          hc(TRIG_PIN, ECHO_PIN);
int             lastMillis = 0;
uint8_t         attempt    = 0;

//
//  Helper function for deep-sleep processing
//
void go_deep_sleep() {
    Serial.println("Entering deep-sleep state...");
    ESP.deepSleep(SLEEP_DURATION, WAKE_RF_DEFAULT);
}

//
//  This function sends an ultarasonic ping and performs simple calculation to
//  get current fuel level
//
int get_tank_capacity() {
    unsigned int sonar_depth_cm = hc.dist();
    
    if (sonar_depth_cm > (TANK_HEIGHT + 5)) {
        return 0;
    }
    else {
        return ((TANK_HEIGHT - sonar_depth_cm + 5) * TANK_LENGTH * TANK_WIDTH / 1000);
    }
}

//
//  Function that performs Wifi and MQTT connections, if connection fails
//  after 10 attempts, it will put the device to deep-sleep.
//
void connect() {
    Serial.println("Attempting to connect to network...");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);

        if (attempt >= WIFI_MQTT_TRY) {
            go_deep_sleep();
        }
        attempt++;
    }

    attempt = 0;

    Serial.println("\nAttempting to connect to MQTT server...");
    while (!client.connect(MQTT_HOST, MQTT_USER, MQTT_PASS)) {
        Serial.print(".");
        delay(1000);

        if (attempt >= WIFI_MQTT_TRY) {
            go_deep_sleep();
        }
        attempt++;
    }

    Serial.println("\nConnected!!");
    client.subscribe("/oil_sensor");
}

void messageReceived(String &topic, String &payload) {
    Serial.println("Incoming data: " + topic + " - " + payload);
    go_deep_sleep();
}

void setup() {
    pinMode(TRIG_PIN, OUTPUT); 
    pinMode(ECHO_PIN, INPUT); 

    Serial.begin(9600);
    WiFi.begin(NTWK_SSID, NTWK_PASS);

    client.begin(MQTT_HOST, net);
    client.onMessage(messageReceived);

    connect();
}

void loop() {
    client.loop();
    delay(10);  // <- fixes some issues with WiFi stability

    if (!client.connected()) {
        connect();
    }
    //
    //  Attempt to publish fuel levels using MQTT and put device into
    //  deep-sleep
    //
    if (millis() - lastMillis > 1000) {
        lastMillis = millis();
        String volume = String(get_tank_capacity(), DEC);
        client.publish("/oil_sensor", volume);
    }
}
