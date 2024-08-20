// Include the libraries
#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>
#include <DHT.h>  
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <NewPing.h>
#include <MQ135.h>
#include <SPI.h>
#include <uRTCLib.h>
#include <EEPROM.h>

// Define DHT sensor type and pin
#define DHT_TYPE DHT11   // Define the type of the DHT sensor
#define DHT_PIN 2        // Pin where the DHT sensor is connected

int sensorPin = A0;      // select the input pin for the water level sensor
const int FAN_PIN = 9;  // Pin where the fan is connected
const int motorPin = 10;       // select the output pin for the motor
const int BULB_PIN = 11; // Pin where the bulb is connected
const int Hum_PIN = 12; // Pin where the Humiditifier is connected
const int egg_Roller = 13;       // select the output pin for the motor

// Other pin definitions
#define TRIGGER_PIN 5
#define ECHO_PIN 4
#define MAX_DISTANCE 100 // Maximum distance (in cm) that the ultrasonic sensor can measure
#define SERVO_PIN 8
#define PUSH_BUTTON_PIN 6 // Pin where the push button is connected

#define bt_set  A1
#define bt_up   A2
#define bt_down A3

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servo;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
DHT dht(DHT_PIN, DHT_TYPE);
uRTCLib rtc(0x68);

#define MQ135_PIN 3       // Pin for MQ135 sensor
#define RLOAD 1.0
#define r0Air 1
#define scaleFactorNH4 102.694
#define exponentNH4 2.48818
#define atmNH4 15

// Thresholds for Bruder mode
float TEMP_THRESHOLD_HIGH_BRUDER = 32.0;    // Threshold temperature for turning on the fan in Bruder mode
float TEMP_THRESHOLD_LOW_BRUDER = 15.0;     // Threshold temperature for turning on the bulb in Bruder mode
float HUM_THRESHOLD_BRUDER = 60;          // Threshold humidity for turning on the humidifier in Bruder mode

// Thresholds for Incubator mode
float TEMP_THRESHOLD_HIGH_INCUBATOR = 37.0; // Threshold temperature for turning on the bulb in Incubator mode
float HUM_THRESHOLD_INCUBATOR = 50;       // Threshold humidity for turning on the humidifier in Incubator mode

int Set = 0, flag = 0, flash = 0;

#define ROTATE_INTERVAL 8 * 3600 * 1000 // Interval between egg rotations (8 hours)
#define ROLLER_DURATION 5 * 1000        // Duration of egg roller activation (5 seconds)
#define ROLLER_COUNT_MAX 3              // Maximum number of times egg roller activates per day

MQ135 gasSensor = MQ135(MQ135_PIN, RLOAD, r0Air);

unsigned long lastRotateTime = 0;
int rollerCount = 0;

void setup() {
  Serial.begin(9600);       // Initialize serial communication
  lcd.begin();              // initialize the lcd 
  lcd.backlight();          // turn on the backlight
  pinMode(FAN_PIN, OUTPUT);   // Set the fan pin as an output
  pinMode(BULB_PIN, OUTPUT);  // Set the bulb pin as an output
  pinMode(Hum_PIN, OUTPUT);   // Set the Humiditifier pin as an output
  pinMode(sensorPin, INPUT);  // set the sensor pin as an input
  pinMode(motorPin, OUTPUT);  // set the motor pin as an output
  pinMode(egg_Roller, OUTPUT);  // set the egg roller pin as an output
  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP); // set the push button pin as an input with internal pull-up resistor
  servo.attach(SERVO_PIN);    // connect servo motor
  dht.begin();                // Initialize the DHT sensor

  // Pin setup
  pinMode(bt_set,  INPUT_PULLUP);
  pinMode(bt_up,   INPUT_PULLUP);
  pinMode(bt_down, INPUT_PULLUP);

  // Set the RTC time
  URTCLIB_WIRE.begin();       // Initialize the RTC module
  rtc.set(0, 12, 7, 59, 16, 7, 24);  // Set the RTC module time

  // Initialize the LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("   Welcome To   ");
  lcd.setCursor(0, 1);
  lcd.print("Bruder/Incubator");

  // Initialize EEPROM (assuming the values should be stored if not already)
  if (EEPROM.read(0) == 0) {
    EEPROM.put(10, TEMP_THRESHOLD_HIGH_BRUDER);
    EEPROM.put(20, TEMP_THRESHOLD_LOW_BRUDER);
    EEPROM.put(30, HUM_THRESHOLD_BRUDER);
    EEPROM.put(40, TEMP_THRESHOLD_HIGH_INCUBATOR);
    EEPROM.put(50, HUM_THRESHOLD_INCUBATOR);
    EEPROM.write(0, 1);
  } else {
    EEPROM.get(10, TEMP_THRESHOLD_HIGH_BRUDER);
    EEPROM.get(20, TEMP_THRESHOLD_LOW_BRUDER);
    EEPROM.get(30, HUM_THRESHOLD_BRUDER);
    EEPROM.get(40, TEMP_THRESHOLD_HIGH_INCUBATOR);
    EEPROM.get(50, HUM_THRESHOLD_INCUBATOR);
  }

  delay(2000); // Wait 2000 ms
  lcd.clear();
}

