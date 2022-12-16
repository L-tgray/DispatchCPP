#ifndef __ARGS_H__
#define __ARGS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cctype>
#include <cstdio>
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include <algorithm>

// Top-level function to call for parsing arguments.
void parseArgs(int numArgs, char ** ppArgs);

// Top-level functions for checking if an arg and/or a value for it exists.
bool argExists     (const std::string & targetArg);
bool argValueExists(const std::string & targetArg);

// Top-level functions for fetching an arg's value in various types.
std::pair<bool, std::string> getArgValueString(const std::string & targetArg);
std::pair<bool, size_t>      getArgValueUInt  (const std::string & targetArg);
std::pair<bool, ssize_t>     getArgValueInt   (const std::string & targetArg);
std::pair<bool, double>      getArgValueFloat (const std::string & targetArg);

#endif // __ARGS_H__
