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

/// List of all the possible log tags with their corresponding log levels:

struct TagLevel {
	std::string  tag;
	int          level;

	TagLevel() : tag(""), level(0) {}
	TagLevel(const std::string &_tag, const int _level) : tag(_tag), level(_level) {}
};


class LogLevels
{
public:
	static const int err_levelNotFound = -1,
					 err_fileNotFound  = -2;

public:
	LogLevels() { InitLogLevels(); }

	int InitLogLevels();
	int InitLogLevels(const std::vector<TagLevel> &_levels);
	int AddLogLevels(const std::vector<TagLevel> &_levels);
	int AddLogLevel(const TagLevel &_level);
	int InitLogLevels(const std::string &_levelsFName);
	int AddLogLevels(const std::string &_levelsFName);
	int ClearLogLevels();

	int         GetVal(const std::string &_tag);
	std::string GetTag(int _val);

	int LogLevelMapping(const std::string &_tag);

	// Return the log level tag in a log message; empty string if not found
	std::string FindLogLevelTag(const std::string &_log);

	// Return the log level value in a log message
	int FindLogLevelVal(const std::string &_log);

private:
	std::vector<TagLevel> levels;

	static std::string ToUppercase(const std::string &_str);
};


} // LogViewer

#endif // LOGLEVELS_H

