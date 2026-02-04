#include "FixtureSettingsManager.h"
#include <ArduinoLog.h>

FixtureSettingsManager::FixtureSettingsManager(const String &namespaceName) : _namespace(namespaceName) {
    if (_namespace.length() > 15) {
        _namespace = _namespace.substring(0, 15);
    }
}

void FixtureSettingsManager::addSetting(const String &key, const String &label, String *valuePtr, const String &type) {
    _bindings.push_back({key, label, type, SettingBinding::STRING, valuePtr});
}

void FixtureSettingsManager::addSetting(const String &key, const String &label, uint32_t *valuePtr,
                                        const String &type) {
    _bindings.push_back({key, label, type, SettingBinding::UINT32, valuePtr});
}

void FixtureSettingsManager::addSetting(const String &key, const String &label, int *valuePtr, const String &type) {
    _bindings.push_back({key, label, type, SettingBinding::INT, valuePtr});
}

void FixtureSettingsManager::addSetting(const String &key, const String &label, uint16_t *valuePtr,
                                        const String &type) {
    _bindings.push_back({key, label, type, SettingBinding::UINT16, valuePtr});
}

void FixtureSettingsManager::load() {
    Preferences prefs;
    prefs.begin(_namespace.c_str(), true);
    for (auto &b: _bindings) {
        if (!prefs.isKey(b.key.c_str())) continue;

        switch (b.valueType) {
            case SettingBinding::STRING:
                *((String *) b.ptr) = prefs.getString(b.key.c_str(), *((String *) b.ptr));
                break;
            case SettingBinding::UINT32:
                *((uint32_t *) b.ptr) = prefs.getUInt(b.key.c_str(), *((uint32_t *) b.ptr));
                break;
            case SettingBinding::INT:
                *((int *) b.ptr) = prefs.getInt(b.key.c_str(), *((int *) b.ptr));
                break;
            case SettingBinding::UINT16:
                *((uint16_t *) b.ptr) = (uint16_t) prefs.getUInt(b.key.c_str(), *((uint16_t *) b.ptr));
                break;
        }
    }
    prefs.end();
}

void FixtureSettingsManager::save() {
    Preferences prefs;
    prefs.begin(_namespace.c_str(), false);
    for (auto &b: _bindings) {
        switch (b.valueType) {
            case SettingBinding::STRING:
                prefs.putString(b.key.c_str(), *((String *) b.ptr));
                break;
            case SettingBinding::UINT32:
                prefs.putUInt(b.key.c_str(), *((uint32_t *) b.ptr));
                break;
            case SettingBinding::INT:
                prefs.putInt(b.key.c_str(), *((int *) b.ptr));
                break;
            case SettingBinding::UINT16:
                prefs.putUInt(b.key.c_str(), *((uint16_t *) b.ptr));
                break;
        }
    }
    prefs.end();
}

bool FixtureSettingsManager::applySetting(const String &key, const String &value) {
    for (auto &b: _bindings) {
        if (b.key == key) {
            switch (b.valueType) {
                case SettingBinding::STRING:
                    *((String *) b.ptr) = value;
                    break;
                case SettingBinding::UINT32:
                    *((uint32_t *) b.ptr) = (uint32_t) value.toInt();
                    break;
                case SettingBinding::INT:
                    *((int *) b.ptr) = value.toInt();
                    break;
                case SettingBinding::UINT16:
                    *((uint16_t *) b.ptr) = (uint16_t) value.toInt();
                    break;
            }
            return true;
        }
    }
    return false;
}

std::vector<FixtureSetting> FixtureSettingsManager::getSettingsMetadata() const {
    std::vector<FixtureSetting> meta;
    for (const auto &b: _bindings) {
        String valStr;
        switch (b.valueType) {
            case SettingBinding::STRING: valStr = *((String *) b.ptr);
                break;
            case SettingBinding::UINT32: valStr = String(*((uint32_t *) b.ptr));
                break;
            case SettingBinding::INT: valStr = String(*((int *) b.ptr));
                break;
            case SettingBinding::UINT16: valStr = String(*((uint16_t *) b.ptr));
                break;
        }
        meta.push_back(FixtureSetting(b.key, b.label, valStr, b.type));
    }
    return meta;
}