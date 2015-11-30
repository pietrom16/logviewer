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
#include <map>

namespace LogViewer {

class LogLevels
{
	/// List of all the possible log tags with their corresponding log levels:

	static const std::map<std::string, const int> levels;

public:
	LogLevels() {}

	static int LogLevelVal(const std::string &_tag);		//+TODO
	static std::string LogLevelTag(int _val);				//+TODO

	// Return the log level tag in a log message; empty string if not found
	static std::string FindLogLevelTag(const std::string &_log);		//+TODO

	// Return the log level value in a log message; -1 if not found
	static int FindLogLevelVal(const std::string &_log);				//+TODO

	static bool LessThan(const std::string &_tag1, const std::string &_tag2) {
//+		return levels(_tag1) < levels(_tag2);
	}

};


const std::map<std::string, const int> LogLevels::levels =
		{
			/* Level tag, Level value */
			{"VERBOSE",   1},
			{"TRACE",     1},
			{"DETAIL",    2},
			{"DEBUG",     2},
			{"INFO",      3},
			{"NOTICE",    3},
			{"WARNING",   4},
			{"WARN",      4},
			{"ERROR",     5},
			{"SEVERE",    6},
			{"CRITICAL",  6},
			{"ALERT",     6},
			{"FATAL",     7},
			{"EMERGENCY", 7}
		};


} // LogViewer

#endif // LOGLEVELS_H

