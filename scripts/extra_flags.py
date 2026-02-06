Import("env")

# Keep default builds quiet without affecting the strict env.
if env.get("PIOENV") == "wt32-eth01":
    # Suppress C-only warning from esp_dmx.
    env.Append(CFLAGS=["-Wno-discarded-qualifiers"])
    # Suppress C++17 register warnings from ArduinoLog.
    env.Append(CXXFLAGS=["-Wno-register"])
