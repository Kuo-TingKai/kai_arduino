/*
 * Interactive Arduino Program
 * This program creates a simple command-line interface through serial communication
 * You can send commands to control the Arduino and get responses
 */

#include <EEPROM.h>
#include <Wire.h>

String inputString = "";         // String to hold incoming data
boolean stringComplete = false;  // Whether the string is complete
int ledPin = 13;                 // Built-in LED pin

void setup() {
  // Initialize serial communication at 9600 baud rate
  Serial.begin(9600);
  
  // Initialize the LED pin as output
  pinMode(ledPin, OUTPUT);
  
  // Reserve 100 bytes for the inputString
  inputString.reserve(100);
  
  // Print welcome message using F() macro to save SRAM
  Serial.println(F("================================="));
  Serial.println(F("Arduino Interactive Console v2.0"));
  Serial.println(F("================================="));
  Serial.println(F("Type 'help' for commands"));
  Serial.println(F("================================="));
  Serial.print(F("Arduino> "));
}

void loop() {
  // Check if a complete command has been received
  if (stringComplete) {
    processCommand(inputString);
    
    // Clear the string for next command
    inputString = "";
    stringComplete = false;
    Serial.print("Arduino> ");
  }
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
  else if (command == "led on") {
    digitalWrite(ledPin, HIGH);
    Serial.println("LED turned ON");
  }
  else if (command == "led off") {
    digitalWrite(ledPin, LOW);
    Serial.println("LED turned OFF");
  }
  else if (command == "blink") {
    blinkLED();
  }
  else if (command == "info") {
    showSystemInfo();
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
  else if (command == "memory") {
    showDetailedMemory();
  }
  else if (command == "uptime") {
    showDetailedUptime();
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
  else if (command == "reset") {
    Serial.println("Performing software reset...");
    delay(1000);
    asm volatile ("  jmp 0"); // Software reset
  }
  else if (command == "") {
    // Empty command, do nothing
  }
  else {
    Serial.println("Unknown command: " + command);
    Serial.println("Type 'help' for available commands");
  }
}

void showHelp() {
  Serial.println(F("Basic: help, status, info, memory, uptime, reset"));
  Serial.println(F("LED: led on/off, blink"));
  Serial.println(F("I/O: pins, analog, temp"));
  Serial.println(F("PWM: pwm <pin> <val>"));
  Serial.println(F("Audio: tone <pin> <freq>, notone <pin>"));
  Serial.println(F("Storage: eeprom read/write <addr> [val]"));
  Serial.println(F("Tools: scan"));
}

void showStatus() {
  Serial.println(F("=== Arduino Status ==="));
  Serial.print(F("Uptime: "));
  Serial.print(millis() / 1000);
  Serial.println(F(" seconds"));
  
  Serial.print(F("LED Status: "));
  Serial.println(digitalRead(ledPin) ? F("ON") : F("OFF"));
  
  Serial.print(F("Free RAM: "));
  Serial.print(getFreeRam());
  Serial.println(F(" bytes"));
}

void showSystemInfo() {
  Serial.println(F("=== System Information ==="));
  Serial.println(F("Board: Arduino Compatible"));
  Serial.println(F("MCU: ATmega328P, 16MHz"));
  Serial.println(F("Memory: 32KB Flash, 2KB SRAM, 1KB EEPROM"));
  Serial.print(F("Firmware: Interactive Console v2.0 - "));
  Serial.print(F(__DATE__));
  Serial.print(F(" "));
  Serial.println(F(__TIME__));
}

void blinkLED() {
  Serial.println("Blinking LED 5 times...");
  for (int i = 0; i < 5; i++) {
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
    Serial.print(".");
  }
  Serial.println(" Done!");
}

int getFreeRam() {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void showPinStatus() {
  Serial.println("=== Digital Pin Status ===");
  for (int pin = 2; pin <= 13; pin++) {
    Serial.print("Pin ");
    Serial.print(pin);
    Serial.print(": ");
    Serial.println(digitalRead(pin) ? "HIGH" : "LOW");
  }
}

void readAnalogPins() {
  Serial.println("=== Analog Pin Readings ===");
  for (int pin = 0; pin <= 5; pin++) {
    int value = analogRead(pin);
    float voltage = value * (5.0 / 1023.0);
    Serial.print("A");
    Serial.print(pin);
    Serial.print(": ");
    Serial.print(value);
    Serial.print(" (");
    Serial.print(voltage, 2);
    Serial.println("V)");
  }
}

void readTemperature() {
  // Read temperature from A0 (assuming TMP36 sensor)
  int tempReading = analogRead(0);
  float voltage = tempReading * (5.0 / 1023.0);
  float temperatureC = (voltage - 0.5) * 100;
  float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
  
  Serial.println("=== Temperature Reading (A0) ===");
  Serial.print("Raw ADC: ");
  Serial.println(tempReading);
  Serial.print("Voltage: ");
  Serial.print(voltage, 3);
  Serial.println("V");
  Serial.print("Temperature: ");
  Serial.print(temperatureC, 1);
  Serial.print("°C (");
  Serial.print(temperatureF, 1);
  Serial.println("°F)");
  Serial.println("Note: Assumes TMP36 sensor on A0");
}

void showDetailedMemory() {
  Serial.println("=== Detailed Memory Information ===");
  
  extern int __heap_start, *__brkval;
  int v;
  int freeMemory = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
  
  Serial.print("Free SRAM: ");
  Serial.print(freeMemory);
  Serial.println(" bytes");
  
  Serial.print("Used SRAM: ");
  Serial.print(2048 - freeMemory);
  Serial.println(" bytes");
  
  Serial.print("Total SRAM: 2048 bytes");
  Serial.print(" (");
  Serial.print((float)(2048 - freeMemory) / 2048 * 100, 1);
  Serial.println("% used)");
  
  Serial.println("Flash Memory: 32768 bytes");
  Serial.println("EEPROM: 1024 bytes");
}

void showDetailedUptime() {
  unsigned long ms = millis();
  unsigned long seconds = ms / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  
  Serial.println("=== Detailed Uptime ===");
  Serial.print("Total milliseconds: ");
  Serial.println(ms);
  
  Serial.print("Uptime: ");
  if (days > 0) {
    Serial.print(days);
    Serial.print(" days, ");
  }
  if (hours % 24 > 0 || days > 0) {
    Serial.print(hours % 24);
    Serial.print(" hours, ");
  }
  if (minutes % 60 > 0 || hours > 0) {
    Serial.print(minutes % 60);
    Serial.print(" minutes, ");
  }
  Serial.print(seconds % 60);
  Serial.println(" seconds");
  
  // Calculate approximate power consumption
  float powerConsumption = (ms / 1000.0) * 0.02; // Assume 20mA average
  Serial.print("Estimated power consumption: ");
  Serial.print(powerConsumption, 2);
  Serial.println(" mAh");
}

void handlePWM(String command) {
  int firstSpace = command.indexOf(' ');
  int secondSpace = command.indexOf(' ', firstSpace + 1);
  
  if (firstSpace == -1 || secondSpace == -1) {
    Serial.println("Usage: pwm <pin> <value>");
    Serial.println("Example: pwm 9 128");
    return;
  }
  
  int pin = command.substring(firstSpace + 1, secondSpace).toInt();
  int value = command.substring(secondSpace + 1).toInt();
  
  if (pin != 3 && pin != 5 && pin != 6 && pin != 9 && pin != 10 && pin != 11) {
    Serial.println("Error: PWM only available on pins 3, 5, 6, 9, 10, 11");
    return;
  }
  
  if (value < 0 || value > 255) {
    Serial.println("Error: PWM value must be 0-255");
    return;
  }
  
  analogWrite(pin, value);
  Serial.print("PWM pin ");
  Serial.print(pin);
  Serial.print(" set to ");
  Serial.print(value);
  Serial.print(" (");
  Serial.print((value / 255.0) * 100, 1);
  Serial.println("%)");
}


void handleTone(String command) {
  int firstSpace = command.indexOf(' ');
  int secondSpace = command.indexOf(' ', firstSpace + 1);
  
  if (firstSpace == -1 || secondSpace == -1) {
    Serial.println("Usage: tone <pin> <frequency>");
    Serial.println("Example: tone 8 440");
    return;
  }
  
  int pin = command.substring(firstSpace + 1, secondSpace).toInt();
  int frequency = command.substring(secondSpace + 1).toInt();
  
  if (frequency < 31 || frequency > 65535) {
    Serial.println("Error: Frequency must be 31-65535 Hz");
    return;
  }
  
  tone(pin, frequency);
  Serial.print("Tone ");
  Serial.print(frequency);
  Serial.print(" Hz generated on pin ");
  Serial.println(pin);
}

void handleNoTone(String command) {
  int spacePos = command.indexOf(' ');
  if (spacePos == -1) {
    Serial.println("Usage: notone <pin>");
    return;
  }
  
  int pin = command.substring(spacePos + 1).toInt();
  noTone(pin);
  Serial.print("Tone stopped on pin ");
  Serial.println(pin);
}

void handleEEPROM(String command) {
  int firstSpace = command.indexOf(' ');
  if (firstSpace == -1) {
    Serial.println("Usage: eeprom read <addr> or eeprom write <addr> <value>");
    return;
  }
  
  String operation = command.substring(firstSpace + 1);
  
  if (operation.startsWith("read ")) {
    int addr = operation.substring(5).toInt();
    if (addr < 0 || addr >= EEPROM.length()) {
      Serial.print("Error: Address must be 0-");
      Serial.println(EEPROM.length() - 1);
      return;
    }
    
    byte value = EEPROM.read(addr);
    Serial.print("EEPROM[");
    Serial.print(addr);
    Serial.print("] = ");
    Serial.print(value);
    Serial.print(" (0x");
    Serial.print(value, HEX);
    Serial.println(")");
  }
  else if (operation.startsWith("write ")) {
    int secondSpace = operation.indexOf(' ', 6);
    if (secondSpace == -1) {
      Serial.println("Usage: eeprom write <addr> <value>");
      return;
    }
    
    int addr = operation.substring(6, secondSpace).toInt();
    int value = operation.substring(secondSpace + 1).toInt();
    
    if (addr < 0 || addr >= EEPROM.length()) {
      Serial.print("Error: Address must be 0-");
      Serial.println(EEPROM.length() - 1);
      return;
    }
    
    if (value < 0 || value > 255) {
      Serial.println("Error: Value must be 0-255");
      return;
    }
    
    EEPROM.write(addr, value);
    Serial.print("EEPROM[");
    Serial.print(addr);
    Serial.print("] = ");
    Serial.println(value);
  }
  else {
    Serial.println("Usage: eeprom read <addr> or eeprom write <addr> <value>");
  }
}


void scanI2C() {
  Serial.println("=== I2C Device Scanner ===");
  Wire.begin();
  
  byte error, address;
  int devices = 0;
  
  Serial.println("Scanning I2C bus (0x03-0x77)...");
  
  for (address = 3; address < 120; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println();
      devices++;
    }
    else if (error == 4) {
      Serial.print("Unknown error at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  
  if (devices == 0) {
    Serial.println("No I2C devices found");
  } else {
    Serial.print("Found ");
    Serial.print(devices);
    Serial.println(" device(s)");
  }
}


// Serial event occurs whenever new data comes in the serial RX
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
    // Add character to input string
    if (inChar == '\n' || inChar == '\r') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
}