#include <ESP8266WiFi.h>
#include <Servo.h>
#include <PubSubClient.h>
  //Define the different pins used in the project
#define POWERPIN D1
#define DIRECTIONPIN D3
#define SERVOPIN D0
#define ECHOPIN D5
#define TRIGPIN D6
#define STEP_UP_INITIAL_VALUE 1024

Servo servo;

   //Define the different credentials for wifi and maqiatto
const char* ssid = "KjellUrbansmobilaenhet";
const char* password = "Herkukes";
const char* mqttServer = "maqiatto.com";
const int mqttPort = 1883;
const char* topicControl = "alwin.forslund@hitachigymnasiet.se/driverbot/control";
const char* topicDistance = "alwin.forslund@hitachigymnasiet.se/driverbot/distance";
const char* mqttUsername = "alwin.forslund@hitachigymnasiet.se";
const char* mqttPassword = "Kingsvilleracecrew";

WiFiClient espClient;
PubSubClient client(espClient);

float distance = 0;  // Global distance variable
int SHOULDROAM = 0;   // Flag to indicate if roaming should be enabled

  //Reverse function
void reverse() {
    digitalWrite(DIRECTIONPIN, LOW);
    analogWrite(POWERPIN, 512);
    Serial.println("reverse");
}

  //Stop function
void hardstop() {
    analogWrite(POWERPIN, 0);
    Serial.println("HARDSTOP");
}

  //Forward function
void forward() {
    digitalWrite(DIRECTIONPIN, HIGH);
    analogWrite(POWERPIN, 1024);
    Serial.println("forwardslow");
}

  //MQTT-Message processing
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.println("Message arrived");
    String message = "";

    Serial.print("Received message: ");
    for (int i = 0; i < length; i++) {
        char character = (char)payload[i];
        message += character;
        Serial.print(character);
    }
    Serial.println();

  //Call for different functions depending on message
    if (strcmp(topic, topicControl) == 0) {
        if (message.equals("forward")) {
            Serial.println("Received forward command");
            forward();
        } else if (message.equals("reverse")) {
            Serial.println("Received reverse command");
            reverse();
        } else if (message.equals("hardstop")) {
            Serial.println("Received hardstop command");
            hardstop();
        } else if (message.equals("roam")) {
            Serial.println("Received roam command");
            SHOULDROAM = 1;
        } else if (message.equals("stoproam")) {
            Serial.println("Received stop_roam command");
            SHOULDROAM = 0;
            hardstop();
        } else if (message.toInt() >= 0 && message.toInt() <= 180) {
            servo.write(message.toInt());
        }
    }
}
  //MQTT connection/reconnection
void mqttReconnect() {
    while (!client.connected()) {
        Serial.println("Attempting MQTT (re)connection");
        if (client.connect("trehjuling", mqttUsername, mqttPassword)) {
            Serial.println("MQTT connected successfully");
            client.subscribe(topicControl);  // Subscribe to the control topic
        } else {
          //Debug MQTT failure
            Serial.print("MQTT Failure, rc=");
            Serial.print(client.state());
            Serial.println(", trying again in 5 seconds");
            delay(5000);  
        }
    }  
}

void setup() {
    Serial.begin(9600);
  //WiFi connection
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to the WiFi network");
  //Set the pinmodes to output/inputs
    servo.attach(SERVOPIN);
    pinMode(POWERPIN, OUTPUT);
    pinMode(DIRECTIONPIN, OUTPUT);
    pinMode(TRIGPIN, OUTPUT);
    pinMode(ECHOPIN, INPUT);
    analogWrite(POWERPIN, 1);
    digitalWrite(DIRECTIONPIN, 1);

    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);
}

void loop(){
    if (!client.connected()) {
        mqttReconnect();
    }

    client.loop();
  //Configure the Roam function
    if (SHOULDROAM == 1) {
        if (distance < 20) {
            hardstop();
            servo.write(180);  // Turn servo to 180 degrees
            delay(500); // Wait for the servo to turn
            reverse();
            delay(500); // Reverse for half a second
        } else if (distance < 40) {
            servo.write(40);  // Turn servo to 40 degrees
            Serial.println("Turning left");
            delay(500); // Wait for the servo to turn
            forward();
        } else {
            forward();
            servo.write(85); //Center the steering
        }
    }

    // Send trigger pulse
    digitalWrite(TRIGPIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGPIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGPIN, LOW);

    // Measure echo pulse
    long duration = pulseIn(ECHOPIN, HIGH);

    // Calculate distance
    distance = duration * 0.034 / 2;

    // Publish distance to MQTT topic
    String distanceStr = String(distance);
    client.publish(topicDistance, distanceStr.c_str());

    delay(500); // Wait before next measurement
}
