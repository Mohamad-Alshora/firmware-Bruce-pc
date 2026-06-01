/*
  Arduino Pro Micro - TextBridge Serial Keyboard Receiver

  Dieses Sketch empfängt TextBridge-Befehle vom ESP32 (GPIO 12/13 @ 115200 Baud)
  und sendet sie als USB-HID-Tastaturlayout an den angeschlossenen Computer.

  TextBridge-Befehls-Format:
  - STR:[text]\n      → Text schreiben
  - KEY:[name]\n      → Einzelne Taste
  - KEY:MOD1+MOD2+KEY\n → Tastenkombination
  - DLY:[ms]\n        → Pause

  Hardware-Verbindung:
  - ESP32 GPIO 12 (BAD_TX) → Arduino Pro Micro RX (0)
  - ESP32 GPIO 13 (BAD_RX) → Arduino Pro Micro TX (1)
  - ESP32 GND → Arduino Pro Micro GND

  Die Arduino Pro Micro wird als USB-HID-Tastatur erkannt.
*/

// TextBridge receiver for Arduino Pro Micro
// This sketch reads TextBridge commands from Serial1 (hardware UART)
// and executes them as USB-HID keyboard events using Keyboard.h.

#include <Keyboard.h>

// Hardware: connect Bruce TTGO TX -> Pro Micro RX (pin 0), Bruce RX -> Pro Micro TX (pin 1), common GND

// Buffer to accumulate incoming line
String lineBuffer = "";

// --- Special key mapping table ---
struct KeyMapping {
    const char *name;
    uint8_t keycode;
};

const KeyMapping special_keys[] = {
    {"ENTER",       KEY_RETURN      },
    {"RETURN",      KEY_RETURN      },
    {"ESC",         KEY_ESC         },
    {"ESCAPE",      KEY_ESC         },
    {"BACKSPACE",   KEY_BACKSPACE   },
    {"TAB",         KEY_TAB         },
    {"SPACE",       ' '             },
    {"HOME",        KEY_HOME        },
    {"END",         KEY_END         },
    {"PAGEUP",      KEY_PAGE_UP     },
    {"PAGEDOWN",    KEY_PAGE_DOWN   },
    {"UP",          KEY_UP_ARROW    },
    {"DOWN",        KEY_DOWN_ARROW  },
    {"LEFT",        KEY_LEFT_ARROW  },
    {"RIGHT",       KEY_RIGHT_ARROW },
    {"INSERT",      KEY_INSERT      },
    {"DELETE",      KEY_DELETE      },
    {"CAPSLOCK",    KEY_CAPS_LOCK   },
    {"NUMLOCK",     KEY_NUM_LOCK    },
    {"SCROLLLOCK",  KEY_SCROLL_LOCK },
    {"PRINTSCREEN", KEY_PRINT_SCREEN},
    {"PAUSE",       KEY_PAUSE       },
    {"MENU",        KEY_MENU        },
    {"F1",          KEY_F1          },
    {"F2",          KEY_F2          },
    {"F3",          KEY_F3          },
    {"F4",          KEY_F4          },
    {"F5",          KEY_F5          },
    {"F6",          KEY_F6          },
    {"F7",          KEY_F7          },
    {"F8",          KEY_F8          },
    {"F9",          KEY_F9          },
    {"F10",         KEY_F10         },
    {"F11",         KEY_F11         },
    {"F12",         KEY_F12         },
    {nullptr,       0               }
};

void setup() {
    Serial.begin(115200);  // USB serial for debug
    Serial1.begin(115200); // Hardware UART to Bruce
    delay(200);
    Keyboard.begin();
    Serial.println("[TextBridge Receiver] Pro Micro ready — executing HID commands");
}

uint8_t lookupKeycode(const String &keyname) {
    String k = keyname;
    k.toUpperCase();
    k.trim();
    for (int i = 0; special_keys[i].name != nullptr; i++) {
        if (k == String(special_keys[i].name)) return special_keys[i].keycode;
    }
    return 0;
}