void loop() {
  // Read the temperature and humidity from the DHT sensor
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // read the water level sensor value
  int sensorValue = analogRead(sensorPin);
  
  // Measure the distance of feed box
  int feedlevel =  sonar.ping_cm();

  servo.write(160);
  
  // Measure the feedlevel in percentage
  if (feedlevel <= 9) {
    feedlevel = ((11-feedlevel*0)/11)*100;
  }
  else if (feedlevel == 10) {
    feedlevel = ((11-feedlevel*0.5)/11)*100;
  }
  else if (feedlevel >= 11) {
    feedlevel = ((11-feedlevel)/11)*100;      
  }

  // Measure NH4 levels
  int val1 = analogRead(MQ135_PIN);
  int val = 74;  
  float resistance = ((1024/(float)val) * 5 - 1)* RLOAD; 
  float r0NH4 = resistance * pow((atmNH4/scaleFactorNH4), (1./exponentNH4));
  float NH4=scaleFactorNH4 * pow((resistance/r0NH4), -exponentNH4);

  // convert the sensor value to a water level percentage
  int waterlevel = map(sensorValue, 0, 1023, 0, 100);

  // Check the state of the push button
  int buttonState = digitalRead(PUSH_BUTTON_PIN);

   if (digitalRead(bt_set) == 0) {
    if (flag == 0) {
      flag = 1;
      Set = Set + 1;
      if (Set > 7) {
        Set = 0; 
        flash = 0;
      } 
      delay(200);
    }
  } else {
    flag = 0;
  }

  if (digitalRead(bt_up) == 0) {
    if (Set == 2) { TEMP_THRESHOLD_HIGH_BRUDER += 0.1; EEPROM.put(10, TEMP_THRESHOLD_HIGH_BRUDER); }
    if (Set == 3) { TEMP_THRESHOLD_LOW_BRUDER += 0.1; EEPROM.put(20, TEMP_THRESHOLD_LOW_BRUDER); }
    if (Set == 4) { HUM_THRESHOLD_BRUDER += 1; EEPROM.put(30, HUM_THRESHOLD_BRUDER); }
    if (Set == 5) { TEMP_THRESHOLD_HIGH_INCUBATOR += 0.1; EEPROM.put(40, TEMP_THRESHOLD_HIGH_INCUBATOR); }
    if (Set == 6) { HUM_THRESHOLD_INCUBATOR += 1; EEPROM.put(50, HUM_THRESHOLD_INCUBATOR); }
    delay(10);
  }

  if (digitalRead(bt_down) == 0) {
    if (Set == 2) { TEMP_THRESHOLD_HIGH_BRUDER -= 0.1; EEPROM.put(10, TEMP_THRESHOLD_HIGH_BRUDER); }
    if (Set == 3) { TEMP_THRESHOLD_LOW_BRUDER -= 0.1; EEPROM.put(20, TEMP_THRESHOLD_LOW_BRUDER); }
    if (Set == 4) { HUM_THRESHOLD_BRUDER -= 1; EEPROM.put(30, HUM_THRESHOLD_BRUDER); }
    if (Set == 5) { TEMP_THRESHOLD_HIGH_INCUBATOR -= 0.1; EEPROM.put(40, TEMP_THRESHOLD_HIGH_INCUBATOR); }
    if (Set == 6) { HUM_THRESHOLD_INCUBATOR -= 1; EEPROM.put(50, HUM_THRESHOLD_INCUBATOR); }
    delay(10);
  }

  if (Set == 1) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set Threshold");
    lcd.clear();
  } else if (Set == 2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("B Hi Temp: ");
    if (flash == 1) { lcd.print("            "); }
    else { lcd.print(TEMP_THRESHOLD_HIGH_BRUDER, 1); lcd.print("C"); }
  } else if (Set == 3) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("B Low Temp: ");
    if (flash == 1) { lcd.print("            "); }
    else { lcd.print(TEMP_THRESHOLD_LOW_BRUDER, 1); lcd.print("C"); }
  } else if (Set == 4) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("B Hum: ");
    if (flash == 1) { lcd.print("            "); }
    else { lcd.print(HUM_THRESHOLD_BRUDER); lcd.print("%"); }
  } else if (Set == 5) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("I Hi Temp: ");
    if (flash == 1) { lcd.print("            "); }
    else { lcd.print(TEMP_THRESHOLD_HIGH_INCUBATOR, 1); lcd.print("C"); }
  } else if (Set == 6) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("I Hum: ");
    if (flash == 1) { lcd.print("            "); }
    else { lcd.print(HUM_THRESHOLD_INCUBATOR); lcd.print("%"); }
  } else if (Set == 7){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Set Complete");
    lcd.clear();
  } else if (Set == 0){
      if (buttonState == LOW) {
      // Turn off the Egg Roller
      digitalWrite(egg_Roller, HIGH);
      // Manage Bruder functionalities
      manageBruderFunctionalities(temperature, humidity, waterlevel, feedlevel, NH4);
      displayBruderMode(temperature, humidity, waterlevel, feedlevel, NH4);
    } else {
      // Turn off the fan and water pump
      digitalWrite(FAN_PIN, HIGH);
      digitalWrite(motorPin, HIGH);
      // Manage Incubator functionalities
      manageIncubatorFunctionalities(temperature, humidity, waterlevel);
      displayIncubatorMode(temperature, humidity);
    }
  }
  if (Set > 0) {
    flash = !flash;
  }

  delay(1); // Wait 1 ms for next loop iteration

  // Print the values to the serial monitor
  Serial.print("\n");
  Serial.print(buttonState);
  Serial.print(" ");
  Serial.print(temperature);
  Serial.print(" ");
  Serial.print(humidity);
  Serial.print(" ");
  Serial.print(waterlevel);
  Serial.print(" ")-
  Serial.print(feedlevel);
  Serial.print(" ");
  Serial.print(NH4);
  Serial.print(" ");
  Serial.print(TEMP_THRESHOLD_HIGH_BRUDER);
  Serial.print(" ");
  Serial.print(TEMP_THRESHOLD_LOW_BRUDER);
  Serial.print(" ");
  Serial.print(HUM_THRESHOLD_BRUDER);
  Serial.print(" ");
  Serial.print(TEMP_THRESHOLD_HIGH_INCUBATOR);
  Serial.print(" ")-
  Serial.print(HUM_THRESHOLD_INCUBATOR);

}

