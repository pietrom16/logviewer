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
#include <vector>

namespace LogViewer {

struct LogLevel
{
	LogLevel(const std::string &_tag, int _val) : tag(_tag), value(_val) {}

	std::string tag;		// case insensitive
	int         value;
};


/// List of all the possible log tags with their corresponding log levels:

const std::vector<LogLevel> logLevels =
	{
		LogLevel("VERBOSE",   1),
		LogLevel("TRACE",     1),
		LogLevel("DETAIL",    2),
		LogLevel("DEBUG",     2),
		LogLevel("INFO",      3),
		LogLevel("NOTICE",    3),
		LogLevel("WARNING",   4),
		LogLevel("WARN",      4),
		LogLevel("ERROR",     5),
		LogLevel("SEVERE",    6),
		LogLevel("CRITICAL",  6),
		LogLevel("ALERT",     6),
		LogLevel("FATAL",     7),
		LogLevel("EMERGENCY", 7)
	};

}

#endif // LOGLEVELS_H