void parseAndSetModifiers(const String &modifier, bool &ctrl, bool &shift, bool &alt, bool &gui) {
    String m = modifier;
    m.toUpperCase();
    m.trim();
    if (m == "CTRL") ctrl = true;
    else if (m == "SHIFT") shift = true;
    else if (m == "ALT") alt = true;
    else if (m == "GUI") gui = true;
}

void handleStringCommand(const String &text) {
    Serial.print("Typing STR: ");
    Serial.println(text);
    for (int i = 0; i < text.length(); ++i) {
        char c = text[i];
        if (c == '\\' && i + 1 < text.length()) {
            char next = text[i + 1];
            if (next == 'n') {
                Keyboard.press(KEY_RETURN);
                Keyboard.releaseAll();
                i++;
                continue;
            }
            if (next == 'r') {
                Keyboard.press(KEY_RETURN);
                Keyboard.releaseAll();
                i++;
                continue;
            }
            if (next == ':') {
                Keyboard.write(':');
                i++;
                continue;
            }
        }
        Keyboard.write(c);
        delay(5);
    }
}

void handleKeyCommand(String key) {
    Serial.print("KEY command: ");
    Serial.println(key);
    // Remove whitespace
    key.trim();
    // Parse modifiers separated by +
    bool ctrl = false, shift = false, alt = false, gui = false;
    int pos = 0;
    int plus_pos;
    while ((plus_pos = key.indexOf('+', pos)) != -1) {
        String mod = key.substring(pos, plus_pos);
        parseAndSetModifiers(mod, ctrl, shift, alt, gui);
        pos = plus_pos + 1;
    }
    String last = key.substring(pos);
    last.trim();

    // Press modifiers
    if (ctrl) Keyboard.press(KEY_LEFT_CTRL);
    if (shift) Keyboard.press(KEY_LEFT_SHIFT);
    if (alt) Keyboard.press(KEY_LEFT_ALT);
    if (gui) Keyboard.press(KEY_LEFT_GUI);

    // If last is single char, send it
    if (last.length() == 1) {
        Keyboard.press(last[0]);
        delay(50);
        Keyboard.releaseAll();
        return;
    }

    // Otherwise lookup special key code
    uint8_t code = lookupKeycode(last);
    if (code != 0) {
        Keyboard.press(code);
        delay(50);
        Keyboard.releaseAll();
    } else {
        Serial.print("Unknown key: ");
        Serial.println(last);
        Keyboard.releaseAll();
    }
}

void loop() {
    while (Serial1.available()) {
        char c = (char)Serial1.read();
        if (c == '\r') continue;
        if (c == '\n') {
            if (lineBuffer.length() > 0) {
                String cmd = lineBuffer;
                // Some forwarders may prefix with "RX: ", remove if present
                if (cmd.startsWith("RX: ")) cmd = cmd.substring(4);
                cmd.trim();
                Serial.print("RX: ");
                Serial.println(cmd);
                if (cmd.startsWith("STR:")) {
                    String s = cmd.substring(4);
                    handleStringCommand(s);
                } else if (cmd.startsWith("KEY:")) {
                    String k = cmd.substring(4);
                    handleKeyCommand(k);
                } else if (cmd.startsWith("DLY:")) {
                    String ms_str = cmd.substring(4);
                    int ms = ms_str.toInt();
                    if (ms > 0) {
                        Serial.print("Delay: ");
                        Serial.println(ms);
                        delay(ms);
                    }
                } else {
                    Serial.print("Unknown cmd: ");
                    Serial.println(cmd);
                }
                lineBuffer = "";
            }
        } else {
            lineBuffer += c;
            if (lineBuffer.length() > 1024) {
                Serial.println("[WARN] line too long, truncating");
                lineBuffer = lineBuffer.substring(lineBuffer.length() - 1024);
            }
        }
    }
}
