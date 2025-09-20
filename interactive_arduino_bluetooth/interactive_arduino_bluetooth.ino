/*
 * Interactive Arduino Console with Bluetooth Support
 * This program creates a command-line interface through both USB serial and Bluetooth
 * You can send commands to control the Arduino and get responses wirelessly
 */

#include <EEPROM.h>
#include <Wire.h>
#include <SoftwareSerial.h>

// Create a SoftwareSerial object for Bluetooth communication
SoftwareSerial bluetooth(2, 3); // RX pin 2, TX pin 3

String inputString = "";         // String to hold incoming data
boolean stringComplete = false;  // Whether the string is complete
int ledPin = 13;                 // Built-in LED pin
bool useUSB = true;              // Flag to determine output method
bool useBluetooth = true;        // Flag for Bluetooth output

void setup() {
  // Initialize USB serial communication at 9600 baud rate
  Serial.begin(9600);
  
  // Initialize Bluetooth serial communication at 9600 baud rate
  bluetooth.begin(9600);
  
  // Initialize the LED pin as output
  pinMode(ledPin, OUTPUT);
  
  // Reserve 100 bytes for the inputString
  inputString.reserve(100);
  
  // Print welcome message to both USB and Bluetooth
  printWelcomeMessage();
  
  // Brief LED blink to indicate startup
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
}

void loop() {
  // Check for USB Serial input
  if (Serial.available()) {
    useUSB = true;
    useBluetooth = false;
    while (Serial.available()) {
      char inChar = (char)Serial.read();
      if (inChar == '\n' || inChar == '\r') {
        if (inputString.length() > 0) {
          processCommand(inputString);
          inputString = "";
        }
        printPrompt();
      } else {
        inputString += inChar;
      }
    }
  }
  
  // Check for Bluetooth input
  if (bluetooth.available()) {
    useUSB = false;
    useBluetooth = true;
    while (bluetooth.available()) {
      char inChar = (char)bluetooth.read();
      if (inChar == '\n' || inChar == '\r') {
        if (inputString.length() > 0) {
          processCommand(inputString);
          inputString = "";
        }
        printPrompt();
      } else {
        inputString += inChar;
      }
    }
  }
}

void printToOutput(const String& message) {
  if (useUSB) {
    Serial.print(message);
  }
  if (useBluetooth) {
    bluetooth.print(message);
  }
}

void printToOutput(const __FlashStringHelper* message) {
  if (useUSB) {
    Serial.print(message);
  }
  if (useBluetooth) {
    bluetooth.print(message);
  }
}

void printlnToOutput(const String& message) {
  if (useUSB) {
    Serial.println(message);
  }
  if (useBluetooth) {
    bluetooth.println(message);
  }
}

void printlnToOutput(const __FlashStringHelper* message) {
  if (useUSB) {
    Serial.println(message);
  }
  if (useBluetooth) {
    bluetooth.println(message);
  }
}

void printlnToOutput(int value) {
  if (useUSB) {
    Serial.println(value);
  }
  if (useBluetooth) {
    bluetooth.println(value);
  }
}

void printlnToOutput(float value, int decimals = 2) {
  if (useUSB) {
    Serial.println(value, decimals);
  }
  if (useBluetooth) {
    bluetooth.println(value, decimals);
  }
}

void printWelcomeMessage() {
  // Send to both USB and Bluetooth
  Serial.println(F("================================="));
  Serial.println(F("Arduino Bluetooth Console v2.0"));
  Serial.println(F("================================="));
  Serial.println(F("USB Serial: Ready"));
  Serial.println(F("Bluetooth: Ready (Pair: 1234)"));
  Serial.println(F("Type 'help' for commands"));
  Serial.println(F("================================="));
  Serial.print(F("Arduino> "));
  
  bluetooth.println(F("================================="));
  bluetooth.println(F("Arduino Bluetooth Console v2.0"));
  bluetooth.println(F("================================="));
  bluetooth.println(F("Bluetooth Connected Successfully!"));
  bluetooth.println(F("Type 'help' for commands"));
  bluetooth.println(F("================================="));
  bluetooth.print(F("Arduino> "));
}

void printPrompt() {
  printToOutput(F("Arduino> "));
}

