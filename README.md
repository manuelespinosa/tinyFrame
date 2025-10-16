# tinyFrame

Tiny Frame for Microcontrollers with TTN Decoder Helper

A lightweight library for microcontrollers to build and manage binary payloads, with built-in tools to generate JavaScript-style decoders compatible with The Things Network (TTN). Ideal for IoT applications like LoRaWAN where compact and efficient data handling is key.

## Features

* Simple Payload Construction : Add 8-bit, 16-bit, and 32-bit signed/unsigned integers to a fixed-size buffer with ease.
* TTN Decoder Helper : Optionally prints JavaScript-like decoding instructions for TTN or similar platforms.
* Hexadecimal Output : Convert the payload to a human-readable hexadecimal string for debugging or transmission.
* Lightweight : Designed for resource-constrained microcontrollers, with minimal overhead.
* TNV Support : Added functionality for Type-Number-Value (TNV) encoding, allowing tagged and indexed fields with implicit lengths for extensibility and efficiency in variable payloads.

## Installation

0. Download the latest version from [GitHub](https://github.com/manuelespinosa/tinyFrame).
1. Copy the `tinyFrame.h` file to your project folder or include it in your microcontroller development environment.
2. Include it in your code:

  ```
  #include <tinyFrame.h>
  ```

## Usage

### Basic Example

```
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

### TNV Example

The library now supports TNV (Type-Number-Value) for creating extensible payloads with tagged fields. Types are defined using an enum for better readability and maintainability.

```
void loop() {
  frame.clear();
  
  // Add a version field (using TNV with num=0 for non-array fields)
  uint8_t version = 1;
  frame.add_tnv(FrameType::VERSION, 0, &version);
  
  // Add temperatura_suelo (int16)
  int16_t temp = 255;  // e.g., 25.5°C * 10
  frame.add_tnv(FrameType::TEMPERATURA_SUELO, 0, &temp);
  
  // Add VWC fields as array (indexed with num)
  uint16_t vwc0 = 1234;
  frame.add_tnv(FrameType::VWC, 0, &vwc0);  // VWC0
  uint16_t vwc1 = 5678;
  frame.add_tnv(FrameType::VWC, 1, &vwc1);  // VWC1
  
  frame.printHex();  // Print the resulting hex payload
}
```

### Console Output for TNV (with printDecoder = true)

The decoder helper now generates snippets for TNV parsing, suitable for integration into a TTN decoder function with a loop and switch.

```
// TNV Decoder snippet for FrameType::VERSION (integrate in TTN switch):
case 0x00:  // FrameType::VERSION
  var num = input.bytes[1];
  decoded["Field" + num] = input.bytes[2];
  break;

// TNV Decoder snippet for FrameType::TEMPERATURA_SUELO (integrate in TTN switch):
case 0x01:  // FrameType::TEMPERATURA_SUELO
  var num = input.bytes[4];
  var val = input.bytes[5] | input.bytes[6] << 8;
  decoded["TemperaturaSuelo" + num] = val;  // Adjust key/name as needed
  break;

// Similar for VWC...
```

### Basic TNV decoder for TTN
```
function decodeUplink(input) {
  var decoded = {};
  var bytes = input.bytes;
  var offset = 0;
  var warnings = [];
  var errors = [];

  // Helper para leer uint16 (little-endian)
  function readUint16(off) {
    return bytes[off] | (bytes[off + 1] << 8);
  }

  // Helper para leer int16 (signed, si necesitas para temperaturas negativas)
  function readInt16(off) {
    var val = readUint16(off);
    return val > 32767 ? val - 65536 : val;
  }

  // Mapa de longitudes implícitas y escalas (basado en enum FrameType)
  // Ajusta escalas según tu lógica (e.g., /10 para temp, /100 para hum)
  function getTypeInfo(type) {
    switch (type) {
      case 0x00: // VERSION (uint8)
        return { len: 1, key: "version", scale: 1, signed: false };
      case 0x01: // TEMPERATURA_SUELO (uint16 o int16)
        return { len: 2, key: "temperatura_suelo", scale: 10, signed: false }; // Ej: /10 para °C
      case 0x02: // HUMEDAD_SUELO (uint16)
        return { len: 2, key: "humedad_suelo", scale: 100, signed: false }; // Ej: /100 para %
      case 0x03: // VWC (uint16)
        return { len: 2, key: "VWC", scale: 1, signed: false };
      default:
        return null; // Desconocido
    }
  }

  while (offset < bytes.length) {
    if (offset + 2 > bytes.length) { // Min: Type + Num
      errors.push("Payload TNV incompleto");
      break;
    }

    var type = bytes[offset++];
    var num = bytes[offset++];
    var info = getTypeInfo(type);

    if (!info) {
      warnings.push("Tipo desconocido: 0x" + type.toString(16));
      continue; // Salta a próximo TNV
    }

    if (offset + info.len > bytes.length) {
      errors.push("Longitud insuficiente para tipo 0x" + type.toString(16));
      break;
    }

    var val;
    if (info.len === 1) {
      val = bytes[offset++];
    } else if (info.len === 2) {
      val = info.signed ? readInt16(offset) : readUint16(offset);
      offset += 2;
    } // Extiende para len=4 si agregas floats/uint32

    // Aplica escala y guarda en decoded con índice (num)
    var scaledVal = val / info.scale;
    decoded[info.key + num] = Number.isInteger(scaledVal) ? scaledVal : scaledVal.toFixed(2);

  }

  return {
    data: decoded,
    warnings: warnings,
    errors: errors
  };
}
```

----------------
## Old Version. Compatibility Only
### Key Methods

```
clear(): Resets the frame buffer and index to zero.
append_uint8_t(uint8_t), append_uint16_t(uint16_t), append_uint32_t(uint32_t): Append unsigned integers of 8, 16, or 32 bits.
append_int8_t(int8_t), append_int16_t(int16_t), append_int32_t(int32_t): Append signed integers of 8, 16, or 32 bits.
add_tnv(FrameType type, uint8_t num, const void *value): Append a TNV field using the enum type, index (num), and pointer to value. Length is implicit based on type.
printHex(): Prints the current buffer as space-separated hexadecimal values.
hexChar(char *buff, uint16_t size): Converts the buffer to a hexadecimal string (e.g., "873075") in the provided buffer.
size(): Returns the current number of bytes in the frame.
```

### Enum for Frame Types

The library uses an `enum class FrameType` for defining payload types symbolically:

```
enum class FrameType : uint8_t {
    VERSION = 0x00,
    TEMPERATURA_SUELO = 0x01,
    HUMEDAD_SUELO = 0x02,
    VWC = 0x03,
    // Add more as needed
};
```

Extend the enum and update `get_value_len(FrameType type)` and `get_type_name(FrameType type)` for new types.

### Configuration Options

```
printDecoder: Set to true to print TTN-compatible decoder instructions (default: false).
printAddedBytes: Set to true to log each byte as it’s added to the buffer (default: false). Now includes TNV details.
FRAME_SERIAL: Define this macro to use a different serial port (e.g., #define FRAME_SERIAL Serial1). Defaults to Serial.
```

## Example with LoRaWAN

```
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
  uint16_t uptime = millis() / 60000; // Uptime in minutes
  frame.add_tnv(FrameType::VERSION, 0, &uptime); // Example use of TNV
  uint8_t sensor = 42;
  frame.add_tnv(FrameType::HUMEDAD_SUELO, 0, &sensor);

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

## Notes

```
Buffer Size: The frame uses a fixed 255-byte buffer. Ensure your payload doesn’t exceed this limit to avoid overflow.
Signed Integers: When using append_int*_t, the values are treated as signed, with appropriate bit-shifting for TTN decoding.
Portability: Compatible with any microcontroller supporting C++ and a serial interface (e.g., Arduino, ESP32, STM32).
TNV Extensibility: Implicit lengths make TNV efficient for LoRaWAN; add new types to the enum for custom fields.
```

## Limitations

```
No built-in overflow protection (yet). Be mindful of the 255-byte limit when appending data.
Debugging output relies on a serial port being available.
TNV currently supports fixed-length values; variable lengths may require custom extensions.
```

## Contributing

Feel free to fork the repository, submit issues, or send pull requests at [https://github.com/manuelespinosa/tinyFrame](https://github.com/manuelespinosa/tinyFrame). Suggestions for improvements are welcome! License

This project is licensed under the MIT License - see the LICENSE file for details. About

Developed by Manuel Espinosa for simplifying payload construction in microcontroller-based IoT projects, with a focus on LoRaWAN compatibility.

© 2025 Manuel Espinosa
