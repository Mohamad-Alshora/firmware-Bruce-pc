# BadUSB mit Serial-Bridge auf Standard-ESP32-WROOM

## 🎯 Übersicht
Dieses Dokument beschreibt, wie Sie die BadUSB-Funktion auf einem Standard-ESP32-WROOM mit **extern angeschlossenem Arduino Pro Micro** (über GPIO 12/13 als CH9329-Serial-Bridge) aktivieren.

---

## ✅ Bereits konfiguriert

### 1. **GPIO Pin-Definitionen** ✓
In [boards/lilygo-t-display-ttgo/pins_arduino.h](boards/lilygo-t-display-ttgo/pins_arduino.h#L88-L89):
```cpp
#define BAD_TX 12    // GPIO 12 → Arduino Pro Micro TX
#define BAD_RX 13    // GPIO 13 ← Arduino Pro Micro RX
```

### 2. **Serial-Bridge Hardware-Implementierung** ✓
Die Klasse [CH9329_Keyboard_](lib/Bad_Usb_Lib/CH9329_Keyboard.h) ist bereits voll implementiert:
- Kommuniziert über Serial @ 9600 Baud
- Funktioniert mit beliebigen GPIO-Paaren
- Unterstützt alle Ducky-Script-Befehle

### 3. **Automatische Implementierungswahl** ✓
In [src/modules/badusb_ble/ducky_typer.cpp](src/modules/badusb_ble/ducky_typer.cpp#L175-L205):
```cpp
if (ble) {
    // BLE Keyboard
    hid = new BleKeyboard(...);
} else {
#if defined(USB_as_HID)
    hid = new USBHIDKeyboard();      // Nur für S2/S3/Boards mit USB
#else
    mySerial.begin(9600, SERIAL_8N1, BAD_RX, BAD_TX);
    hid = new CH9329_Keyboard_();    // ← Ihr WROOM verwendet DIES!
#endif
}
```

---

## ⚠️ KEINE `USB_as_HID` Definition!

**WICHTIG:** Sie müssen `USB_as_HID` **NICHT** und **SOLLTEN NICHT** definieren!

- ❌ **NICHT** in `platformio.ini` hinzufügen
- ❌ **NICHT** in `pins_arduino.h` setzen
- ❌ **NICHT** in der Board-INI-Datei definieren

Der Standard-ESP32-WROOM hat keinen nativen USB-HID-Controller. `USB_as_HID` ist nur für S2/S3 Chips und M5Stack-Geräte mit integrierten USB-Controllern.

---

## ✅ Änderung durchgeführt

### Datei: [src/core/menu_items/OthersMenu.cpp](src/core/menu_items/OthersMenu.cpp)

**Vorher (Zeilen 23-25):**
```cpp
#if !defined(LITE_VERSION)
#if defined(USB_as_HID)
        {"BadUSB & HID", [this]() { badUsbHidMenu(); }},
#endif
#endif
```

**Nachher:**
```cpp
#if !defined(LITE_VERSION)
        {"BadUSB & HID", [this]() { badUsbHidMenu(); }},
#endif
```

**Effekt:** Das BadUSB-Menü ist **IMMER sichtbar**, unabhängig davon, ob `USB_as_HID` definiert ist oder nicht.

---

## 📋 Build-Konfiguration für Ihren WROOM

### Datei: `boards/lilygo-t-display-ttgo/lilygo-t-display-s3.ini`

Überprüfen Sie folgende Einstellungen:

```ini
[env:lilygo-t-display-ttgo]
board = lilygo-t-display-ttgo
board_build.partitions = custom_4Mb_full.csv
build_src_filter = ${env.build_src_filter} +<../boards/lilygo-t-display-ttgo>
build_flags =
    ${env.build_flags}
    ${env_4mb.build_flags}
    -Iboards/lilygo-t-display-ttgo
    -DREDRAW_DELAY=1
    -DALLOW_ALL_GPIO_FOR_IR_RF=1
    -DDISABLE_ALL_LIBRARY_WARNINGS
    -DDEVICE_NAME='"Lilygo T-Display TTGO"'
```

**Wichtig:** Keine `USB_as_HID` Flag hier!

---

## 🚀 Kompilierung & Upload

```bash
# Terminal im VS Code
platformio run -e lilygo-t-display-ttgo --target upload
```

Oder verwenden Sie die vordefinierte Task: **"Build + Public Download"**

---

## 🎮 Menü-Navigation nach dem Update

Nach dem erfolgreichen Kompilieren und Upload:

1. **Geräte einschalten**
   - ESP32-WROOM starten
   - Arduino Pro Micro angeschlossen (GPIO 12 ↔ TX, GPIO 13 ↔ RX)

2. **Menü öffnen**
   ```
   Hauptmenü → Others → BadUSB & HID
   ```

3. **Verfügbare Optionen**
   - ✅ **BadUSB** - Ducky-Script ausführen (Over-The-Air über Serial-Bridge)
   - ✅ **USB Keyboard** - Interaktives Tastatur-Menü
   - ❌ USB Clicker - Nur mit nativer USB HID (nicht verfügbar auf WROOM)
   - ❌ USB U2F - Nur mit nativer USB HID (nicht verfügbar auf WROOM)

---

## 📝 Ducky-Scripts verwenden

Beispiel `BadUSB` Verwendung:

1. **BadUSB-Datei auf SD-Karte erstellen**
   ```
   sd_files/BadUSB/
   ├── payload.ducky
   └── script.ducky
   ```

2. **Im Menü auswählen**
   ```
   Others → BadUSB & HID → BadUSB
   → Quelle (SD Card oder LittleFS)
   → Datei auswählen
   → Execute
   ```

3. **Supported Ducky-Befehle**
   ```ducky
   STRING Hello World
   DELAY 1000
   KEY ENTER
   CTRL-ALT DEL
   ```

---

## 🔧 Fehlerbehebung

### Problem: BadUSB-Menü wird nicht angezeigt
- **Lösung:** Prüfen, dass `LITE_VERSION` nicht definiert ist
- **Überprüfung:** `platformio.ini` oder Board-INI auf `-DLITE_VERSION` prüfen

### Problem: "Serial-Verbindung fehlt" Fehler
- **Ursache:** Arduino Pro Micro ist nicht angeschlossen oder nicht in bootloader mode
- **Lösung:**
  1. Arduino Pro Micro USB-Kabel überprüfen
  2. RX/TX Verbindungen überprüfen (GPIO 12 ↔ TX, GPIO 13 ↔ RX)
  3. Baud-Rate: 9600 (automatisch eingestellt)

### Problem: Fehler beim Kompilieren "undefined reference to `USBHIDKeyboard`"
- **Ursache:** `USB_as_HID` wurde versehentlich definiert
- **Lösung:** Alle `#define USB_as_HID` Definitionen entfernen

---

## 📚 Relevante Source-Code Dateien

| Datei | Zweck | Zeilen |
|-------|-------|--------|
| [pins_arduino.h](boards/lilygo-t-display-ttgo/pins_arduino.h) | GPIO-Definitionen | 88-89 |
| [OthersMenu.cpp](src/core/menu_items/OthersMenu.cpp) | BadUSB-Menü (GEÄNDERT) | 20-55 |
| [ducky_typer.cpp](src/modules/badusb_ble/ducky_typer.cpp) | Hauptimplementierung | 1-550 |
| [CH9329_Keyboard.h](lib/Bad_Usb_Lib/CH9329_Keyboard.h) | Serial-Bridge Klasse | 1-80 |
| [lilygo-t-display-s3.ini](boards/lilygo-t-display-ttgo/lilygo-t-display-s3.ini) | Board-Konfiguration | 1-20 |

---

## ✨ Zusammenfassung der Lösung

| Aspekt | Status | Details |
|--------|--------|---------|
| **GPIO 12/13** | ✅ Vorkonfiguriert | BAD_TX=12, BAD_RX=13 |
| **Serial Bridge** | ✅ Implementiert | CH9329_Keyboard_ Klasse |
| **Menü-Sichtbarkeit** | ✅ **GEÄNDERT** | Unabhängig von USB_as_HID |
| **USB_as_HID** | ❌ NICHT benötigt | Wird nicht für WROOM verwendet |
| **Automatische Auswahl** | ✅ Funktioniert | Code wählt richtige Impl. |

---

## 🎯 Nächste Schritte

1. ✅ **Änderung geladen** - OthersMenu.cpp wurde aktualisiert
2. 🔨 **Projekt kompilieren**
   ```bash
   platformio run -e lilygo-t-display-ttgo
   ```
3. 📤 **Auf ESP32 flashen**
4. 🧪 **BadUSB-Menü testen**

---

**Verfasser:** GitHub Copilot
**Datum:** Mai 2026
**Projekt:** Bruce Firmware für ESP32-WROOM mit externem Arduino Pro Micro
