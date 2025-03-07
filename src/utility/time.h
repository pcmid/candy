// SPDX-License-Identifier: MIT
#ifndef CANDY_UTILITY_TIME_H
#define CANDY_UTILITY_TIME_H

#include <cstdint>

namespace Candy {

class Time {
public:
    static int64_t unixTime();
    static int64_t hostToNet(int64_t host);
    static int64_t netToHost(int64_t net);
};

}; // namespace Candy

#endif