void displayDateTimeOnLCD() {
  // Read and display the current date and time from the RTC module
  rtc.refresh();
  lcd.setCursor(0, 0);
  lcd.print("Date: ");
  lcd.print(rtc.year());
  lcd.print("/");
  lcd.print(rtc.month());
  lcd.print("/");
  lcd.print(rtc.day());
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(rtc.hour());
  lcd.print(":");
  lcd.print(rtc.minute());
  lcd.print(":");
  lcd.print(rtc.second());
  delay(2000);
  lcd.clear();
} 

void displayBruderMode(float temperature, float humidity, int waterlevel, int feedlevel, float NH4) {
    lcd.clear();
    // Display the current date and time on the LCD screen
    displayDateTimeOnLCD();
    lcd.clear();
    // Bruder mode
    lcd.setCursor(0, 0);
    lcd.print("Bruder Mode:");
    lcd.setCursor(0, 1);
    lcd.print("Working");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print("*C");
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(humidity);
    lcd.print("%");
    delay(2000);
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Water Level: ");
    lcd.print(waterlevel);
    lcd.print("%");
    lcd.setCursor(0, 1);
    lcd.print("Feed Level: ");
    lcd.print(feedlevel);
    lcd.print("%");
    delay(2000);
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("NH4: ");
    lcd.print(NH4);
    lcd.print("ppm");
    delay(2000);
    lcd.clear();
}

