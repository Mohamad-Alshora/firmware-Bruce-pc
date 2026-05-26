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

#include <Keyboard.h>

// Globale Variablen für Modifier-Tracking
bool ctrl_pressed = false;
bool shift_pressed = false;
bool alt_pressed = false;
bool gui_pressed = false;

// Lookup-Tabelle für Sondertasten
struct KeyMapping {
    const char* name;
    uint8_t keycode;
};

const KeyMapping special_keys[] = {
    // Modifiers
    {"CTRL", KEY_LEFT_CTRL},
    {"SHIFT", KEY_LEFT_SHIFT},
    {"ALT", KEY_LEFT_ALT},
    {"GUI", KEY_LEFT_GUI},
    {"RCTRL", KEY_RIGHT_CTRL},
    {"RSHIFT", KEY_RIGHT_SHIFT},
    {"RALT", KEY_RIGHT_ALT},
    {"RGUI", KEY_RIGHT_GUI},
    
    // Function/Special Keys
    {"ENTER", KEY_RETURN},
    {"RETURN", KEY_RETURN},
    {"ESC", KEY_ESC},
    {"ESCAPE", KEY_ESC},
    {"BACKSPACE", KEY_BACKSPACE},
    {"TAB", KEY_TAB},
    {"SPACE", ' '},
    
    // Navigation
    {"HOME", KEY_HOME},
    {"END", KEY_END},
    {"PAGEUP", KEY_PAGE_UP},
    {"PAGEDOWN", KEY_PAGE_DOWN},
    {"UP", KEY_UP_ARROW},
    {"DOWN", KEY_DOWN_ARROW},
    {"LEFT", KEY_LEFT_ARROW},
    {"RIGHT", KEY_RIGHT_ARROW},
    
    // Editing
    {"INSERT", KEY_INSERT},
    {"DELETE", KEY_DELETE},
    
    // Special
    {"CAPSLOCK", KEY_CAPS_LOCK},
    {"NUMLOCK", KEY_NUM_LOCK},
    {"SCROLLLOCK", KEY_SCROLL_LOCK},
    {"PRINTSCREEN", KEY_PRINT_SCREEN},
    {"PAUSE", KEY_PAUSE},
    {"MENU", KEY_MENU},
    
    // Function Keys
    {"F1", KEY_F1}, {"F2", KEY_F2}, {"F3", KEY_F3}, {"F4", KEY_F4},
    {"F5", KEY_F5}, {"F6", KEY_F6}, {"F7", KEY_F7}, {"F8", KEY_F8},
    {"F9", KEY_F9}, {"F10", KEY_F10}, {"F11", KEY_F11}, {"F12", KEY_F12},
    {"F13", KEY_F13}, {"F14", KEY_F14}, {"F15", KEY_F15}, {"F16", KEY_F16},
    {"F17", KEY_F17}, {"F18", KEY_F18}, {"F19", KEY_F19}, {"F20", KEY_F20},
    {"F21", KEY_F21}, {"F22", KEY_F22}, {"F23", KEY_F23}, {"F24", KEY_F24},
    
    {nullptr, 0}  // Ende der Tabelle
};

void setup() {
    Serial.begin(115200);  // TextBridge @ 115200 Baud
    Keyboard.begin();      // USB-HID Keyboard initialisieren
    
    delay(1000);
    Serial.println("[TextBridge] Arduino Pro Micro ready");
}

void loop() {
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        
        if (cmd.length() == 0) return;
        
        // Debug Output (optional - für Entwicklung)
        Serial.print("RX: ");
        Serial.println(cmd);
        
        if (cmd.startsWith("STR:")) {
            // STRING-Befehl: Text schreiben
            String text = cmd.substring(4);
            handleStringCommand(text);
        }
        else if (cmd.startsWith("KEY:")) {
            // KEY-Befehl: Tasten drücken
            String key = cmd.substring(4);
            handleKeyCommand(key);
        }
        else if (cmd.startsWith("DLY:")) {
            // DELAY-Befehl: Warten
            String ms_str = cmd.substring(4);
            int ms = ms_str.toInt();
            if (ms > 0) {
                delay(ms);
                Serial.print("Delayed: ");
                Serial.println(ms);
            }
        }
        else {
            Serial.print("Unknown command: ");
            Serial.println(cmd);
        }
    }
}

/**
 * Verarbeitet STRING-Befehle
 * Format: "STR:Hello World"
 */
