# Bluetooth Module Setup Guide for Arduino Interactive Console

This guide covers the complete setup process for integrating a Bluetooth module (HC-05/HC-06 compatible) with the Arduino Interactive Console on macOS.

## Hardware Requirements

- Arduino Uno or compatible board
- HC-05/HC-06 Bluetooth module (or compatible like LTC 6739)
- Jumper wires for connections
- macOS computer with Bluetooth capability

## Hardware Connections

Connect the Bluetooth module to Arduino Uno as follows:

| Bluetooth Module Pin | Arduino Uno Pin | Description |
|---------------------|-----------------|-------------|
| **VCC** | **5V** (or 3.3V depending on module) | Power supply positive |
| **GND** | **GND** | Power supply ground |
| **TXD** | **Digital Pin 2** | Bluetooth transmit → Arduino receive |
| **RXD** | **Digital Pin 3** | Arduino transmit → Bluetooth receive |
| **KEY** | Not connected | AT command mode (optional) |
| **STATE** | Not connected | Connection status indicator (optional) |

⚠️ **Important Notes:**
- Cross-connect TXD/RXD pins (Bluetooth TXD → Arduino Pin 2)
- Verify your module's voltage requirements (3.3V or 5V)
- Some modules may require 3.3V power supply

## Software Setup

### 1. Arduino Code Modifications

The `interactive_arduino_bluetooth.ino` includes the following key features:

- **SoftwareSerial Library**: Enables Bluetooth communication on pins 2 and 3
- **Dual Communication**: Supports both USB Serial and Bluetooth simultaneously
- **Smart Output Routing**: Automatically detects command source and responds accordingly
- **Pin Protection**: Prevents conflicts with Bluetooth-occupied pins (2 and 3)

### 2. Key Code Components

```cpp
#include <SoftwareSerial.h>

// Create SoftwareSerial object for Bluetooth
SoftwareSerial bluetooth(2, 3); // RX pin 2, TX pin 3

void setup() {
  Serial.begin(9600);      // USB Serial
  bluetooth.begin(9600);   // Bluetooth Serial
  // ... initialization code
}
```

### 3. Upload Process

1. Open `interactive_arduino_bluetooth.ino` in Arduino IDE
2. Select correct board: `Arduino Uno`
3. Select correct port: `/dev/cu.wchusbserial110` (or similar)
4. Upload the sketch
5. Verify upload success in Serial Monitor

## macOS Bluetooth Setup

### 1. Initial Pairing

1. **Open System Preferences → Bluetooth**
2. **Put Arduino in discoverable mode** (power on with Bluetooth sketch running)
3. **Search for devices** - look for "LTC 6739" or similar name
4. **Pair the device** using default password: `1234`

### 2. Establishing SPP Connection

The key issue on macOS is establishing a proper Serial Port Profile (SPP) connection:

#### Install Required Tools

```bash
# Install Bluetooth utilities
brew install blueutil

# Install serial communication tools
brew install --cask serial
brew install minicom
```

#### Create SPP Connection

```bash
# Check paired devices
blueutil --paired

# Connect to your Bluetooth module (replace with your device's MAC address)
blueutil --connect 98-d3-11-fc-7c-2d

# Verify connection
blueutil --is-connected 98-d3-11-fc-7c-2d
```

#### Verify Serial Port Creation

After successful SPP connection, check for the new serial port:

```bash
ls /dev/cu.* | grep -E "(LTC|bluetooth)" -i
```

You should see something like `/dev/cu.LTC6739` appear.

### 3. Testing Connection

#### Method 1: Using Serial.app

1. **Launch Serial.app**
2. **Select the Bluetooth device**: `/dev/cu.LTC6739` (not "Incoming Port")
3. **Configure settings**:
   - Baud Rate: `9600`
   - Data Bits: `8`
   - Parity: `None`
   - Stop Bits: `1`
4. **Connect and test**

#### Method 2: Using Terminal

```bash
# Using screen
screen /dev/cu.LTC6739 9600

# Using minicom
minicom -D /dev/cu.LTC6739 -b 9600
```

#### Method 3: Simple Command Test

