#include <WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

#define BUTTON_PIN 21 // GIOP21 pin connected to button
#define SERVO_PIN 13 //GPIO13 pin connected to servo 
#define TRIGGER_PIN 5 //Triger D5
#define ECHO_PIN 18 //Echo D18

#define SOUND_SPEED 0.034

#define MOUSE_TOPIC "esp32/mouse_data" //pub
#define MOUSE_DETECTED "esp32cam/image_processing" //sub
#define ESP32_STATUS "esp32/status" //pub
#define APP_REQUEST "app/request_status" //sub

const char* ssid = "AIS_2.4GHz";
const char* password =  "2543katw";
const char* mqttServer = "projecttech.thddns.net";
const int mqttPort = 5050;
const char* mqttUser = "";
const char* mqttPassword = "";

bool defaultButtonValue = 1;  // Value when unpress the button
bool buttonPressed = 0;       // Press Flag
int doorState = 0;            //door need to be close as default

WiFiClient espClient;
PubSubClient client(espClient);
Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

//Ultrasonic variables
long duration;
float distanceCm;

// Subscribe Message
int payloadImage_msg = 0;
int payload_msg = 2;

void callback(char* topic, byte* payload, unsigned int length) {

  // Get the message
  Serial.print("\nMessage arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  payload[length] = '\0';
  char * charPointer = (char *)payload;
  

  if(strcmp(topic,"esp32cam/image_processing") == 0){
    if(isDigit(charPointer[0])){
      payloadImage_msg = atoi(charPointer);
    }
    //Serial.printf("\nInt = %d\n", payload_msg);
  } 
  else if (strcmp(topic, "app/request_status") == 0){
    if(isDigit(charPointer[0])){
      payload_msg = atoi(charPointer);
    }
    else{
      String doorStateString = String(doorState);
      client.publish(ESP32_STATUS, doorStateString.c_str());
    }
  }
 
  Serial.println();
  Serial.println("-----------------------");
}

void setupWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

// Connect with MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP32-Servo", mqttUser, mqttPassword )) {
      Serial.println("connected ");
      client.subscribe(APP_REQUEST);
      client.subscribe(MOUSE_DETECTED);
    } 
    
    else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(TRIGGER_PIN, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an Input
  myservo.attach(SERVO_PIN);  // attaches the servo on pin 13 to the servo object
  setupWifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  int buttonState = digitalRead(BUTTON_PIN);
  
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  
  distanceCm = duration * SOUND_SPEED/2;
  // detect the mouse ultrasonic
  if(doorState == 1) {
    if((distanceCm <= 6) and (payloadImage_msg == 1)){
      Serial.print("Distance (cm): ");
      Serial.println(distanceCm);
      Serial.println("Mouse Detected");
      client.publish(MOUSE_TOPIC,"Mouse Detected!");
      myservo.write(180);
      //delay(750); //180 degree turn
      delay(375);
      myservo.write(90); 
      delay(1000);
      doorState = 0;
      payloadImage_msg = 0;
      Serial.println("Door Closed"); 
    }
    String doorStateString = String(doorState);
    client.publish(ESP32_STATUS, doorStateString.c_str());
  }

  // reset image processing value
  payloadImage_msg = 0;
  
  if (buttonState != defaultButtonValue)
    buttonPressed = 1;
  else
    buttonPressed = 0;

  // print out the button's state
  // Serial.println(buttonState); 
  if((payload_msg != doorState) and (payload_msg != 2)){

    //Serial.println("change state");
    //doorState = payload_msg;
    
    if(payload_msg == 0){
      myservo.write(180);
      //delay(750); //180 degree turn
      delay(375);
      myservo.write(90); 
      delay(1000);
      doorState = payload_msg;
      payload_msg = 2;
      Serial.println("Door Closed"); 
    }
    else if(payload_msg == 1){
      myservo.write(-180);
      //delay(750); //180 degree turn
      delay(375);
      myservo.write(90); 
      delay(1000);
      doorState = payload_msg;
      payload_msg = 2;
      Serial.println("Door Opened");
    }
    String doorStateString = String(doorState);
    client.publish(ESP32_STATUS, doorStateString.c_str());
  }

  // Change door state when press button
  if(buttonPressed == 1) {
    if(doorState == 1){
      myservo.write(180);
      //delay(750); //180 degree turn
      delay(375);
      myservo.write(90); 
      delay(1000);
      doorState = 0;
      Serial.println("Door Closed"); 
    }
    else if(doorState == 0){
      myservo.write(-180);
      //delay(750); //180 degree turn
      delay(375);
      myservo.write(90); 
      delay(1000);
      doorState = 1;
      Serial.println("Door Opened");
    }
    buttonPressed = 0;
    String doorStateString = String(doorState);
    client.publish(ESP32_STATUS, doorStateString.c_str());
  }

  //Serial.println(payload_msg);
}