void handleStringCommand(String text) {
    Serial.print("Typing: ");
    Serial.println(text);
    
    // Unescapen spezieller Zeichen
    for (int i = 0; i < text.length(); i++) {
        char c = text[i];
        
        if (c == '\\' && i + 1 < text.length()) {
            char next = text[i + 1];
            if (next == 'n') {
                Keyboard.press(KEY_RETURN);
                Keyboard.releaseAll();
                i++;  // Skip das nächste Zeichen
                continue;
            }
            else if (next == 'r') {
                Keyboard.press(KEY_RETURN);
                Keyboard.releaseAll();
                i++;
                continue;
            }
            else if (next == ':') {
                Keyboard.write(':');
                i++;
                continue;
            }
        }
        
        Keyboard.write(c);
        delay(5);  // Kleine Verzögerung zwischen Zeichen
    }
}

/**
 * Verarbeitet KEY-Befehle
 * Format: "KEY:ENTER" oder "KEY:CTRL+ALT+DEL"
 */
void handleKeyCommand(String key) {
    Serial.print("Pressing: ");
    Serial.println(key);
    
    // Reset Modifiers
    ctrl_pressed = false;
    shift_pressed = false;
    alt_pressed = false;
    gui_pressed = false;
    
    // Parse Kombinationen (z.B. "CTRL+ALT+DEL")
    int pos = 0;
    int plus_pos;
    
    while ((plus_pos = key.indexOf('+', pos)) != -1) {
        String modifier = key.substring(pos, plus_pos);
        parseAndPressModifier(modifier);
        pos = plus_pos + 1;
    }
    
    // Letzter Teil (die eigentliche Taste)
    String lastKey = key.substring(pos);
    
    if (lastKey.length() == 1) {
        // Einzelnes ASCII-Zeichen (z.B. "R" in GUI+R)
        if (ctrl_pressed) Keyboard.press(KEY_LEFT_CTRL);
        if (shift_pressed) Keyboard.press(KEY_LEFT_SHIFT);
        if (alt_pressed) Keyboard.press(KEY_LEFT_ALT);
        if (gui_pressed) Keyboard.press(KEY_LEFT_GUI);
        
        Keyboard.press(lastKey[0]);
        delay(50);
        Keyboard.releaseAll();
    }
    else {
        // Sondertaste (z.B. "ENTER", "F1")
        if (ctrl_pressed) Keyboard.press(KEY_LEFT_CTRL);
        if (shift_pressed) Keyboard.press(KEY_LEFT_SHIFT);
        if (alt_pressed) Keyboard.press(KEY_LEFT_ALT);
        if (gui_pressed) Keyboard.press(KEY_LEFT_GUI);
        
        uint8_t keycode = lookupKeycode(lastKey);
        if (keycode != 0) {
            Keyboard.press(keycode);
            delay(50);
            Keyboard.releaseAll();
        }
    }
}

/**
 * Parst Modifier und setzt die entsprechenden Flags
 */
void parseAndPressModifier(String modifier) {
    modifier.toUpperCase();
    modifier.trim();
    
    if (modifier == "CTRL") {
        ctrl_pressed = true;
    }
    else if (modifier == "SHIFT") {
        shift_pressed = true;
    }
    else if (modifier == "ALT") {
        alt_pressed = true;
    }
    else if (modifier == "GUI") {
        gui_pressed = true;
    }
}

/**
 * Sucht den Keycode für einen Tastenamen
 */
uint8_t lookupKeycode(String keyname) {
    keyname.toUpperCase();
    keyname.trim();
    
    for (int i = 0; special_keys[i].name != nullptr; i++) {
        if (keyname == special_keys[i].name) {
            return special_keys[i].keycode;
        }
    }
    
    // Nicht gefunden
    Serial.print("Unknown key: ");
    Serial.println(keyname);
    return 0;
}

/*
  BEISPIEL DUCKY-SKRIPTE UND IHRE TEXTBRIDGE-AUSGABE:
  
  === Beispiel 1: Windows+R Dialog ===
  Ducky-Script:
    GUI r
    DELAY 200
    STRING notepad
    DELAY 100
    KEY ENTER
  
  TextBridge-Ausgabe:
    KEY:GUI+R
    DLY:200
    STR:notepad
    DLY:100
    KEY:ENTER
  
  === Beispiel 2: CTRL+ALT+DEL ===
  Ducky-Script:
    CTRL-ALT DEL
    DELAY 500
  
  TextBridge-Ausgabe:
    KEY:CTRL+ALT+DELETE
    DLY:500
  
  === Beispiel 3: Text eingeben ===
  Ducky-Script:
    STRING Hello World
    KEY ENTER
    STRING This is a test
  
  TextBridge-Ausgabe:
    STR:Hello World
    KEY:ENTER
    STR:This is a test
*/
