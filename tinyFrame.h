#ifndef TINY_FRAME_H
#define TINY_FRAME_H

#include <Arduino.h>  // Para Serial, etc.

// NUEVO: Enum para tipos simbólicos (extensible)
enum class FrameType : uint8_t {
    VERSION = 0x00,
    TEMPERATURA_SUELO = 0x01,
    HUMEDAD_SUELO = 0x02,
    VWC = 0x03,
    // Agrega más aquí, e.g.:
    // HUMEDAD_SUELO_1_5CM = 0x04,
    // FRAME_NUMBER = 0x10,  // Para fPort 99
    // etc. Reserva rangos por fPort si necesitas (e.g., 0x00-0x0F para fPort 1)
};

// NUEVO PARA TNV: Mapa de longitudes implícitas por type (usa enum)
uint8_t get_value_len(FrameType type) {
    switch (type) {
        case FrameType::VERSION: return 1;  // uint8
        case FrameType::TEMPERATURA_SUELO: return 2;  // int16
        case FrameType::HUMEDAD_SUELO: return 2;  // uint16
        case FrameType::VWC: return 2;  // uint16
        // Agrega más cases para nuevos types
        default: return 0;  // Inválido
    }
}

// Configuraciones
#define FRAME_BUFFER_SIZE 255  // Tamaño max buffer
#define FRAME_SERIAL Serial    // Puerto serial default (cambia a SerialUSB si necesitas)

struct tinyFrame {
    uint8_t buffer[FRAME_BUFFER_SIZE];
    uint16_t index = 0;
    bool printDecoder = false;  // Imprime decoder JS
    bool printAddedBytes = false;  // Log bytes agregados

    void clear() {
        memset(buffer, 0, FRAME_BUFFER_SIZE);
        index = 0;
    }

    // Funciones append existentes (sin cambios)
    void append_uint8_t(uint8_t value) {
        if (index >= FRAME_BUFFER_SIZE) return;
        buffer[index++] = value;
        if (printDecoder) FRAME_SERIAL.print("val_uint8 = input.bytes[" + String(index - 1) + "];\n");
        if (printAddedBytes) FRAME_SERIAL.printf("Added uint8: %u\n", value);
    }

    void append_uint16_t(uint16_t value) {
        if (index + 2 > FRAME_BUFFER_SIZE) return;
        buffer[index++] = value & 0xFF;
        buffer[index++] = value >> 8;
        if (printDecoder) FRAME_SERIAL.print("val_uint16 = input.bytes[" + String(index - 2) + "] | input.bytes[" + String(index - 1) + "] << 8;\n");
        if (printAddedBytes) FRAME_SERIAL.printf("Added uint16: %u\n", value);
    }

    void append_uint32_t(uint32_t value) {
        if (index + 4 > FRAME_BUFFER_SIZE) return;
        buffer[index++] = value & 0xFF;
        buffer[index++] = (value >> 8) & 0xFF;
        buffer[index++] = (value >> 16) & 0xFF;
        buffer[index++] = value >> 24;
        if (printDecoder) FRAME_SERIAL.print("val_uint32 = input.bytes[" + String(index - 4) + "] | input.bytes[" + String(index - 3) + "] << 8 | input.bytes[" + String(index - 2) + "] << 16 | input.bytes[" + String(index - 1) + "] << 24;\n");
        if (printAddedBytes) FRAME_SERIAL.printf("Added uint32: %lu\n", value);
    }

    void append_int8_t(int8_t value) {
        append_uint8_t(static_cast<uint8_t>(value));
        if (printDecoder) FRAME_SERIAL.print(" // Note: Treat as signed int8\n");
    }

    void append_int16_t(int16_t value) {
        append_uint16_t(static_cast<uint16_t>(value));
        if (printDecoder) FRAME_SERIAL.print("val_int16 = (val_uint16 << 16) >> 16;\n");  // Ajuste signed
    }

    void append_int32_t(int32_t value) {
        append_uint32_t(static_cast<uint32_t>(value));
        if (printDecoder) FRAME_SERIAL.print("val_int32 = (val_uint32 << 32) >> 32;\n");  // Ajuste signed
    }

    // NUEVO PARA TNV: Función para agregar Type-Number-Value (usa enum para type)
    // Value debe ser un puntero al dato (e.g., &my_uint16), len implícita por type
    void add_tnv(FrameType type, uint8_t num, const void *value) {
        uint8_t len = get_value_len(type);
        if (len == 0 || index + 2 + len > FRAME_BUFFER_SIZE) return;  // +2 para Type + Num

        // Agrega Type (cast a uint8_t)
        buffer[index++] = static_cast<uint8_t>(type);

        // Agrega Number
        buffer[index++] = num;

        // Agrega Value (copia bytes)
        const uint8_t *val_bytes = static_cast<const uint8_t *>(value);
        for (uint8_t i = 0; i < len; i++) {
            buffer[index++] = val_bytes[i];
        }

        if (printAddedBytes) FRAME_SERIAL.printf("Added TNV: Type=0x%02X (%s), Num=%u, Len=%u\n", 
                                                 static_cast<uint8_t>(type), get_type_name(type).c_str(), num, len);

        // NUEVO: Genera decoder JS para TNV con nombre simbólico
        if (printDecoder) {
            FRAME_SERIAL.print("// TNV Decoder snippet for " + get_type_name(type) + " (integrate in TTN switch):\n");
            FRAME_SERIAL.print("case 0x" + String(static_cast<uint8_t>(type), HEX) + ":  // " + get_type_name(type) + "\n");
            FRAME_SERIAL.print("  var num = input.bytes[" + String(index - len - 1) + "];\n");  // Pos de Num
            if (len == 2) {
                FRAME_SERIAL.print("  var val = input.bytes[" + String(index - len) + "] | input.bytes[" + String(index - 1) + "] << 8;\n");
                FRAME_SERIAL.print("  decoded[\"VWC\" + num] = val;  // Adjust key/name as needed\n");
            } else if (len == 1) {
                FRAME_SERIAL.print("  decoded[\"Field\" + num] = input.bytes[" + String(index - 1) + "];\n");
            }  // Extiende para otros lens/signed
            FRAME_SERIAL.print("  break;\n");
        }
    }

    // NUEVO: Helper para obtener nombre string del enum (para logging/decoder)
    String get_type_name(FrameType type) {
        switch (type) {
            case FrameType::VERSION: return "FrameType::VERSION";
            case FrameType::TEMPERATURA_SUELO: return "FrameType::TEMPERATURA_SUELO";
            case FrameType::HUMEDAD_SUELO: return "FrameType::HUMEDAD_SUELO";
            case FrameType::VWC: return "FrameType::VWC";
            default: return "Unknown";
        }
    }

    // Funciones output existentes (sin cambios)
    void printHex() {
        for (uint16_t i = 0; i < index; i++) {
            FRAME_SERIAL.printf("0x%02X ", buffer[i]);
        }
        FRAME_SERIAL.println();
    }

    void hexChar(char *buff, uint16_t size) {
        if (size < index * 2 + 1) return;  // Asegura espacio
        for (uint16_t i = 0; i < index; i++) {
            sprintf(&buff[i * 2], "%02X", buffer[i]);
        }
        buff[index * 2] = '\0';
    }

    uint16_t size() {
        return index;
    }
};

#endif