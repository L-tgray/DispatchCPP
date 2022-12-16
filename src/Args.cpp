#include "Args.h"
using namespace std;
using namespace std::string_literals;

// Convenience type for representing an argument and its value, if provided.
typedef struct __ARG_TYPE__ {
	bool   hasName;
	bool   hasValue;
	string argName;
	string argValue;
} ArgType, * pArgType;

// Convenience type for representing all arguments.
typedef unordered_map<string, ArgType> ArgMap, * pArgMap;

// Convenience function for string conversions.
static string toLower(string str) { transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return(tolower(c)); }); return(str); }
static string toUpper(string str) { transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return(toupper(c)); }); return(str); }

// Convenience function for getting the index of the first occurrence of a string within another string.
static pair<bool, size_t> hasSubstring(const string & targetString, const string & targetSubstring) {
	auto returnValue        = pair<bool, size_t>(false, 0);
	auto targetStringLen    = targetString.length();
	auto targetSubstringLen = targetSubstring.length();
	if ((targetStringLen > 0) && (targetSubstringLen > 0) && (targetSubstringLen <= targetStringLen)) {
		returnValue.first = ((returnValue.second = targetString.find(targetSubstring)) != string::npos);
	}
	return(returnValue);
}

// Convenience function for checking if a string starts with another string.
static bool startsWith(const string & targetString, const string & targetPrefix) {
	auto targetResults = hasSubstring(targetString, targetPrefix);
	return(targetResults.first && (targetResults.second == 0));
}

// Convenience function for splitting a string by some delimiter/separator/token.
static vector<string> splitArg(const string & targetString,
                               const string & argSepString   = "="s,
                               const string & argPrefixLong  = "--"s,
                               const string & argPrefixShort = "-"s) {
	auto returnValue       = vector<string>();
	auto targetStringLen   = targetString.length();
	auto argSepStringLen   = argSepString.length();
	auto argPrefixLongLen  = argPrefixLong.length();
	auto argPrefixShortLen = argPrefixShort.length();
	auto argSepResults     = hasSubstring(targetString, argSepString);
	auto argIndexName      = string::npos;
	auto sepIndexValue     = string::npos;
	auto argIndexValue     = string::npos;

	// Is the arg string long format?
	if ((targetStringLen > argPrefixLongLen) && startsWith(targetString, argPrefixLong)) {
		argIndexName = argPrefixLongLen;

	// Is the arg string short format?
	} else if ((targetStringLen > argPrefixShortLen) && startsWith(targetString, argPrefixShort)) {
		argIndexName = argPrefixShortLen;
	}

	// Is there an arg sep present?
	if ((argIndexName != string::npos) && argSepResults.first && (argSepResults.second > argIndexName)) {
		sepIndexValue = argSepResults.second;
		if ((sepIndexValue + argSepStringLen) < targetStringLen) {
			argIndexValue = (sepIndexValue + argSepStringLen);
		}
	}

	// Do we have a valid index for the arg's name?
	if (argIndexName != string::npos) {
		// The length of the arg's name and value.
		auto argStringNameLen  = ((sepIndexValue != string::npos) ? (sepIndexValue - argIndexName) : (targetStringLen - argIndexName));
		auto argStringValueLen = ((argIndexValue != string::npos) ? (targetStringLen - argIndexValue) : 0);

		// Do we have a valid arg name to grab?
		if (argStringNameLen > 0) {
			returnValue.push_back(targetString.substr(argIndexName, argStringNameLen));

			// Do we have a valid arg value to grab?
			if (argStringValueLen > 0) {
				returnValue.push_back(targetString.substr(argIndexValue));
			}
		}
	}
	return(returnValue);
}

// Convenience function for fetching an argument and its value, if provided.
static ArgType parseArg(const string & targetArg) {
	// Declare our default return values.
	ArgType returnValue = {
		.hasName = false, .hasValue = false,
		.argName = ""s,   .argValue = ""s
	};

	// Attempt to split the arg into its parts.
	auto targetArgParts    = splitArg(targetArg);
	auto targetArgPartsNum = targetArgParts.size();

	// Did we at least get a valid arg name?
	if ((targetArgPartsNum > 0) && (targetArgParts[0].length() > 0)) {
		returnValue.hasName = true;
		returnValue.argName = targetArgParts[0];

		// Did we also get a valid arg value?
		if ((targetArgPartsNum > 1) && (targetArgParts[1].length() > 0)) {
			returnValue.hasValue = true;
			returnValue.argValue = targetArgParts[1];
		}
	}
	return(returnValue);
}

// The map of all args we parse.
static ArgMap allParsedArgs;

// Top-level function to call for parsing arguments.
void parseArgs(int numArgs, char ** ppArgs) {
	// Clear any previous args that still exist.
	allParsedArgs.clear();

	// Iterate over all args besides the first one.
	for (auto argIndex = 1; argIndex < numArgs; ++argIndex) {
		// Attempt to parse this arg, now.
		auto argString  = string(ppArgs[argIndex]);
		auto argDetails = parseArg(argString);

		// Does this arg have a valid name?
		if (argDetails.hasName) {
			// It does! Insert this arg into our map, overwriting any previous arg(s) of the same name.
			allParsedArgs[toLower(argDetails.argName)] = argDetails;
		}
	}
}

// Top-level functions for checking if an arg and/or a value for it exists.
bool argExists(const string & targetArg) {
	return(allParsedArgs.find(toLower(targetArg)) != allParsedArgs.end());
}
bool argValueExists(const string & targetArg) {
	return(argExists(targetArg) ? allParsedArgs[toLower(targetArg)].hasValue : false);
}

// Top-level functions for fetching an arg's value in various types.
pair<bool, string> getArgValueString(const string & targetArg) {
	auto returnValue = pair<bool, string>(false, ""s);
	if (argValueExists(targetArg)) {
		returnValue.first  = true;
		returnValue.second = allParsedArgs[toLower(targetArg)].argValue;
	}
	return(returnValue);
}
pair<bool, size_t> getArgValueUInt(const string & targetArg) {
	auto returnValue    = pair<bool, size_t>(false, 0);
	auto argValueString = getArgValueString(targetArg);
	if (argValueString.first) {
		returnValue.first = (sscanf(argValueString.second.c_str(), "%zu", &(returnValue.second)) == 1);
	}
	return(returnValue);
}
pair<bool, ssize_t> getArgValueInt(const string & targetArg) {
	auto returnValue    = pair<bool, ssize_t>(false, 0);
	auto argValueString = getArgValueString(targetArg);
	if (argValueString.first) {
		returnValue.first = (sscanf(argValueString.second.c_str(), "%zd", &(returnValue.second)) == 1);
	}
	return(returnValue);
}
pair<bool, double> getArgValueFloat(const string & targetArg) {
	auto returnValue    = pair<bool, double>(false, 0.0);
	auto argValueString = getArgValueString(targetArg);
	if (argValueString.first) {
		returnValue.first = (sscanf(argValueString.second.c_str(), "%lf", &(returnValue.second)) == 1);
	}
	return(returnValue);
}