void processCommand(String command) {
  command.trim(); // Remove whitespace
  command.toLowerCase(); // Convert to lowercase
  
  if (command == "help") {
    showHelp();
  }
  else if (command == "status") {
    showStatus();
  }
  else if (command == "info") {
    showSystemInfo();
  }
  else if (command == "memory") {
    showDetailedMemory();
  }
  else if (command == "uptime") {
    showDetailedUptime();
  }
  else if (command == "led on") {
    digitalWrite(ledPin, HIGH);
    printlnToOutput(F("LED turned ON"));
  }
  else if (command == "led off") {
    digitalWrite(ledPin, LOW);
    printlnToOutput(F("LED turned OFF"));
  }
  else if (command == "blink") {
    blinkLED();
  }
  else if (command == "pins") {
    showPinStatus();
  }
  else if (command == "analog") {
    readAnalogPins();
  }
  else if (command == "temp") {
    readTemperature();
  }
  else if (command.startsWith("pwm ")) {
    handlePWM(command);
  }
  else if (command.startsWith("tone ")) {
    handleTone(command);
  }
  else if (command.startsWith("notone ")) {
    handleNoTone(command);
  }
  else if (command.startsWith("eeprom ")) {
    handleEEPROM(command);
  }
  else if (command == "scan") {
    scanI2C();
  }
  else if (command == "bluetooth") {
    showBluetoothInfo();
  }
  else if (command == "reset") {
    printlnToOutput(F("Performing software reset..."));
    delay(1000);
    asm volatile ("  jmp 0"); // Software reset
  }
  else if (command == "") {
    // Empty command, do nothing
  }
  else {
    printlnToOutput("Unknown command: " + command);
    printlnToOutput(F("Type 'help' for available commands"));
  }
}

void showHelp() {
  printlnToOutput(F("Basic: help, status, info, memory, uptime, reset"));
  printlnToOutput(F("LED: led on/off, blink"));
  printlnToOutput(F("I/O: pins, analog, temp"));
  printlnToOutput(F("PWM: pwm <pin> <val>"));
  printlnToOutput(F("Audio: tone <pin> <freq>, notone <pin>"));
  printlnToOutput(F("Storage: eeprom read/write <addr> [val]"));
  printlnToOutput(F("Tools: scan, bluetooth"));
}

void showStatus() {
  printlnToOutput(F("=== Arduino Status ==="));
  printToOutput(F("Uptime: "));
  printToOutput(String(millis() / 1000));
  printlnToOutput(F(" seconds"));
  
  printToOutput(F("LED Status: "));
  printlnToOutput(digitalRead(ledPin) ? F("ON") : F("OFF"));
  
  printToOutput(F("Free RAM: "));
  printToOutput(String(getFreeRam()));
  printlnToOutput(F(" bytes"));
  
  printToOutput(F("Connection: "));
  if (useUSB) printToOutput(F("USB "));
  if (useBluetooth) printToOutput(F("Bluetooth"));
  printlnToOutput(F(""));
}

void showSystemInfo() {
  printlnToOutput(F("=== System Information ==="));
  printlnToOutput(F("Board: Arduino Uno"));
  printlnToOutput(F("MCU: ATmega328P, 16MHz"));
  printlnToOutput(F("Memory: 32KB Flash, 2KB SRAM, 1KB EEPROM"));
  printlnToOutput(F("Bluetooth: HC-05/HC-06 Compatible"));
  printToOutput(F("Firmware: Bluetooth Console v2.0 - "));
  printToOutput(F(__DATE__));
  printToOutput(F(" "));
  printlnToOutput(F(__TIME__));
}

void showBluetoothInfo() {
  printlnToOutput(F("=== Bluetooth Information ==="));
  printlnToOutput(F("Module: HC-05/HC-06 Compatible"));
  printlnToOutput(F("ID: LTC6739"));
  printlnToOutput(F("Default Password: 1234"));
  printlnToOutput(F("Baud Rate: 9600"));
  printlnToOutput(F("Pins: RX=D2, TX=D3"));
  printlnToOutput(F("Status: Connected"));
}

void blinkLED() {
  printlnToOutput(F("Blinking LED 5 times..."));
  for (int i = 0; i < 5; i++) {
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
    printToOutput(F("."));
  }
  printlnToOutput(F(" Done!"));
}

void showPinStatus() {
  printlnToOutput(F("=== Digital Pin Status ==="));
  for (int pin = 4; pin <= 13; pin++) { // Skip pins 2,3 used for Bluetooth
    printToOutput(F("Pin "));
    printToOutput(String(pin));
    printToOutput(F(": "));
    printlnToOutput(digitalRead(pin) ? F("HIGH") : F("LOW"));
  }
}

