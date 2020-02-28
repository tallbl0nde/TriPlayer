#ifndef SYSMODULE_COMMANDS_HPP
#define SYSMODULE_COMMANDS_HPP

#include <string>

// This file stores the code for each command sent/received due to communication
// with the sysmodule.

// Protocol version
const int SM_PROTOCOL_VERSION = 1;

// Commands
enum SM_Command {
    RESUME,
    PAUSE,
    PREVIOUS,
    NEXT,
    GETVOLUME,
    SETVOLUME,
    PLAY,
    ADDTOQUEUE,
    REMOVEFROMQUEUE,
    GETQUEUE,
    SETQUEUE,
    SHUFFLE,
    REPEATON,
    REPEATOFF,
    GETSONG,
    GETSTATUS,
    RESET
};

// Character used to separate strings (record separator)
const char SM_DELIMITER = '\x1e';

#endif