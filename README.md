# tinyFrame

**Tiny Frame for Microcontrollers with TTN Decoder Helper**

A lightweight library for microcontrollers to build and manage binary payloads, with built-in tools to generate JavaScript-style decoders compatible with The Things Network (TTN). Ideal for IoT applications like LoRaWAN where compact and efficient data handling is key.

[![GitHub](https://img.shields.io/github/license/manuelespinosa/tinyFrame)](https://github.com/manuelespinosa/tinyFrame/blob/main/LICENSE)

---

## Features

- **Simple Payload Construction**: Add 8-bit, 16-bit, and 32-bit signed/unsigned integers to a fixed-size buffer with ease.
- **TTN Decoder Helper**: Optionally prints JavaScript-like decoding instructions for TTN or similar platforms.
- **Hexadecimal Output**: Convert the payload to a human-readable hexadecimal string for debugging or transmission.
- **Lightweight**: Designed for resource-constrained microcontrollers, with minimal overhead.

---

## Installation

1. Download the latest version from [GitHub](https://github.com/manuelespinosa/tinyFrame).
2. Copy the `tinyFrame.h` file to your project folder or include it in your microcontroller development environment.
3. Include it in your code:
   ```cpp
   #include <tinyFrame.h>

--- 

## Usage
### Basic Example
```cpp
#define FRAME_SERIAL SerialUSB // Use SerialUSB instead of Serial (optional)
#include <tinyFrame.h>

struct tinyFrame frame;

void setup() {
  Serial.begin(115200);       // Initialize your serial port
  frame.printDecoder = true;  // Enable TTN decoder output for debugging
}

void loop() {
  frame.clear();              // Reset the frame
  frame.append_uint8_t(135);  // Add an 8-bit unsigned integer (135)
  frame.append_int16_t(30000); // Add a 16-bit signed integer (30000)
  frame.printHex();           // Print the frame as hex values

  // Access the raw buffer and size
  // frame.buffer: uint8_t array for transmission (e.g., LoRa)
  // frame.size(): Current frame size in bytes

  delay(10000);               // Delay in milliseconds
}
```

### Console Output
```
val_uint8 = input.bytes[0];
val_int16 = (input.bytes[1] | input.bytes[2] << 8) << 16 >> 16;
0x87 0x30 0x75
```

### Key Methods

    clear(): Resets the frame buffer and index to zero.
    append_uint8_t(uint8_t), append_uint16_t(uint16_t), append_uint32_t(uint32_t): Append unsigned integers of 8, 16, or 32 bits.
    append_int8_t(int8_t), append_int16_t(int16_t), append_int32_t(int32_t): Append signed integers of 8, 16, or 32 bits.
    printHex(): Prints the current buffer as space-separated hexadecimal values.
    hexChar(char *buff, uint16_t size): Converts the buffer to a hexadecimal string (e.g., "873075") in the provided buffer.
    size(): Returns the current number of bytes in the frame.

### Configuration Options

    printDecoder: Set to true to print TTN-compatible decoder instructions (default: false).
    printBufferBytes: Set to true to log each byte as it’s added to the buffer (default: false).
    FRAME_SERIAL: Define this macro to use a different serial port (e.g., #define FRAME_SERIAL Serial1). Defaults to Serial.

## Example with LoRaWAN
```cpp
#include <tinyFrame.h>
#include <SomeLoRaLibrary.h> // Hypothetical LoRa library

struct tinyFrame frame;
SomeLoRaClass lora;

void setup() {
  Serial.begin(115200);    // Initialize serial for debugging
  lora.begin();            // Initialize LoRa module
}

void loop() {
  frame.clear();
  frame.append_uint16_t(millis() / 60000); // Uptime in minutes
  frame.append_uint8_t(42);                // Example sensor value

  char dataToSend[32];
  frame.hexChar(dataToSend, sizeof(dataToSend));
  lora.send(dataToSend);                   // Send via LoRa

  delay(60000);                            // Send every minute
}
```

### TTN Decoder Output (with printDecoder = true)
```
val_uint16 = input.bytes[0] | input.bytes[1] << 8;
val_uint8 = input.bytes[2];
```

---

## Notes

    Buffer Size: The frame uses a fixed 255-byte buffer. Ensure your payload doesn’t exceed this limit to avoid overflow.
    Signed Integers: When using append_int*_t, the values are treated as signed, with appropriate bit-shifting for TTN decoding.
    Portability: Compatible with any microcontroller supporting C++ and a serial interface (e.g., Arduino, ESP32, STM32).

---

## Limitations

    No built-in overflow protection (yet). Be mindful of the 255-byte limit when appending data.
    Debugging output relies on a serial port being available.

---

## Contributing

Feel free to fork the repository, submit issues, or send pull requests at https://github.com/manuelespinosa/tinyFrame. Suggestions for improvements are welcome!
License

This project is licensed under the MIT License - see the LICENSE file for details.
About

Developed by Manuel Espinosa for simplifying payload construction in microcontroller-based IoT projects, with a focus on LoRaWAN compatibility.

© 2025 Manuel Espinosa