void readAnalogPins() {
  printlnToOutput(F("=== Analog Pin Readings ==="));
  for (int pin = 0; pin <= 5; pin++) {
    int value = analogRead(pin);
    float voltage = value * (5.0 / 1023.0);
    printToOutput(F("A"));
    printToOutput(String(pin));
    printToOutput(F(": "));
    printToOutput(String(value));
    printToOutput(F(" ("));
    printToOutput(String(voltage, 2));
    printlnToOutput(F("V)"));
  }
}

void readTemperature() {
  int tempReading = analogRead(0);
  float voltage = tempReading * (5.0 / 1023.0);
  float temperatureC = (voltage - 0.5) * 100;
  float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
  
  printlnToOutput(F("=== Temperature Reading (A0) ==="));
  printToOutput(F("Raw ADC: "));
  printlnToOutput(tempReading);
  printToOutput(F("Voltage: "));
  printToOutput(String(voltage, 3));
  printlnToOutput(F("V"));
  printToOutput(F("Temperature: "));
  printToOutput(String(temperatureC, 1));
  printToOutput(F("°C ("));
  printToOutput(String(temperatureF, 1));
  printlnToOutput(F("°F)"));
  printlnToOutput(F("Note: Assumes TMP36 sensor on A0"));
}

void showDetailedMemory() {
  printlnToOutput(F("=== Detailed Memory Information ==="));
  
  extern int __heap_start, *__brkval;
  int v;
  int freeMemory = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
  
  printToOutput(F("Free SRAM: "));
  printToOutput(String(freeMemory));
  printlnToOutput(F(" bytes"));
  
  printToOutput(F("Used SRAM: "));
  printToOutput(String(2048 - freeMemory));
  printlnToOutput(F(" bytes"));
  
  printToOutput(F("Total SRAM: 2048 bytes ("));
  printToOutput(String((float)(2048 - freeMemory) / 2048 * 100, 1));
  printlnToOutput(F("% used)"));
  
  printlnToOutput(F("Flash Memory: 32768 bytes"));
  printlnToOutput(F("EEPROM: 1024 bytes"));
}

void showDetailedUptime() {
  unsigned long ms = millis();
  unsigned long seconds = ms / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  
  printlnToOutput(F("=== Detailed Uptime ==="));
  printToOutput(F("Total milliseconds: "));
  printlnToOutput(String(ms));
  
  printToOutput(F("Uptime: "));
  if (days > 0) {
    printToOutput(String(days));
    printToOutput(F(" days, "));
  }
  if (hours % 24 > 0 || days > 0) {
    printToOutput(String(hours % 24));
    printToOutput(F(" hours, "));
  }
  if (minutes % 60 > 0 || hours > 0) {
    printToOutput(String(minutes % 60));
    printToOutput(F(" minutes, "));
  }
  printToOutput(String(seconds % 60));
  printlnToOutput(F(" seconds"));
  
  float powerConsumption = (ms / 1000.0) * 0.02;
  printToOutput(F("Estimated power consumption: "));
  printToOutput(String(powerConsumption, 2));
  printlnToOutput(F(" mAh"));
}

void handlePWM(String command) {
  int firstSpace = command.indexOf(' ');
  int secondSpace = command.indexOf(' ', firstSpace + 1);
  
  if (firstSpace == -1 || secondSpace == -1) {
    printlnToOutput(F("Usage: pwm <pin> <value>"));
    printlnToOutput(F("Example: pwm 9 128"));
    return;
  }
  
  int pin = command.substring(firstSpace + 1, secondSpace).toInt();
  int value = command.substring(secondSpace + 1).toInt();
  
  if ((pin != 5 && pin != 6 && pin != 9 && pin != 10 && pin != 11) || pin == 2 || pin == 3) {
    printlnToOutput(F("Error: PWM available on pins 5,6,9,10,11 (2,3 used for Bluetooth)"));
    return;
  }
  
  if (value < 0 || value > 255) {
    printlnToOutput(F("Error: PWM value must be 0-255"));
    return;
  }
  
  analogWrite(pin, value);
  printToOutput(F("PWM pin "));
  printToOutput(String(pin));
  printToOutput(F(" set to "));
  printToOutput(String(value));
  printToOutput(F(" ("));
  printToOutput(String((value / 255.0) * 100, 1));
  printlnToOutput(F("%)"));
}

