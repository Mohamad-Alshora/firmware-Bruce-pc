/*
  TextBridgeKeyboard.cpp
  
  Implementation of simple text-based serial keyboard bridge.
  Sends keyboard commands as readable text to external Arduino Pro Micro.
*/

#include "TextBridgeKeyboard.h"

TextBridgeKeyboard::TextBridgeKeyboard(void) : _stream(nullptr), _asciimap(KeyboardLayout_en_US) {
    memset(_keyBuffer, 0, sizeof(_keyBuffer));
}

void TextBridgeKeyboard::begin(Stream &stream, const uint8_t *layout) {
    _stream = &stream;
    _asciimap = layout;
    Serial.println("[TextBridge] Keyboard initialized");
}

void TextBridgeKeyboard::begin(const uint8_t *layout) {
    _asciimap = layout;
}

void TextBridgeKeyboard::end(void) {
    _stream = nullptr;
}

int TextBridgeKeyboard::getReportData(uint8_t *buffer, size_t size) {
    return 0;
}

// Convert key code to readable name
const char* TextBridgeKeyboard::keyCodeToString(uint8_t keyCode) {
    switch (keyCode) {
        // Modifiers
        case KEY_LEFT_CTRL:      return "CTRL";
        case KEY_LEFT_SHIFT:     return "SHIFT";
        case KEY_LEFT_ALT:       return "ALT";
        case KEY_LEFT_GUI:       return "GUI";
        case KEY_RIGHT_CTRL:     return "RCTRL";
        case KEY_RIGHT_SHIFT:    return "RSHIFT";
        case KEY_RIGHT_ALT:      return "RALT";
        case KEY_RIGHT_GUI:      return "RGUI";
        
        // Function keys
        case KEY_RETURN:         return "ENTER";
        case KEY_ESC:            return "ESC";
        case KEYBACKSPACE:       return "BACKSPACE";
        case KEYTAB:             return "TAB";
        case KEY_SPACE:          return "SPACE";
        
        // Navigation keys
        case KEY_HOME:           return "HOME";
        case KEY_END:            return "END";
        case KEY_PAGE_UP:        return "PAGEUP";
        case KEY_PAGE_DOWN:      return "PAGEDOWN";
        case KEY_UP_ARROW:       return "UP";
        case KEY_DOWN_ARROW:     return "DOWN";
        case KEY_LEFT_ARROW:     return "LEFT";
        case KEY_RIGHT_ARROW:    return "RIGHT";
        
        // Editing keys
        case KEY_INSERT:         return "INSERT";
        case KEY_DELETE:         return "DELETE";
        
        // Special keys
        case KEY_CAPS_LOCK:      return "CAPSLOCK";
        case KEY_NUM_LOCK:       return "NUMLOCK";
        case KEY_SCROLL_LOCK:    return "SCROLLLOCK";
        case KEY_PRINT_SCREEN:   return "PRINTSCREEN";
        case KEY_PAUSE:          return "PAUSE";
        case KEY_MENU:           return "MENU";
        
        // Function keys F1-F24
        case KEY_F1:  return "F1";  case KEY_F2:  return "F2";
        case KEY_F3:  return "F3";  case KEY_F4:  return "F4";
        case KEY_F5:  return "F5";  case KEY_F6:  return "F6";
        case KEY_F7:  return "F7";  case KEY_F8:  return "F8";
        case KEY_F9:  return "F9";  case KEY_F10: return "F10";
        case KEY_F11: return "F11"; case KEY_F12: return "F12";
        case KEY_F13: return "F13"; case KEY_F14: return "F14";
        case KEY_F15: return "F15"; case KEY_F16: return "F16";
        case KEY_F17: return "F17"; case KEY_F18: return "F18";
        case KEY_F19: return "F19"; case KEY_F20: return "F20";
        case KEY_F21: return "F21"; case KEY_F22: return "F22";
        case KEY_F23: return "F23"; case KEY_F24: return "F24";
        
        default:
            return nullptr;
    }
}

bool TextBridgeKeyboard::isModifierKey(uint8_t keyCode) {
    return (keyCode == KEY_LEFT_CTRL || keyCode == KEY_LEFT_SHIFT || 
            keyCode == KEY_LEFT_ALT || keyCode == KEY_LEFT_GUI ||
            keyCode == KEY_RIGHT_CTRL || keyCode == KEY_RIGHT_SHIFT ||
            keyCode == KEY_RIGHT_ALT || keyCode == KEY_RIGHT_GUI);
}

void TextBridgeKeyboard::sendKeyCommand(const char* keyName) {
    if (_stream && keyName) {
        _stream->print("KEY:");
        _stream->print(keyName);
        _stream->print("\n");
        _stream->flush();
        if (_delay_ms > 0) delay(_delay_ms);
    }
}

