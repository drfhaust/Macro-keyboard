![image of the macro keyboard](https://github.com/drfhaust/Macro-keyboard/blob/main/IMG_3720.JPG)
# Macro Keyboard

**Macro Keyboard with ESP32-S3**

This repository contains the design and code for building a custom macro keyboard powered by the ESP32-S3 microcontroller.

## Overview

- **Design**: A macro keyboard with customizable keys and LEDs for different functionalities.
- **Microcontroller**: Powered by the ESP32-S3 with a built-in touch screen and additional hardware components.
- **Customization**: Supports custom key mappings and macros via a JSON configuration.
- **Hardware Design**: The keyboard's hardware is modular, making it adaptable for various input needs.

## Hardware Specifications

- **Microcontroller**: ESP32-S3 with built-in wireless connectivity.
- **Key Components**: Uses Cherry MX-compatible key switches.
- **LEDs**: Individually addressable RGB LEDs for customizable lighting.
- **Screen**: Touch-enabled screen for additional control and feedback.
- **Battery**: Supports external power, and can be adapted for battery-powered use.
- **Charging**: USB Type-C port for charging and data transfer.
- **Connection**: Supports both wired and wireless communication with a host device.

## Software Features

- **Macro Management**: The macros can be configured through the `button.json` file, where each button is assigned a specific action or macro.
- **LED Control**: RGB LEDs can be customized for each button based on the configuration.
- **Key Mapping**: Easily programmable for various applications, from simple shortcuts to complex multi-step macros.
- **Python Integration**: Python scripts are provided to manage macro configurations and interface with the hardware.

## Power Details

- **Power Input**: Powered through USB Type-C or battery.
- **Power Consumption**: Optimized for low power consumption during idle states.

## Repository Structure

- **Arduino code**: Contains the LVGL porting for the ESP32-S3 along with the Arduino source code for the project.
  - `lvgl_Porting`: Core source files for integrating LVGL (LittlevGL) with the ESP32-S3.
  
- **Macro keyboard**: Includes hardware designs, PCB schematics, and component configurations.
  - `GERBER`: Files for manufacturing the PCB.
  - `custom components`: KiCad components for custom parts used in the keyboard.
  - `backups`: Backup files for the PCB designs.

- **python code**: Python scripts to manage macros and configure button functionalities.
  - `macro_manager_v4.py`: Python script for managing the macro functions of the keyboard.
  
- **solidwork 3d**: 3D models of the keyboard parts, including key switches and case designs.
  
- **button.json**: Configuration file where button mappings and LED colors are stored.

## Getting Started

1. **Clone this repository**:
   ```bash
   git clone <repository-url>
   ```

2. **Hardware Setup**: 
   - Flash the ESP32-S3 with the code in `Arduino code/lvgl_Porting`.
   - Assemble the PCB and components according to the schematics in the `Macro keyboard` folder.

3. **Software Setup**:
   - Install the required Python libraries by running:
     ```bash
     pip install -r requirements.txt
     ```

4. **Macro Management**:
   - Configure macros by editing `button.json`.
   - Run the macro manager script to apply your changes:
     ```bash
     python python code/v4/macro_manager_v4.py
     ```

## Contributing

Please read the contribution guidelines before making any changes.

## License

This project is licensed under the MIT License.
