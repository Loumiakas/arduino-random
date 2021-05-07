#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <NewPing.h>

#define TRIG_PIN        14      // D5
#define ECHO_PIN        12      // D6
#define MAX_DISTANCE    200     // 2 metres
#define SLEEP_DURATION  21600   // 6 hours
#define WIFI_MQTT_TRY   10      // Attempts before going into deep-sleep
/*
 *  Fuel tank dimensions
 */
unsigned const int tank_height = 105;
unsigned const int tank_length = 165;
unsigned const int tank_width  = 65;
/*
 *  Network constants
 */
const char  *NTWK_SSID = "<SSID>";
const char  *NTWK_PASS = "<PSWD>";
/*
 *  MQTT constants
 */
const char  *MQTT_HOST = "<HOSTADDR>";
const char  *MQTT_USER = "<MQTT_USER>";
const char  *MQTT_PASS = "<MQTT_PASS>";
/*
 *  Global variables
 */
WiFiClient      net;
MQTTClient      client;
NewPing         sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);
unsigned long   lastMillis = 0;
unsigned long   attempt    = 0;

//
//  Helper function for deep-sleep processing
//
void go_deep_sleep() {
    Serial.println("Entering deep-sleep state...");
    ESP.deepSleep(SLEEP_DURATION * 1000000UL, WAKE_RF_DEFAULT);
}

//
//  This function sends an ultarasonic ping and performs simple calculation to
//  get current fuel level
//
int get_tank_capacity() {
    delay(50);
    unsigned int sonar_depth_cm = (sonar.ping() / US_ROUNDTRIP_CM);
    return ((tank_height - sonar_depth_cm + 5) * tank_length * tank_width / 1000);
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
    while ( attempt < 10 ) {
        Serial.println("Incoming data: " + topic + " - " + payload);
        attempt++;
    }
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
        client.publish("/oil_sensor", String(get_tank_capacity(), DEC));

        if ( attempt >= WIFI_MQTT_TRY ) {
            go_deep_sleep();
        }
        attempt++;
    }
}
