#!/usr/bin/env python3
"""
Arduino Device Enumerator for macOS
Detects and lists connected Arduino devices
"""

import os
import glob
import subprocess
import json
import re
from typing import List, Dict, Optional

class ArduinoDetector:
    """Class to detect Arduino devices on macOS"""
    
    # Common Arduino USB vendor/product ID combinations
    ARDUINO_IDS = {
        '0x2341': 'Arduino',           # Official Arduino
        '0x1a86': 'CH340/CH341',       # CH340/CH341 chips (common in clones)
        '0x0403': 'FTDI',              # FTDI chips
        '0x10c4': 'Silicon Labs',      # CP210x chips
        '0x067b': 'Prolific',          # PL2303 chips
    }
    
    def __init__(self):
        """Initialize the Arduino detector"""
        pass
    
    def get_serial_ports(self) -> List[str]:
        """
        Get all available serial ports that could be Arduino devices
        Returns list of device paths
        """
        # Look for USB serial devices
        usb_devices = []
        
        # Check for common Arduino serial device patterns
        patterns = [
            '/dev/cu.usbserial*',
            '/dev/cu.usbmodem*', 
            '/dev/cu.wchusbserial*',
            '/dev/tty.usbserial*',
            '/dev/tty.usbmodem*',
            '/dev/tty.wchusbserial*'
        ]
        
        for pattern in patterns:
            usb_devices.extend(glob.glob(pattern))
        
        return sorted(list(set(usb_devices)))
    
    def get_usb_device_info(self) -> List[Dict]:
        """
        Get detailed USB device information using system_profiler
        Returns list of USB device dictionaries
        """
        try:
            # Run system_profiler to get USB device information
            result = subprocess.run(
                ['system_profiler', 'SPUSBDataType', '-json'],
                capture_output=True,
                text=True,
                check=True
            )
            
            usb_data = json.loads(result.stdout)
            arduino_devices = []
            
            def search_usb_tree(items, parent_info=None):
                """Recursively search USB device tree"""
                for item in items:
                    # Check if this could be an Arduino device
                    vendor_id = item.get('vendor_id', '').lower()
                    product_id = item.get('product_id', '').lower()
                    product_name = item.get('_name', '').lower()
                    
                    # Check for Arduino-related identifiers
                    is_arduino = (
                        vendor_id in self.ARDUINO_IDS or
                        'arduino' in product_name or
                        'serial' in product_name or
                        'ch340' in product_name or
                        'ftdi' in product_name or
                        'usb2.0-serial' in product_name
                    )
                    
                    if is_arduino:
                        device_info = {
                            'name': item.get('_name', 'Unknown'),
                            'vendor_id': item.get('vendor_id', 'Unknown'),
                            'product_id': item.get('product_id', 'Unknown'),
                            'version': item.get('bcd_device', 'Unknown'),
                            'speed': item.get('speed', 'Unknown'),
                            'location_id': item.get('location_id', 'Unknown'),
                            'manufacturer': self.ARDUINO_IDS.get(vendor_id, 'Unknown'),
                            'serial_number': item.get('serial_num', 'Unknown')
                        }
                        arduino_devices.append(device_info)
                    
                    # Recursively search child items
                    if '_items' in item:
                        search_usb_tree(item['_items'], item)
            
            # Search through USB data
            for bus in usb_data.get('SPUSBDataType', []):
                if '_items' in bus:
                    search_usb_tree(bus['_items'])
            
            return arduino_devices
            
        except (subprocess.CalledProcessError, json.JSONDecodeError, KeyError) as e:
            print(f"Error getting USB device info: {e}")
            return []
    
    def detect_arduino_devices(self) -> Dict:
        """
        Main method to detect Arduino devices
        Returns dictionary with serial ports and USB device info
        """
        print("🔍 Scanning for Arduino devices...")
        
        # Get serial ports
        serial_ports = self.get_serial_ports()
        
        # Get USB device information
        usb_devices = self.get_usb_device_info()
        
        return {
            'serial_ports': serial_ports,
            'usb_devices': usb_devices,
            'total_devices': len(serial_ports)
        }
    
    def print_device_info(self, devices: Dict):
        """Print formatted device information"""
        print("\n" + "="*60)
        print("🔌 ARDUINO DEVICE DETECTION RESULTS")
        print("="*60)
        
        if devices['total_devices'] == 0:
            print("❌ No Arduino devices found.")
            print("\n💡 Tips:")
            print("   • Make sure your Arduino is connected via USB")
            print("   • Check that the USB cable supports data transfer")
            print("   • Try a different USB port")
            return
        
        print(f"✅ Found {devices['total_devices']} potential Arduino device(s)")
        
        # Print serial ports
        print(f"\n📡 Serial Ports ({len(devices['serial_ports'])}):")
        for i, port in enumerate(devices['serial_ports'], 1):
            print(f"   {i}. {port}")
        
        # Print USB device details
        if devices['usb_devices']:
            print(f"\n🔧 USB Device Details ({len(devices['usb_devices'])}):")
            for i, device in enumerate(devices['usb_devices'], 1):
                print(f"\n   Device {i}:")
                print(f"      Name: {device['name']}")
                print(f"      Manufacturer: {device['manufacturer']}")
                print(f"      Vendor ID: {device['vendor_id']}")
                print(f"      Product ID: {device['product_id']}")
                print(f"      Version: {device['version']}")
                print(f"      Speed: {device['speed']}")
                print(f"      Location: {device['location_id']}")
                if device['serial_number'] != 'Unknown':
                    print(f"      Serial Number: {device['serial_number']}")
        
        print(f"\n💻 Usage in Arduino IDE:")
        print("   Use these ports in Arduino IDE > Tools > Port:")
        for port in devices['serial_ports']:
            if 'cu.' in port:  # Prefer cu. devices for Arduino IDE
                print(f"   • {port}")

def main():
    """Main function"""
    print("Arduino Device Enumerator for macOS")
    print("Created for detecting Arduino Uno and compatible devices")
    
    detector = ArduinoDetector()
    devices = detector.detect_arduino_devices()
    detector.print_device_info(devices)
    
    print("\n" + "="*60)

if __name__ == "__main__":
    main()
