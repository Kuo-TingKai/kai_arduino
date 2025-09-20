# Kai Arduino - Interactive Arduino Console

An advanced command-line interface for Arduino control and monitoring that transforms your Arduino into a powerful interactive development platform.

## 🚀 Features

### Core Functions
- **System Information**: View Arduino status, memory usage, and uptime
- **LED Control**: Control built-in LED with on/off and blink functions
- **I/O Monitoring**: Read digital pins and analog inputs
- **Temperature Sensing**: Read temperature sensors (TMP36 compatible)

### Advanced Functions  
- **PWM Control**: Set PWM output on pins 3, 5, 6, 9, 10, 11
- **Audio Generation**: Generate tones and control audio output
- **EEPROM Operations**: Read and write to Arduino's EEPROM memory
- **I2C Device Scanner**: Scan and detect I2C devices on the bus

## 📋 Available Commands

### Basic Commands
```
help      - Show available commands
status    - Show Arduino status
info      - Show system information  
memory    - Show detailed memory usage
uptime    - Show detailed uptime
reset     - Perform software reset
```

### LED Control
```
led on    - Turn on built-in LED
led off   - Turn off built-in LED
blink     - Blink LED 5 times
```

### I/O Operations
```
pins      - Show digital pin status (pins 2-13)
analog    - Read all analog pins (A0-A5)
temp      - Read temperature from A0 (assumes TMP36 sensor)
```

### Advanced Features
```
pwm <pin> <value>         - Set PWM output (0-255)
tone <pin> <frequency>    - Generate tone (31-65535 Hz)
notone <pin>              - Stop tone generation
eeprom read <addr>        - Read EEPROM byte
eeprom write <addr> <val> - Write EEPROM byte
scan                      - Scan I2C bus for devices
```

## 🔧 Hardware Requirements

- Arduino Uno or compatible board
- USB connection to computer
- Optional: TMP36 temperature sensor on A0
- Optional: I2C devices for scanning

## 📱 Installation

1. **Install Arduino IDE 2.x**
2. **Connect your Arduino** via USB
3. **Open** `interactive_arduino.ino` in Arduino IDE
4. **Select** correct board and port
5. **Upload** the sketch to your Arduino
6. **Open Serial Monitor** at 9600 baud
7. **Type** `help` to see available commands

## 💡 Usage Examples

```
Arduino> status
=== Arduino Status ===
Uptime: 120 seconds
LED Status: OFF
Free RAM: 1205 bytes

Arduino> pwm 9 128
PWM pin 9 set to 128 (50.1%)

Arduino> analog
=== Analog Pin Readings ===
A0: 512 (2.50V)
A1: 0 (0.00V)
...

Arduino> scan
=== I2C Device Scanner ===
Scanning I2C bus (0x03-0x77)...
I2C device found at 0x3C
Found 1 device(s)
```

## 🛠 Technical Details

- **Memory Optimized**: Uses F() macro to store strings in Flash memory
- **Error Handling**: Comprehensive input validation and error messages
- **Interactive**: Real-time command processing with immediate feedback
- **Extensible**: Easy to add new commands and features

## 📊 Memory Usage

- **Flash Memory**: ~15KB (optimized for Arduino Uno's 32KB)
- **SRAM**: ~1.2KB (leaves plenty for user variables)
- **EEPROM**: Available for user data storage

## 🔍 Troubleshooting

1. **Upload Issues**: Check board selection and port configuration
2. **Memory Errors**: Use Arduino Uno or board with ≥2KB SRAM
3. **Communication Issues**: Verify 9600 baud rate in Serial Monitor
4. **CH340 Driver**: Install CH340 driver for compatible boards

## 📝 Version History

- **v2.0**: Memory optimized version with core features
- **v1.0**: Initial release with full feature set

## 🤝 Contributing

Feel free to submit issues, feature requests, or pull requests to improve this interactive Arduino console.

## 📄 License

This project is open source and available under the MIT License.

---
Created by Kuo Ting-Kai | Interactive Arduino Development Platform