void TextBridgeKeyboard::sendKeyCombo(const char* keyName) {
    if (!_stream || !keyName) return;
    
    // Build combo string: MOD1+MOD2+...+KEY
    _stream->print("KEY:");
    
    if (_ctrl) _stream->print("CTRL+");
    if (_shift) _stream->print("SHIFT+");
    if (_alt) _stream->print("ALT+");
    if (_gui) _stream->print("GUI+");
    
    _stream->print(keyName);
    _stream->print("\n");
    _stream->flush();
    
    // Reset modifiers after sending
    _ctrl = false;
    _shift = false;
    _alt = false;
    _gui = false;
    
    if (_delay_ms > 0) delay(_delay_ms);
}

size_t TextBridgeKeyboard::write(uint8_t k) {
    if (!_stream) return 0;
    
    // Send single character as STR command
    _stream->print("STR:");
    _stream->print((char)k);
    _stream->print("\n");
    _stream->flush();
    
    if (_delay_ms > 0) delay(_delay_ms);
    return 1;
}

size_t TextBridgeKeyboard::write(const uint8_t *buffer, size_t size) {
    if (!_stream || !buffer) return 0;
    
    // Send entire string as single STR command
    _stream->print("STR:");
    for (size_t i = 0; i < size; i++) {
        // Escape special characters if needed
        if (buffer[i] == '\n') {
            _stream->print("\\n");
        } else if (buffer[i] == '\r') {
            _stream->print("\\r");
        } else if (buffer[i] == ':') {
            _stream->print("\\:");
        } else {
            _stream->write(buffer[i]);
        }
    }
    _stream->print("\n");
    _stream->flush();
    
    if (_delay_ms > 0) delay(_delay_ms);
    return size;
}

size_t TextBridgeKeyboard::press(uint8_t k) {
    if (!_stream) return 0;
    
    // Track modifiers
    if (k == KEY_LEFT_CTRL || k == KEY_RIGHT_CTRL) {
        _ctrl = true;
        return 1;
    }
    if (k == KEY_LEFT_SHIFT || k == KEY_RIGHT_SHIFT) {
        _shift = true;
        return 1;
    }
    if (k == KEY_LEFT_ALT || k == KEY_RIGHT_ALT) {
        _alt = true;
        return 1;
    }
    if (k == KEY_LEFT_GUI || k == KEY_RIGHT_GUI) {
        _gui = true;
        return 1;
    }
    
    const char* keyName = keyCodeToString(k);
    
    if (keyName) {
        // It's a special key - send with modifiers if any are pressed
        if (_ctrl || _shift || _alt || _gui) {
            sendKeyCombo(keyName);
        } else {
            sendKeyCommand(keyName);
        }
    } else if (k < 128) {
        // Regular ASCII character - send with modifiers if any are pressed
        if (_ctrl || _shift || _alt || _gui) {
            _stream->print("KEY:");
            if (_ctrl) _stream->print("CTRL+");
            if (_shift) _stream->print("SHIFT+");
            if (_alt) _stream->print("ALT+");
            if (_gui) _stream->print("GUI+");
            _stream->print((char)k);
            _stream->print("\n");
            _stream->flush();
            
            _ctrl = false;
            _shift = false;
            _alt = false;
            _gui = false;
            
            if (_delay_ms > 0) delay(_delay_ms);
        } else {
            _stream->print("STR:");
            _stream->print((char)k);
            _stream->print("\n");
            _stream->flush();
            if (_delay_ms > 0) delay(_delay_ms);
        }
    }
    
    return 1;
}

// Override println to send ENTER key instead of newline
size_t TextBridgeKeyboard::println(void) {
    // Send ENTER key command instead of newline character
    if (_stream) {
        sendKeyCommand("ENTER");
    }
    return 1;
}

size_t TextBridgeKeyboard::println(const char *str) {
    if (str) write((const uint8_t *)str, strlen(str));
    return println();
}

void TextBridgeKeyboard::sendDelay(uint32_t ms) {
    if (_stream && ms > 0) {
        _stream->print("DLY:");
        _stream->print(ms);
        _stream->print("\n");
        _stream->flush();
    }
    // Also perform the delay on ESP32 side
    delay(ms);
}

size_t TextBridgeKeyboard::release(uint8_t k) {
    // TextBridge doesn't need explicit release - commands are atomic
    return 0;
}

void TextBridgeKeyboard::releaseAll(void) {
    // No need to explicitly release keys with text protocol
    // Keys are released immediately after the command is sent
    if (_stream) {
        _stream->flush();
    }
}
