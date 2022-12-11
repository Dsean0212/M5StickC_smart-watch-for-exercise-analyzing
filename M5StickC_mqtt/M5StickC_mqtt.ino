#include <PubSubClient.h>
#include <WiFi.h>

#include "M5StickC.h"

float gyroX, gyroY, gyroZ;
float accX, accY, accZ;

WiFiClient espClient;
PubSubClient client(espClient);

// Configure the name and password of the connected wifi and your MQTT server host. 
const char* ssid        = "Your_WiFi_ssid";
const char* password    = "Your_WiFi_password";
const char* mqtt_server = "mqtt.beebotte.com";

const char* channelToken = "Your_Token"; // Will be something looks like: "token_************"
const char* Topic = "Your_Channel/Your_Resource"; // This is your MQTT topic
const char* GyroTopic = "IoT_gogogo/gyro"; // You may have multiple topics at the same time
const char* AccTopic = "IoT_gogogo/acc";   // to deliver with different categories of the sensors data

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
char msg_2[MSG_BUFFER_SIZE];
int value = 0;

void setupWifi();
void callback(char* topic, byte* payload, unsigned int length);
void reConnect();

void setup() {
    Serial.begin(115200);
  
    M5.begin();
    M5.IMU.Init();
    
    M5.Lcd.setRotation(3);
    setupWifi();
    
    client.setServer(mqtt_server, 1883);  // Sets the server details. 
    client.setCallback(callback);  // Sets the message callback function. 
}

void loop() {

    if (!client.connected()) {
        reConnect();
    }
    
    client.loop();  // This function is called periodically to allow clients to
                    // process incoming messages and maintain connections to the
                    // server.

    unsigned long now = millis();  // Obtain the host startup duration. 

    M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
    M5.IMU.getAccelData(&accX, &accY, &accZ);
    
    if (now - lastMsg > 2000) {
        lastMsg = now;
        //++value;
        //snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);  // Format to the specified string and store it in MSG.
        snprintf(msg, MSG_BUFFER_SIZE, "X:%7.2f,Y:%7.2f,Z:%7.2f", gyroX, gyroY, gyroZ);
        snprintf(msg_2, MSG_BUFFER_SIZE, "X:%5.2f,Y:%5.2f,Z:%5.2f", accX, accY, accZ);

        M5.Lcd.print("Publish message: ");
        M5.Lcd.println(msg);
        client.publish(GyroTopic, msg);  // Publishes a message to the specified topic. 
        if (value % 4 == 0) {
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0, 0);
        }

        M5.Lcd.print("Publish message: ");
        M5.Lcd.println(msg_2);
        client.publish(AccTopic, msg_2);  // Publishes a message to the specified topic. 
        if (value % 4 == 0) {
            M5.Lcd.fillScreen(BLACK);
            M5.Lcd.setCursor(0, 0);
        }
    }
}

void setupWifi() {
    delay(10);
    M5.Lcd.printf("Connecting to %s", ssid);
    WiFi.mode(WIFI_STA);  // Set the mode to WiFi station mode. 
    WiFi.begin(ssid, password);  // Start Wifi connection. 

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Lcd.print(".");
    }
    M5.Lcd.printf("\nSuccess\n");
}

void callback(char* topic, byte* payload, unsigned int length) {
    M5.Lcd.print("Message arrived\n[");
    M5.Lcd.print(topic);
    M5.Lcd.print("]\n");
    for (int i = 0; i < length; i++) {
        M5.Lcd.print((char)payload[i]);
    }
    M5.Lcd.println();
}

void reConnect() {
    while (!client.connected()) {
        M5.Lcd.print("Attempting MQTT connection...");

        String username = "token:";
        username += channelToken;
        
        // Create a random client ID. 
        String clientId = "M5Stack-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect. 
        if (client.connect(clientId.c_str(), username.c_str(), NULL)) {
            M5.Lcd.printf("\nSuccess\n");
            
            // Once connected, publish an announcement to the topic.
            client.publish(Topic, "hello world");
            
            // ... and resubscribe. 
            client.subscribe(Topic);
            client.subscribe(GyroTopic);
            client.subscribe(AccTopic);
        } else {
            M5.Lcd.print("failed, rc=");
            M5.Lcd.print(client.state());
            M5.Lcd.println("try again in 5 seconds");
            delay(5000);
        }
    }
}
