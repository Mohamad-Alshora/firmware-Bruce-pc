# TextBridge Serial Protocol - ESP32 BadUSB für Arduino Pro Micro

## 🎯 Übersicht

Die Bruce-Firmware wurde so modifiziert, dass der ESP32-WROOM DuckyScript-Befehle **nicht mehr als binäre CH9329-Pakete**, sondern als **einfache, lesbare Text-Befehle** über die serielle Schnittstelle an den Arduino Pro Micro sendet.

### Vorteile:
- ✅ **Einfach zu debuggen** - Befehle sind im Terminal lesbar
- ✅ **Fehlerresistent** - Keine komplexen Binär-Protokolle
- ✅ **Flexibel erweiterbar** - Leicht neue Befehle hinzufügbar
- ✅ **Arduino-seitig einfach** - Einfache String-Parsing-Logik

---

## 📋 TextBridge Protokoll-Format

### Verbindung
- **GPIO 12** (ESP32 BAD_TX) → **RX** (Arduino Pro Micro)
- **GPIO 13** (ESP32 BAD_RX) → **TX** (Arduino Pro Micro)
- **Baud-Rate**: 115200 (statt 9600 bei CH9329)
- **Format**: 8N1 (8 Datenbits, kein Parity, 1 Stopbit)

### Befehls-Format

Alle Befehle enden mit `\n` (Newline/LF).

#### 1. **STRING-Befehl** - Text schreiben
```
STR:[text]\n
```
**Beispiel:**
```
STR:Hello World\n
STR:admin\n
STR:password123\n
```

Spezialzeichen innerhalb von Text:
- `\n` → Zeilenumbruch (innerhalb STR: würde als zwei Zeichen interpretiert)
- `\r` → Carriage Return
- `\:` → Doppelpunkt (escapen)

#### 2. **KEY-Befehl** - Einzelne Tasten
```
KEY:[keyname]\n
```
**Beispiele:**
```
KEY:ENTER\n
KEY:TAB\n
KEY:BACKSPACE\n
KEY:ESC\n
KEY:F1\n
KEY:HOME\n
KEY:DELETE\n
```

**Unterstützte Keys:**
- Funktionstasten: F1-F24
- Navigationstasten: UP, DOWN, LEFT, RIGHT, HOME, END, PAGEUP, PAGEDOWN
- Sondertasten: ENTER, TAB, BACKSPACE, ESC, DELETE, INSERT
- Modifiers: CTRL, SHIFT, ALT, GUI
- Weitere: CAPSLOCK, NUMLOCK, SCROLLLOCK, PAUSE, PRINTSCREEN, MENU

#### 3. **KEY-Kombinationen** - Modifiers + Taste
```
KEY:MOD1+MOD2+...+KEY\n
```
**Beispiele:**
```
KEY:GUI+R\n              # Windows+R (Run-Dialog öffnen)
KEY:CTRL+C\n            # Kopieren
KEY:CTRL+V\n            # Einfügen
KEY:CTRL+SHIFT+ESC\n    # Task Manager
KEY:CTRL+ALT+DEL\n      # Sicherheitsmenü
KEY:ALT+F4\n            # Fenster schließen
KEY:SHIFT+TAB\n         # Reverse Tab
KEY:GUI+SHIFT+S\n       # Screenshot Tool (Windows 10+)
```

#### 4. **DELAY-Befehl** - Pause
```
DLY:[milliseconds]\n
```
**Beispiele:**
```
DLY:1000\n    # 1 Sekunde warten
DLY:500\n     # 500 Millisekunden warten
DLY:100\n     # 100 Millisekunden warten
```

---

## 📝 Beispiel DuckyScript und resultierende TextBridge-Ausgabe

### Input (DuckyScript)
```ducky
REM Windows Calculator öffnen
DELAY 500
GUI r
DELAY 200
STRING calc
DELAY 200
KEY ENTER
DELAY 1000

REM Einfache Rechnung
STRING 2+2
KEY ENTER
DELAY 500

REM Fenster schließen
KEY F4
```

### Output (TextBridge)
```
DLY:500
KEY:GUI+R
DLY:200
STR:calc
DLY:200
KEY:ENTER
DLY:1000
STR:2+2
KEY:ENTER
DLY:500
KEY:F4
```

---

## 🛠️ Implementierung der Arduino Pro Micro Seite

Der Arduino Pro Micro empfängt diese Befehle über die **Hardware Serial** und interpretiert sie:

