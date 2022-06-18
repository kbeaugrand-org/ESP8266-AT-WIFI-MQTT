#ifndef PLATFORM_LOGGER_HPP_
#define PLATFORM_LOGGER_HPP_

#define LOG_LEVEL LOG_LEVEL_DEBUG
#define LOG_ECHO_EN_DEFAULT true

#include <CircularBufferLogger.h>

using PlatformLogger =
    PlatformLogger_t<CircularLogBufferLogger<256>>;

#endif