```bash
# Send a command and read response
echo "help" > /dev/cu.LTC6739
```

## Available Commands

Once connected, you can use all interactive console commands:

### Basic Commands
- `help` - Show available commands
- `status` - Show Arduino status including connection type
- `info` - Show system information
- `memory` - Show memory usage
- `uptime` - Show detailed uptime
- `reset` - Software reset

### Hardware Control
- `led on/off` - Control built-in LED
- `blink` - Blink LED 5 times
- `pins` - Show digital pin status (pins 4-13, excluding 2,3)
- `analog` - Read all analog pins
- `temp` - Read temperature sensor on A0

### Advanced Features
- `pwm <pin> <value>` - Set PWM output (pins 5,6,9,10,11)
- `tone <pin> <freq>` - Generate audio tone
- `notone <pin>` - Stop audio tone
- `eeprom read <addr>` - Read EEPROM byte
- `eeprom write <addr> <value>` - Write EEPROM byte
- `scan` - Scan I2C bus for devices
- `bluetooth` - Show Bluetooth module information

## Troubleshooting

### Common Issues

1. **"Incoming Port" shows blank screen**
   - Use the specific device port (e.g., `/dev/cu.LTC6739`) instead
   - Ensure SPP connection is established with `blueutil --connect`

2. **Cannot input text in Serial.app**
   - Verify correct port selection
   - Check Hardware Flow Control is set to "No"
   - Ensure Bluetooth module is properly powered

3. **Commands not responding**
   - Verify Arduino sketch is uploaded correctly
   - Test with USB Serial Monitor first
   - Check Bluetooth module wiring (especially TXD/RXD cross-connection)

4. **Module not discoverable**
   - Ensure Arduino is powered and sketch is running
   - Check Bluetooth module power supply (3.3V vs 5V)
   - Verify wiring connections

### Verification Steps

1. **Test USB Serial first**:
   ```
   Arduino IDE → Serial Monitor → Test commands
   ```

2. **Verify Bluetooth hardware**:
   ```
   USB Serial Monitor → Type: bluetooth
   Should show: Status: Connected
   ```

3. **Check macOS Bluetooth**:
   ```bash
   system_profiler SPBluetoothDataType | grep -A 10 "LTC 6739"
   ```

4. **Verify SPP connection**:
   ```bash
   ls /dev/cu.* | grep LTC
   ```

## Automatic Connection Script

Create a script to automatically establish Bluetooth connection:

```bash
#!/bin/bash
# bluetooth_connect.sh

DEVICE_MAC="98-d3-11-fc-7c-2d"  # Replace with your device MAC
DEVICE_NAME="LTC6739"           # Replace with your device name

echo "Connecting to Bluetooth device..."
blueutil --connect $DEVICE_MAC

sleep 2

if [ $(blueutil --is-connected $DEVICE_MAC) -eq 1 ]; then
    echo "Successfully connected to $DEVICE_NAME"
    echo "Serial port available at: /dev/cu.$DEVICE_NAME"
else
    echo "Failed to connect to $DEVICE_NAME"
    exit 1
fi
```

Make it executable:
```bash
chmod +x bluetooth_connect.sh
./bluetooth_connect.sh
```

## Technical Specifications

- **Communication Protocol**: Serial over Bluetooth (SPP)
- **Baud Rate**: 9600 bps
- **Data Format**: 8-N-1 (8 data bits, no parity, 1 stop bit)
- **Default Password**: 1234
- **Operating Range**: ~10 meters (depending on module)
- **Power Consumption**: ~8mA in connected mode

## Security Considerations

- Change default Bluetooth password if possible
- Limit physical access to the Arduino device
- Consider implementing command authentication for sensitive operations
- Monitor for unauthorized connection attempts

## Performance Notes

- Bluetooth communication is slower than USB Serial
- Commands may have slight delay compared to USB connection
- Range limitations apply (typically 10 meters)
- Battery-powered operation possible with power management

---

**Created by**: Kuo Ting-Kai  
**Project**: Interactive Arduino Console with Bluetooth Support  
**Version**: 2.0  
**Last Updated**: September 2025