void displayIncubatorMode(float temperature, float humidity) {
    lcd.clear();
    // Display the current date and time on the LCD screen
    displayDateTimeOnLCD();
    lcd.clear();
    // Incubator mode
    lcd.setCursor(0, 0);
    lcd.print("Incubator Mode:");
    lcd.setCursor(0, 1);
    lcd.print("Working");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print("*C");
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(humidity);
    lcd.print("%");
    delay(2000);
    lcd.clear();
}

void manageBruderFunctionalities(float temperature, float humidity, int waterlevel, int feedlevel, float NH4) {
  // Control the fan based on the temperature
  if (temperature <= TEMP_THRESHOLD_HIGH_BRUDER) {
    digitalWrite(FAN_PIN, HIGH);  // Turn on the fan
  } else {
    digitalWrite(FAN_PIN, LOW);   // Turn off the fan
  }

  // Control the bulb based on the temperature
  if (temperature >= TEMP_THRESHOLD_LOW_BRUDER) {
    digitalWrite(BULB_PIN, HIGH);  // Turn on the bulb
  } else {
    digitalWrite(BULB_PIN, LOW);   // Turn off the bulb
  }

  // Control the Humidifier based on the Humidity
  if (humidity >= HUM_THRESHOLD_BRUDER) {
    digitalWrite(Hum_PIN, HIGH);   // Turn on the humidifier
  } else {
    digitalWrite(Hum_PIN, LOW);    // Turn off the humidifier
  }

  // Start the servo motor if the feed level is less than 20%
  if (feedlevel < 50) { 
    servo.write(20);  // Rotate the servo to the 90 degree position
    delay(5000);
    servo.write(160); // Rotate the servo back to the 0 degree position
  }else {
    servo.write(160); // Rotate the servo back to the 0 degree position
  }

  // Activate the water pump if water level falls below 30%
  if (waterlevel >= 0) {
    digitalWrite(motorPin, HIGH);  // Turn on the water pump
  } else {
    digitalWrite(motorPin, LOW);   // Turn off the water pump
  }
}

void manageIncubatorFunctionalities(float temperature, float humidity, int waterlevel) {
  // Control the bulb based on the temperature
  if (temperature >= TEMP_THRESHOLD_HIGH_INCUBATOR) {
    digitalWrite(BULB_PIN, HIGH);  // Turn on the bulb
  } else {
    digitalWrite(BULB_PIN, LOW);   // Turn off the bulb
  }

  // Control the Humidifier based on the Humidity
  if (humidity >= HUM_THRESHOLD_INCUBATOR) {
    digitalWrite(Hum_PIN, HIGH);   // Turn on the humidifier
  } else {
    digitalWrite(Hum_PIN, LOW);    // Turn off the humidifier
  }

  // Activate the egg roller 3 times a day for 5 seconds each
  unsigned long currentTime = millis();
  int currentHour = currentTime / 3600000; // Get current hour

  // Check if it's time to activate the egg roller
  if ((currentHour == 8 || currentHour == 12 || currentHour == 16) && rollerCount < ROLLER_COUNT_MAX) {
    digitalWrite(egg_Roller, HIGH); // Egg roller on in Incubator mode
    delay(5000); // Keep the egg roller activated for 5 seconds
    digitalWrite(egg_Roller, LOW); // Turn off the egg roller
    rollerCount++; // Increment roller count
  } else if (currentHour == 0) {
    // Reset the roller count at midnight
    rollerCount = 0;
  }
}