### Pseudo-Code
```cpp
void setup() {
    Serial.begin(115200);  // Richtige Baud-Rate!
    Keyboard.begin();      // USB HID Keyboard
}

void loop() {
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        
        if (cmd.startsWith("STR:")) {
            String text = cmd.substring(4);
            Keyboard.print(text);
        }
        else if (cmd.startsWith("KEY:")) {
            String key = cmd.substring(4);
            // Parse und sende Key-Kombination
            // z.B. "GUI+R" → Mod(GUI) + Key(R)
            parseAndPressKey(key);
        }
        else if (cmd.startsWith("DLY:")) {
            int ms = cmd.substring(4).toInt();
            delay(ms);
        }
    }
}
```

---

## 📚 Geänderte Dateien (Zusammenfassung)

### **Neue Dateien:**
1. `lib/Bad_Usb_Lib/TextBridgeKeyboard.h` - Header
2. `lib/Bad_Usb_Lib/TextBridgeKeyboard.cpp` - Implementierung

### **Geänderte Dateien:**
| Datei | Änderung |
|-------|----------|
| `lib/Bad_Usb_Lib/Bad_Usb_Lib.h` | Neue Methode `sendDelay()` zur Abstraktionsebene |
| `src/modules/badusb_ble/ducky_typer.cpp` | Eingebundene TextBridgeKeyboard.h, ersetzt CH9329 mit TextBridge, Baud-Rate 115200, entfernt CH9329-Probe |
| `src/core/menu_items/OthersMenu.cpp` | BadUSB-Menü immer sichtbar (nicht an USB_as_HID gebunden) |

### **Entfernte Abhängigkeiten:**
- ❌ CH9329_Keyboard.h (nicht mehr verwendet)
- ❌ CH9329_DEFAULT_BAUDRATE (9600)
- ❌ CH9329-Probing-Logik

---

## 🔄 Kompilierung

```bash
platformio run -e lilygo-t-display-ttgo --target upload
```

Die neue TextBridgeKeyboard-Klasse wird automatisch verwendet, wenn `USB_as_HID` **nicht** definiert ist (Standard für ESP32-WROOM).

---

## 🧪 Testing & Debugging

### 1. Mit serieller Verbindung testen
```bash
# Terminal zu ESP32 @ 115200 Baud verbinden
# Im Bruce-Menü: Others → BadUSB & HID → BadUSB
# Eine einfache Ducky-Datei ausführen
# Die OUTPUT-Befehle sollten im Terminal sichtbar sein
```

### 2. Mit Logic Analyzer
- GPIO 12 (Tx) und GPIO 13 (Rx) mit Analyzer verbinden
- 115200 Baud UART dekodieren
- Befehle sollten in lesbarem ASCII-Text sichtbar sein

### 3. Arduino Pro Micro seitig
```cpp
void setup() {
    Serial.begin(115200);  // Monitor die Befehle
    Keyboard.begin();
}

void loop() {
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        Serial.print("RX: ");  // Debug
        Serial.println(cmd);
    }
}
```

---

## ⚙️ Konfiguration

### ESP32 Seite (automatisch):
- Baud-Rate: **115200**
- GPIO: BAD_TX=12, BAD_RX=13 (in pins_arduino.h)
- Alle DuckyScript-Befehle werden zu TextBridge-Befehlen konvertiert

### Arduino Pro Micro Seite:
1. Serial @ **115200 Baud** initialisieren
2. Befehle von UART.receive() lesen
3. String-Parser implementieren
4. Entsprechende USB-HID-Befehle ausführen

---

## 🎯 Aktuelle Features

✅ STRING-Befehle (Text schreiben)
✅ KEY-Befehle (Einzelne Tasten)
✅ KEY-Kombinationen (Modifiers + Key)
✅ DELAY-Befehle (Pausen)
✅ Alle Standard-Ducky-Keys unterstützt
✅ Automatische Modifier-Tracking für Kombinationen

---

## 📌 Notizen

- Die **Baud-Rate** wurde von 9600 (CH9329) auf **115200** erhöht für bessere Durchsatzgeschwindigkeit
- **Modifiers** werden automatisch tracked und mit dem nächsten Key kombiniert
- **Verzögerungen** werden auf dem ESP32 durchgeführt, aber auch als DLY-Befehle gesendet (für Arduino-seitige Optimierung)
- Die Implementierung ist **100% rückwärtskompatibel** mit der bestehenden DuckyScript-Syntax

---

**Verfasser:** GitHub Copilot  
**Datum:** Mai 2026  
**Projekt:** Bruce Firmware - TextBridge Serial Protocol
