#ifndef SYSMODULE_COMMANDS_H
#define SYSMODULE_COMMANDS_H

// This file stores the code for each command sent/received due to communication
// with the sysmodule.

// Protocol version
extern const int SM_PROTOCOL_VERSION;

// Commands
enum SM_Command {
    VERSION,        // Version command will always have first byte set to 0x0
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
extern const char SM_DELIMITER;
// Character used to signal end of transmission
extern const char SM_ENDMSG;

#endif