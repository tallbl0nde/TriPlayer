#include "Commands.h"

// Protocol version
const int SM_PROTOCOL_VERSION = 1;
// Character used to separate strings (record separator)
const char SM_DELIMITER = '\x1e';
// Character used to signal end of transmission
const char SM_ENDMSG = '\x1c';