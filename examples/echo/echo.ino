#include <scode.h>

code_stream_t cs;

void setup() {
    Serial.begin(8600);

    // Initialize the code stream with 16 bytes of buffer to begin with
    cs = init_code_stream(16);

    // We won't need to free the code stream since it will be used for the 
    // entire lifetime of the program
}

void loop() {
    // Wait until there is available data from serial
    while (Serial.available() <= 0) delay(1);

    // Update the stream with the new data
    while (Serial.available() > 0) {
        char c = Serial.read();
        code_stream_update(&cs, &c, 1);
    }
    // Try to process all the available codes
    int res = 0;
    while (res == 0) {
        code_t cmd;
        res = code_stream_pop(&cs, &cmd);
        if (res == 0) {
            // Copy the input code back to serial
            char b[64];
            int size;

            // Echo the human version of the code
            size = code_dump_human(&cmd, b, sizeof(b));
            if (size > 0) {
                b[size] = '\0';
                Serial.print(b);
            }
        
            size = code_dump_binary(&cmd, b, sizeof(b));
            // Echo two versions of the binary code
            if (size > 0) {
                for (size_t i = 0; i < size; ++i) {
                    if (b[i] >= 0x20 && b[i] < 0x7F) {
                        putchar(b[i]);
                        Serial.write(b[i]);
                    } else {
                        Serial.print("\\x");
                        Serial.print((uint8_t)b[i], 16);
                    }
                }
                Serial.println();
                for (size_t i = 0; i < size; ++i) {
                    if (b[i] >= 0x20 && b[i] < 0x7F) {
                        Serial.write(b[i]);
                    } else {
                        Serial.write('.');
                    }
                }
                Serial.println();
            }

            // Don't forget to free the code after you are done with it.
            free_code(&cmd);
        } else {
            switch (res) {
            case SCODE_ERROR_EMPTY:
                break;
            case SCODE_ERROR_CRC:
                Serial.println("CRC Error");
                break;
            case SCODE_ERROR_BUFFER:
                break;
            case SCODE_ERROR_PARSE:
                Serial.println("Parse Error");
                break;
            case SCODE_ERROR_DUMP:
                Serial.println("Dump Error");
                break;
            default:
                Serial.println("Unknown Error");
                break;
            }
        }
    }
}
