#include "shared.h"
// Note: For letters and digits, we'll map them based on their ASCII values directly

/************************************************************
 *  FUNCTION DEFINITIONS
 ************************************************************/

/**
 * @brief Converts a JSON keydown event to an internal KeyEvent structure.
 *
 * @param jsonString The JSON string representing the keydown event.
 * @return KeyEvent The internal representation of the key event.
 */
KeyEvent convertJsonToKeyEvent(const std::string& jsonString)
{
    try {
        // Parse the JSON string
        json j = json::parse(jsonString);

        // Validate JSON structure
        if (!j.contains("type") || j["type"] != "keydown") {
            std::cerr << "Invalid JSON type or missing 'type' field.\n";
            return KeyEvent(KEY_UNKNOWN, MOD_NONE);
        }

        if (!j.contains("key") || !j.contains("code")) {
            std::cerr << "Missing 'key' or 'code' field in JSON.\n";
            return KeyEvent(KEY_UNKNOWN, MOD_NONE);
        }

        std::string key = j["key"];
        std::string code = j["code"];

        // Initialize modifiers
        int modifiers = MOD_NONE;
        if (j.contains("ctrlKey") && j["ctrlKey"].is_boolean() && j["ctrlKey"].get<bool>()) {
            modifiers |= MOD_CTRL;
        }
        if (j.contains("shiftKey") && j["shiftKey"].is_boolean() && j["shiftKey"].get<bool>()) {
            modifiers |= MOD_SHIFT;
        }
        if (j.contains("altKey") && j["altKey"].is_boolean() && j["altKey"].get<bool>()) {
            modifiers |= MOD_ALT;
        }

        // Determine keyCode
        int keyCode = KEY_UNKNOWN;

        // First, check if key is in the special keys map
        auto it = keyStringToCodeMap.find(key);
        if (it != keyStringToCodeMap.end()) {
            keyCode = it->second;
        }
        else if (key.length() == 1) {
            // Printable characters: map to their ASCII codes
            char ch = key[0];
            keyCode = static_cast<int>(ch);
        }
        else {
            std::cerr << "Unrecognized key: " << key << "\n";
            keyCode = KEY_UNKNOWN;
        }

        return KeyEvent(keyCode, modifiers);
    }
    catch (json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << "\n";
        return KeyEvent(KEY_UNKNOWN, MOD_NONE);
    }
    catch (json::type_error& e) {
        std::cerr << "JSON Type Error: " << e.what() << "\n";
        return KeyEvent(KEY_UNKNOWN, MOD_NONE);
    }
    catch (...) {
        std::cerr << "Unknown error while parsing JSON.\n";
        return KeyEvent(KEY_UNKNOWN, MOD_NONE);
    }
}
