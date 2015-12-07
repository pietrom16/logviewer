/******************************************************************************
 * logLevels.h
 *
 * Definitions of the log levels as string tokens and numerical values.
 *
 * Copyright (C) 2012-2015 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/

#ifndef LOGLEVELS_H
#define LOGLEVELS_H

#include <string>
#include <array>

namespace LogViewer {

/// List of all the possible log tags with their corresponding log levels:

struct TagLevel {
	std::string  tag;
	int          level;

	TagLevel() : tag(""), level(0) {}
	TagLevel(const std::string &_tag, const int _level) : tag(_tag), level(_level) {}
};


static const std::array<TagLevel, 15> levels =
{
	/* Level tag (case insensitive), Level value */
	TagLevel("VERBOSE",   1),
	TagLevel("TRACE",     1),
	TagLevel("DETAIL",    2),
	TagLevel("DEBUG",     2),
	TagLevel("INFO",      3),
	TagLevel("NOTICE",    3),
	TagLevel("WARNING",   4),
	TagLevel("WARN",      4),
	TagLevel("ERROR",     5),
	TagLevel("ERR",       5),
	TagLevel("CRITICAL",  6),
	TagLevel("SEVERE",    6),
	TagLevel("ALERT",     6),
	TagLevel("FATAL",     7),
	TagLevel("EMERGENCY", 7)
};


class LogLevels
{
public:
	LogLevels() {}

	static int         GetVal(const std::string &_tag);
	static std::string GetTag(int _val);

	static int LogLevelMapping(const std::string &_tag);

	// Return the log level tag in a log message; empty string if not found
	static std::string FindLogLevelTag(const std::string &_log);

	// Return the log level value in a log message; -1 if not found
	static int FindLogLevelVal(const std::string &_log);

private:
	static std::string ToUppercase(const std::string &_str);
};


} // LogViewer

#endif // LOGLEVELS_H

