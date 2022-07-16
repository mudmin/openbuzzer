#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h> //v6.11.5 from Library Manager


//make sure to put your own wifi credentials here
const char* ssid = "YOUR_SSID_HERE";
const char* password = "YOUR_PASSWORD_HERE";
int penalty(2500); //how many ms to penalize a buzzer who buzzes when they shouldn't

// Use arduinojson.org/v6/assistant to compute the capacity.
DynamicJsonDocument doc(1024);

//enter your own credentials here
const char* api = "https://openbuzzer.com/target?owner=1&buzz=1&key=1234";


#include <CuteBuzzerSounds.h>
int blue = 13;   //D7
int green = 4;   //D2
int red = 14;   //D5
int button = 12; //D6
#define BUZZER_PIN 5 //D1


void setup(){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  digitalWrite(red,HIGH);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting...");
  }
  digitalWrite(red,LOW);

  pinMode(button, INPUT_PULLUP);
  cute.init(BUZZER_PIN);
  cute.play(S_CONNECTION);
}

void loop(){
  if(digitalRead(button) == LOW){
    Serial.println("Button Press");
    apiCall();
  }
}

void ledOff(){
  digitalWrite(blue,LOW);
  digitalWrite(red,LOW);
  digitalWrite(green,LOW);
}

void apiCall(){
  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
    Serial.println("Trying API");
    HTTPClient http;    //Declare object of class HTTPClient
    WiFiClientSecure client;
    client.setInsecure();
    http.begin(client,api);      //Specify request destination
    http.addHeader("Content-Type", "text/plain");  //Specify content-type header
    int httpCode = http.GET(); //if you want GET
    String payload = http.getString();
    Serial.println(payload);
    deserializeJson(doc, payload);              //Get the response payload
    // Extract values
    Serial.println(F("Response:"));
    Serial.println(doc["msg"].as<char*>());

    Serial.println(doc["success"].as<int>());
    if(doc["success"] == 1){
      Serial.println("Success");
      // digitalWrite(green,HIGH);
      digitalWrite(blue,HIGH);
      // digitalWrite(red,HIGH);
      cute.play(S_HAPPY_SHORT);
      delay(3000);
      ledOff();
    }else{
      Serial.println("Fail");
      //there is a penalty for pushing the buzzer early, so you are MUCH better off pushing the button just after it's activated than keep hitting the button until it's activated.
      //you can adjust the delay in between the led writes. It's the "penalty" for buzzing in when they shouldn't
      digitalWrite(green,LOW);
      digitalWrite(blue,LOW);
      digitalWrite(red,HIGH);
      delay(penalty);
      ledOff();
    }

    http.end();  //Close connection

  }else{

    Serial.println("Error connecting to API");

  }
}
