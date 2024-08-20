# Incubator-Bruder Control and Monitoring System with IoT Integration

## Overview
This project is designed to monitor and control the environment inside an incubator-bruder system. It helps in maintaining optimal conditions for hatching and brooding poultry by regulating temperature, humidity, and feed levels. The system uses an Arduino microcontroller for hardware control and an ESP8266 module to display real-time data on the Blynk app, enabling remote monitoring.

## Features
- **Temperature Control:** Maintains the required temperature inside the incubator using a temperature sensor and a heater.
- **Humidity Control:** Monitors and adjusts humidity levels using a humidity sensor and a water atomizer or humidifier.
- **Feed and Water Level Monitoring:** Tracks and maintains feed and water levels using respective sensors.
- **IoT Integration:** Real-time data display and control through the Blynk app using the ESP8266 Wi-Fi module.
- **User Interface:** LCD display for local monitoring of temperature, humidity, and other parameters.

## Components
- **Arduino Uno** or compatible microcontroller
- **ESP8266 Wi-Fi Module** for IoT integration
- **DHT22/AM2302 Sensor** for temperature and humidity measurement
- **Water Level Sensor** for monitoring water levels
- **Feed Level Sensor** for monitoring feed levels
- **Relay Module** for controlling heaters and humidifiers
- **LCD Display** (16x2) for local display
- **DC Motor** or Servo for feed dispensing mechanism
- **Blynk App** for remote monitoring and control

## Software Requirements
- **Arduino IDE** for coding and uploading sketches to the Arduino
- **Blynk Library** for IoT integration
- **ESP8266WiFi Library** for Wi-Fi communication
- **DHT Sensor Library** for temperature and humidity sensing

## Installation and Setup
### 1. Arduino IDE Setup
1. Install the Arduino IDE.
2. Install the required libraries:
   - Blynk Library
   - ESP8266WiFi Library
   - DHT Sensor Library
3. Set up the board and port by navigating to `Tools > Board` and selecting `Arduino/Genuino Uno` or your respective board. Then, select the appropriate COM port.

### 2. Circuit Assembly
- Connect the DHT22 sensor to the Arduino pins for temperature and humidity monitoring.
- Connect the water and feed level sensors to the analog pins of the Arduino.
- Interface the ESP8266 module with the Arduino for IoT communication.
- Set up the relay module to control the heating and humidifying elements.
- Attach the LCD display to display the current status locally.

### 3. Blynk App Setup
1. Download the Blynk app.
2. Create a new project and obtain the authentication token.
3. Configure the widgets to display temperature, humidity, and other parameters.
4. Update the `Blynk.begin()` function in the Arduino code with your Wi-Fi credentials and authentication token.

### 4. Uploading the Code
- Write or download the Arduino sketch for the system.
- Upload the code to the Arduino board using the Arduino IDE.

### 5. Testing and Calibration
- Power on the system and check the local LCD display for correct temperature and humidity readings.
- Open the Blynk app to verify that data is being transmitted correctly over Wi-Fi.
- Calibrate sensors if necessary and adjust setpoints in the code for temperature and humidity control.

## Usage
- **Local Monitoring:** The LCD display shows current temperature, humidity, and system status.
- **Remote Monitoring:** The Blynk app displays real-time data and allows remote adjustments.

## Troubleshooting
- **No Data on Blynk App:** Check Wi-Fi credentials and ensure the ESP8266 is correctly connected.
- **Incorrect Sensor Readings:** Recheck connections, and consider replacing the sensor if the issue persists.
- **System Not Responding:** Reset the Arduino and ESP8266 modules, and check the power supply.

## Future Improvements
- **Automated Feed Dispensing:** Improve the feed dispensing mechanism for more precise control.
- **Advanced Data Logging:** Store data in a cloud database for historical analysis.
- **Mobile Alerts:** Implement SMS or push notifications for critical alerts (e.g., low temperature or humidity).

## License
This project is open-source and licensed under the MIT License. Feel free to use, modify, and distribute.

## Contributions
Contributions are welcome! Please fork this repository and submit a pull request with your proposed changes.

## Contact
For any questions or support, please contact Muhammad Subhan at muhammadsubhan.contact@gmail.com
