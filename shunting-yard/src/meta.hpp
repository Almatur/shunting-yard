#pragma once

//#define NDEBUG
#define LIB_SUPPORT
#define NEG_SUPPORT
#define LIB_PREFIX "imp" // prefix of user-defined functions in loaded libraries
                         // all functions in library should start with this prefix, but user should write function names for parser without prefix

#define LIB_PREFIX_LENGTH 3 // length of prefix
