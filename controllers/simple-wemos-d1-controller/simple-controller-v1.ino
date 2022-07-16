/*********
  AnotherMaker Simple ESP8266 Gameshow Controller
  https://youtube.com/AnotherMaker

  Webserver contributions by Rui Santos from https://randomnerdtutorials.com
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>

// Replace with your network credentials
const char* ssid     = "YOUR_SSID_HERE";
const char* password = "YOUR_PASSWORD_HERE";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

int player1 = 0; //red
int player2 = 0; //green
int player3 = 0; //yellow
int player4 = 0; //blue

//LEDs are wired from ground to the short leg of the LED, through a 330ohm resistor (ish) with the long leg going to the pin below

int red = 5;     //D1
int green = 4;   //D2
int blue = 0;   //D3
int yellow = 2; //D4

//buttons are wired with one leg of each button tied together and to ground. The other leg going directly to the pin specified below.
int resetBtn = 16; //D0
int goBtn = 14;    //D5
int unlockBtn = 12;//D6
int button = 0;
int state = -1;
//states
// -1 = booted
//  0 = Asking the question, if they buzz in, they're locked out
//  1 = Available people can buzz in
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
    Serial.begin(115200);
  // Connect to Wi-Fi network with SSID and password
   Serial.print("Connecting to ");
   Serial.println(ssid);
   WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }
   // Print local IP address and start web server
   Serial.println("");
   Serial.println("WiFi connected.");
   Serial.println("IP address: ");
   Serial.println(WiFi.localIP());
   server.begin();


  pinMode(resetBtn, INPUT_PULLUP);
  pinMode(unlockBtn, INPUT_PULLUP);
  pinMode(goBtn, INPUT_PULLUP);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(yellow, OUTPUT);


}


void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
          if (c == '\n') {                    // if the byte is a newline character
            // if the current line is blank, you got two newline characters in a row.
            // that's the end of the client HTTP request, so send a response:
            if (currentLine.length() == 0) {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();

              if(state == 0){ //they buzzed early
              if (header.indexOf("GET /1/buzz") >= 0) {
                player1 = 0; //make the button unavailable
                digitalWrite(red, LOW); //turn the led off
                Serial.println("red disabled");
              } else if (header.indexOf("GET /2/buzz") >= 0) {
                player2 = 0;
                digitalWrite(green, LOW);
                Serial.println("green disabled");
              } else if (header.indexOf("GET /3/buzz") >= 0) {
                player3 = 0;
                digitalWrite(yellow, LOW);
                Serial.println("yellow disabled");
              } else if (header.indexOf("GET /4/buzz") >= 0) {
                player4 = 0;
                digitalWrite(blue, LOW);
                Serial.println("blue disabled");
              }
            } else if (state == 1) { // they buzzed in on time
              if (header.indexOf("GET /1/buzz") >= 0 && player1 == 1) {
                player1 = 0; //make the button unavailable
                digitalWrite(red, LOW);
                delay(100);
                digitalWrite(red, HIGH); //turn the led on
                digitalWrite(yellow,LOW);
                digitalWrite(green,LOW);
                digitalWrite(blue,LOW);
                Serial.println("red buzzed in");

                state = 2;
              } else if (header.indexOf("GET /2/buzz") >= 0 && player2 == 1) {
                player2 = 0;
                digitalWrite(green, LOW);
                delay(100);
                digitalWrite(green, HIGH);
                digitalWrite(yellow,LOW);
                digitalWrite(red,LOW);
                digitalWrite(blue,LOW);
                Serial.println("green buzzed in");
                state = 2;
              } else if (header.indexOf("GET /3/buzz") >= 0 && player3 == 1) {
                player3 = 0;
                digitalWrite(yellow, LOW);
                delay(100);
                digitalWrite(yellow, HIGH);
                digitalWrite(red,LOW);
                digitalWrite(green,LOW);
                digitalWrite(blue,LOW);
                Serial.println("yellow buzzed in");
                state = 2;
              } else if (header.indexOf("GET /4/buzz") >= 0 && player4 == 1) {
                player4 = 0;
                digitalWrite(blue, LOW);
                delay(100);
                digitalWrite(blue, HIGH);
                digitalWrite(yellow,LOW);
                digitalWrite(green,LOW);
                digitalWrite(red,LOW);
                Serial.println("blue buzzed in");
                state = 2;
              }
            }
              // Display the HTML web page
              client.println("<!DOCTYPE html><html>");
              client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");

              // Web Page Heading
              client.println("<body><h1>Game show controller</h1>");
              client.println("</body></html>");

              // The HTTP response ends with another blank line
              client.println();
              // Break out of the while loop
              break;
            } else { // if you got a newline, then clear currentLine
              currentLine = "";
            }
          } else if (c != '\r') {  // if you got anything else but a carriage return character,
            currentLine += c;      // add it to the end of the currentLine
          }
        }
      }
      // Clear the header variable
      header = "";
      // Close the connection
      client.stop();
      Serial.println("Client disconnected.");
      Serial.println("");
    }
button = digitalRead(resetBtn);
if(button == LOW && state != 0){ //button a resets everything
  player1 = 1;
  player2 = 1;
  player3 = 1;
  player4 = 1;

  digitalWrite(red,HIGH);
  digitalWrite(green,HIGH);
  digitalWrite(yellow,HIGH);
  digitalWrite(blue,HIGH);

  state = 0;
  Serial.println("State set to 0. Everything reset.");
}
button = digitalRead(goBtn);
if(button == LOW){ // Buttons unlocked!
      state = 1;
      //if you comment in the line below, the lights will blink 3 times showing the contestants that it is ok to buzz in. This is 2 places in the code.
      blinkToGo();

      //This function resets the LEDs. Take a look at it at the bottom of the code.
      resetLeds();
      Serial.println("State set to 1. Available Players can buzz!!!");
}

button = digitalRead(unlockBtn);
if(button == LOW){ // Buttons unlocked!
      state = 1;
      blinkToGo();
      resetLeds();
      Serial.println("State set to 1. Available Players can buzz!!!");
}
}

void blinkToGo(){
  for (int i = 0; i < 3; i++) {
    digitalWrite(red,HIGH);
    digitalWrite(yellow,HIGH);
    digitalWrite(green,HIGH);
    digitalWrite(blue,HIGH);
    delay(100);
    digitalWrite(red,LOW);
    digitalWrite(yellow,LOW);
    digitalWrite(green,LOW);
    digitalWrite(blue,LOW);
    delay(100);
  }
}

void resetLeds(){
  //Let's reset the LEDs. If someone has already buzzed in, we want their LED to be off
  //If they haven't, we want it to be on.
  if(player1 == 1){
    digitalWrite(red,HIGH);
  }else{
    digitalWrite(red,LOW);
  }

  if(player2 == 1){
    digitalWrite(green,HIGH);
  }else{
    digitalWrite(green,LOW);
  }

  if(player3 == 1){
    digitalWrite(yellow,HIGH);
  }else{
    digitalWrite(yellow,LOW);
  }

  if(player4 == 1){
    digitalWrite(blue,HIGH);
  }else{
    digitalWrite(blue,LOW);
  }


}
