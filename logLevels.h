/******************************************************************************
 * logLevels.h
 *
 * Definitions of the log levels as string tokens and numerical values.
 *
 * Copyright (C) 2012-2019 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/

#ifndef LOGLEVELS_H
#define LOGLEVELS_H

#include <string>
#include <vector>

namespace log_viewer {

/// List of all the possible log tags with their corresponding log levels:

struct TagLevel {
	std::string  tag;		// will be converted to uppercase
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
	LogLevels()
	{
		InitLogLevels();
		multiLineLogs = false;
		prevLevel = 0;
	}
	
	void SetMultiLineLogs(bool multiLine = true) { multiLineLogs = multiLine; }

	int InitLogLevels();
	int InitLogLevels(const std::vector<TagLevel> &_levels);
	int AddLogLevels(const std::vector<TagLevel> &_levels);
	int AddLogLevel(const TagLevel &_level);
	int InitLogLevels(const std::string &_levelsFName);
	int AddLogLevels(const std::string &_levelsFName);
	int ClearLogLevels();

	int         GetVal(const std::string &_tag) const;
	std::string GetTag(int _val) const;
	std::string GetTags(int _val) const;
	size_t      size() const { return levels.size(); }
	size_t      NLevels() const;  // number of distinct value log levels
	int         Indentation() const { return indentation; }

	int LogLevelMapping(const std::string &_tag) const;

	// Return log level tag and value in a log message;
	// empty string/negative value if not found
	int FindLogLevel(const std::string &_log, std::string &_levelTag,
					 bool _pickFirstTag = false,
					 int _column = -1);

	// Return log level value in a log message;
	// negative value if not found
	int FindLogLevel(const std::string &_log,
					 bool _pickFirstTag = false,
					 int _column = -1);

	// Return the log level tag in a log message; empty string if not found
	std::string FindLogLevelTag(const std::string &_log,
								bool _pickFirstTag = false,
								int _column = -1) const;

	// Return the log level value in a log message
	int FindLogLevelVal(const std::string &_log,
						bool _pickFirstTag = false,
						int _column = -1) const;

	void EnableWarnings(bool _enable = true) {
		warnUnknownLogLevel = _enable;
	}

	void MakeAllUppercase();
	int  FindIndentation();

private:
	std::vector<TagLevel> levels;

	bool pickFirstTag = false;			// pick the highest level tag if false
	bool warnUnknownLogLevel = false;
	int  indentation = 0;

	bool multiLineLogs = true;			// log messages spanning multiple lines
	int  prevLevel = 0;					// level of the multi-line log

	static std::string ToUppercase(const std::string &_str);
};


} // log_viewer

#endif // LOGLEVELS_H

