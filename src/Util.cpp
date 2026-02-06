//
// Created by lnx on 06.02.2026.
//

#include "Util.h"

#include "ArduinoLog.h"

std::string Util::logLevelToString(int logLevel) {
    switch (logLevel) {
        default:
        case 0: return "SILENT ";
        case 1: return "FATAL ";
        case 2: return "ERROR ";
        case 3: return "WARNING ";
        case 4: return "INFO ";
        case 5: return "TRACE ";
        case 6: return "VERBOSE ";
    }
}


void Util::printDMXData(const uint8_t *data, uint16_t size) {
    Log.verboseln("DMX Data: |");
    for (uint16_t i = 0; i < size; ++i) {
        Log.verbose("%d", data[i]);
        if (i < size - 1) {
            Log.verbose("|");
        }
    }
    Log.verboseln("|");
}