void handleTone(String command) {
  int firstSpace = command.indexOf(' ');
  int secondSpace = command.indexOf(' ', firstSpace + 1);
  
  if (firstSpace == -1 || secondSpace == -1) {
    printlnToOutput(F("Usage: tone <pin> <frequency>"));
    printlnToOutput(F("Example: tone 8 440"));
    return;
  }
  
  int pin = command.substring(firstSpace + 1, secondSpace).toInt();
  int frequency = command.substring(secondSpace + 1).toInt();
  
  if (pin == 2 || pin == 3) {
    printlnToOutput(F("Error: Pins 2,3 reserved for Bluetooth"));
    return;
  }
  
  if (frequency < 31 || frequency > 65535) {
    printlnToOutput(F("Error: Frequency must be 31-65535 Hz"));
    return;
  }
  
  tone(pin, frequency);
  printToOutput(F("Tone "));
  printToOutput(String(frequency));
  printToOutput(F(" Hz generated on pin "));
  printlnToOutput(String(pin));
}

void handleNoTone(String command) {
  int spacePos = command.indexOf(' ');
  if (spacePos == -1) {
    printlnToOutput(F("Usage: notone <pin>"));
    return;
  }
  
  int pin = command.substring(spacePos + 1).toInt();
  
  if (pin == 2 || pin == 3) {
    printlnToOutput(F("Error: Pins 2,3 reserved for Bluetooth"));
    return;
  }
  
  noTone(pin);
  printToOutput(F("Tone stopped on pin "));
  printlnToOutput(String(pin));
}

void handleEEPROM(String command) {
  int firstSpace = command.indexOf(' ');
  if (firstSpace == -1) {
    printlnToOutput(F("Usage: eeprom read <addr> or eeprom write <addr> <value>"));
    return;
  }
  
  String operation = command.substring(firstSpace + 1);
  
  if (operation.startsWith("read ")) {
    int addr = operation.substring(5).toInt();
    if (addr < 0 || addr >= EEPROM.length()) {
      printToOutput(F("Error: Address must be 0-"));
      printlnToOutput(String(EEPROM.length() - 1));
      return;
    }
    
    byte value = EEPROM.read(addr);
    printToOutput(F("EEPROM["));
    printToOutput(String(addr));
    printToOutput(F("] = "));
    printToOutput(String(value));
    printToOutput(F(" (0x"));
    printToOutput(String(value, HEX));
    printlnToOutput(F(")"));
  }
  else if (operation.startsWith("write ")) {
    int secondSpace = operation.indexOf(' ', 6);
    if (secondSpace == -1) {
      printlnToOutput(F("Usage: eeprom write <addr> <value>"));
      return;
    }
    
    int addr = operation.substring(6, secondSpace).toInt();
    int value = operation.substring(secondSpace + 1).toInt();
    
    if (addr < 0 || addr >= EEPROM.length()) {
      printToOutput(F("Error: Address must be 0-"));
      printlnToOutput(String(EEPROM.length() - 1));
      return;
    }
    
    if (value < 0 || value > 255) {
      printlnToOutput(F("Error: Value must be 0-255"));
      return;
    }
    
    EEPROM.write(addr, value);
    printToOutput(F("EEPROM["));
    printToOutput(String(addr));
    printToOutput(F("] = "));
    printlnToOutput(String(value));
  }
  else {
    printlnToOutput(F("Usage: eeprom read <addr> or eeprom write <addr> <value>"));
  }
}

void scanI2C() {
  printlnToOutput(F("=== I2C Device Scanner ==="));
  Wire.begin();
  
  byte error, address;
  int devices = 0;
  
  printlnToOutput(F("Scanning I2C bus (0x03-0x77)..."));
  
  for (address = 3; address < 120; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      printToOutput(F("I2C device found at 0x"));
      if (address < 16) printToOutput(F("0"));
      printlnToOutput(String(address, HEX));
      devices++;
    }
    else if (error == 4) {
      printToOutput(F("Unknown error at 0x"));
      if (address < 16) printToOutput(F("0"));
      printlnToOutput(String(address, HEX));
    }
  }
  
  if (devices == 0) {
    printlnToOutput(F("No I2C devices found"));
  } else {
    printToOutput(F("Found "));
    printToOutput(String(devices));
    printlnToOutput(F(" device(s)"));
  }
}

int getFreeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
