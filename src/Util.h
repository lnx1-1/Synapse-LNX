//
// Created by lnx on 06.02.2026.
//

#ifndef KALEO_MODULARARTNETNODE_UTIL_H
#define KALEO_MODULARARTNETNODE_UTIL_H
#include <string>


class Util {
public:
    static std::string logLevelToString(int log_level);

    static void printDMXData(const uint8_t *data, uint16_t size);
};


#endif //KALEO_MODULARARTNETNODE_UTIL_H
