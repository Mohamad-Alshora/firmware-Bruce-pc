/*
  TextBridgeKeyboard.h
  
  Simple text-based serial bridge for sending keyboard commands as readable text.
  
  Format:
  - STRING: "STR:[text]\n"
  - KEY: "KEY:[name]\n" (e.g., "KEY:ENTER\n", "KEY:CTRL\n")
  - KEY+MODIFIER: "KEY:MOD1+MOD2+KEY\n" (e.g., "KEY:GUI+R\n", "KEY:CTRL+ALT+DEL\n")
  - DELAY: "DLY:[milliseconds]\n"
  
  Communication: Serial over GPIO (BAD_TX=12, BAD_RX=13) @ 115200 baud
*/

#ifndef TEXT_BRIDGE_KEYBOARD_H
#define TEXT_BRIDGE_KEYBOARD_H

#include "Bad_Usb_Lib.h"
#include "keys.h"
#include <Arduino.h>
#include <cstring>

class TextBridgeKeyboard : public HIDInterface {
private:
    Stream *_stream;
    const uint8_t *_asciimap;
    uint32_t _delay_ms = 50;  // Default delay between key presses
    
    // Track pressed modifiers for combinations
    bool _ctrl = false;
    bool _shift = false;
    bool _alt = false;
    bool _gui = false;
    
    // Helper function to convert key code to readable string
    const char* keyCodeToString(uint8_t keyCode);
    
    // Helper function to check if key is a modifier
    bool isModifierKey(uint8_t keyCode);
    
    // Helper to build and send key combination
    void sendKeyCombo(const char* keyName);
    
    // Helper to send a single key command
    void sendKeyCommand(const char* keyName);
    
    // Buffer for building key combinations
    char _keyBuffer[64];

public:
    TextBridgeKeyboard(void);
    
    // Initialize with stream (GPIO serial) and keyboard layout
    void begin(Stream &stream, const uint8_t *layout = KeyboardLayout_en_US) override;
    void begin(const uint8_t *layout = KeyboardLayout_en_US) override;
    void end(void) override;
    
    // For compatibility with HIDInterface
    int getReportData(uint8_t *buffer, size_t size) override;
    
    // Send text string
    size_t write(uint8_t k) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    
    // Print helpers - println sends ENTER key command
    size_t println(void);
    size_t println(const char *str);
    
    // Send key press (will be released automatically)
    size_t press(uint8_t k) override;
    size_t release(uint8_t k) override;
    void releaseAll(void) override;
    
    // Set keyboard layout
    void setLayout(const uint8_t *layout) override { _asciimap = layout; };
    
    // Set delay between key presses
    void setDelay(uint32_t ms) override { _delay_ms = ms; };
    
    // Send delay command (sends "DLY:[ms]\n")
    void sendDelay(uint32_t ms) override;
    
    // Dummy functions for compatibility
    bool isConnected() override { return _stream != nullptr; };
    
    virtual size_t pressRaw(uint8_t k) override { return 0; };
    virtual size_t releaseRaw(uint8_t k) override { return 0; };
    virtual size_t press(const MediaKeyReport k) override { return 0; };
};

#endif // TEXT_BRIDGE_KEYBOARD_H
