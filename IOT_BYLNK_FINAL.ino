// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings

#define BLYNK_TEMPLATE_ID "TMPL6pxpa5e0S"
#define BLYNK_TEMPLATE_NAME "POULTRY CONTROL SYSTEM"
#define BLYNK_AUTH_TOKEN "jtsO6QC5AkTbbCJYaaaZum8kAWklUE3m"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include<SoftwareSerial.h> 

SoftwareSerial mySerial (D1, D2); //Rx, TX pins

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "abc";
char pass[] = "11221122";

BlynkTimer timer; 

void setup()
{
  // Debug console
  Serial.begin(9600);
  mySerial.begin(9600);
  
  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
}
// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V2, millis() / 1000);
}

void loop() {
  Blynk.run();
  timer.run();
  
  String msg = mySerial.readStringUntil('\r');

  // Extract values from the string and store them in variables
  String values[6]; // Create an array to store the values
  int i = 0;
  for (int j = 0; j < msg.length(); j++) {
    if (msg[j] == ' ' || msg[j] == '\r') { // Check for a space or a newline character
      i++;
    } 
    else {
      values[i] += msg[j]; // Add the current character to the current value string
    }
  }
  
  float buttonstate = values[0].toInt();
  float temperature = values[1].toFloat();
  float humidity = values[2].toFloat();
  int waterlevel = values[3].toInt();
  int feedlevel = values[4].toInt();
  float NH4 = values[5].toFloat();

if(humidity > 1){
    Serial.print("MODE Bruder (0) | Incubator (1): ");
    Serial.print(buttonstate);
    Serial.println(" ");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.print("Water Level: ");
    Serial.print(waterlevel);
    Serial.println(" %");
    Serial.print("Feed Level: ");
    Serial.print(feedlevel);
    Serial.println(" %"); 
    Serial.print("NH4 PPM: ");
    Serial.print(NH4);
    Serial.println(" ppm");

    if (buttonstate == 0.00){
      // Send the sensor data to Blynk virtual pins for Bruder mode
      Blynk.virtualWrite(V1, 1);
      Blynk.virtualWrite(V2, waterlevel);
      Blynk.virtualWrite(V3, humidity);
      Blynk.virtualWrite(V4, temperature);
      Blynk.virtualWrite(V5, feedlevel);
      Blynk.virtualWrite(V6, NH4);
      Serial.println("Wait...");
    }
    else{
      // Send the sensor data to Blynk virtual pins for incubator mode
      Blynk.virtualWrite(V1, 0);
      Blynk.virtualWrite(V2, 0);
      Blynk.virtualWrite(V3, humidity);
      Blynk.virtualWrite(V4, temperature);
      Blynk.virtualWrite(V5, 0);
      Serial.println("Wait...");
    }
  }
  
  delay(8000); // Delay before reading next data from serial
}