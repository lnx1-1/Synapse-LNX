#ifndef FIXURE_SETTINGS_MANAGER_H
#define FIXURE_SETTINGS_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <Preferences.h>
#include "../Idmx_FixtureWorker.h"

class FixtureSettingsManager {
public:
    /**
     * @brief Constructor for the Fixture Settings Manager.
     * @param namespaceName The NVS namespace used for storing these settings.
     */
    FixtureSettingsManager(const String &namespaceName);

    /**
     * @brief Adds a string-based setting to the manager.
     * @param key The unique key for the setting.
     * @param label A human-readable label for the setting.
     * @param valuePtr Pointer to the string variable where the setting value is stored.
     * @param type The UI type for this setting (default is "text").
     */
    void addSetting(const String &key, const String &label, String *valuePtr, const String &type = "text");

    /**
     * @brief Adds a uint32_t-based setting to the manager.
     * @param key The unique key for the setting.
     * @param label A human-readable label for the setting.
     * @param valuePtr Pointer to the uint32_t variable where the setting value is stored.
     * @param type The UI type for this setting (default is "number").
     */
    void addSetting(const String &key, const String &label, uint32_t *valuePtr, const String &type = "number");

    /**
     * @brief Adds an int-based setting to the manager.
     * @param key The unique key for the setting.
     * @param label A human-readable label for the setting.
     * @param valuePtr Pointer to the int variable where the setting value is stored.
     * @param type The UI type for this setting (default is "number").
     */
    void addSetting(const String &key, const String &label, int *valuePtr, const String &type = "number");

    /**
     * @brief Adds a uint16_t-based setting to the manager.
     * @param key The unique key for the setting.
     * @param label A human-readable label for the setting.
     * @param valuePtr Pointer to the uint16_t variable where the setting value is stored.
     * @param type The UI type for this setting (default is "number").
     */
    void addSetting(const String &key, const String &label, uint16_t *valuePtr, const String &type = "number");

    /**
     * @brief Loads all registered settings from NVS.
     */
    void load();

    /**
     * @brief Saves all registered settings to NVS.
     */
    void save();

    /**
     * @brief Updates a setting value by key and saves it.
     * @param key The key of the setting to update.
     * @param value The new value as a string (will be converted if necessary).
     * @return True if the setting was found and updated, false otherwise.
     */
    bool applySetting(const String &key, const String &value);

    /**
     * @brief Retrieves metadata for all registered settings.
     * @return A vector of FixtureSetting objects.
     */
    std::vector<FixtureSetting> getSettingsMetadata() const;

private:
    struct SettingBinding {
        String key;
        String label;
        String type;

        enum ValueType { STRING, UINT32, INT, UINT16 } valueType;

        void *ptr;
    };

    String _namespace;
    std::vector<SettingBinding> _bindings;
};

#endif