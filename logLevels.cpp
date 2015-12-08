/******************************************************************************
 * logLevels.cpp
 *
 * Definitions of the log levels as string tokens and numerical values.
 *
 * Copyright (C) 2012-2015 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/

#include "logLevels.h"
#include "textModeFormatting.h"
#include <cstdlib>
#include <string>


namespace LogViewer {

	using namespace textModeFormatting;


	int LogLevels::GetVal(const std::string &_tag)
	{
		if (isdigit(_tag[0]))
			// A number, use it directly
			return std::atoi(_tag.c_str()) % nLevels;

		const std::string tag = LogLevels::ToUppercase(_tag);

		if (tag[0] == 'L' || tag == "NO_LEVEL")
			// The 'L' special character
			return nLevels - 1;

		// Check for string level

		size_t i = 0;

		while (i < levels.size())
		{
			if (tag == levels[i].tag)
				return levels[i].level;
			++i;
		}

		// Nothing found; use a random mapping

		return LogLevelMapping(tag);
	}


	std::string LogLevels::GetTag(int _val)
	{
		size_t i = 0;

		while (i < levels.size())
		{
			if (_val == levels[i].level)
				return levels[i].tag;
			++i;
		}

		return levels[i - 1].tag;
	}


	int LogLevels::LogLevelMapping(const std::string &_tag)
	{
		int colorCode = 0;

		for (size_t i = 0; i < _tag.size(); ++i)
		{
			colorCode += _tag[i];		//+TODO: more randomness
		}

		colorCode = colorCode % 7;		// use the first 7 colors only

		return colorCode;
	}


// Return the log level tag in a log message; empty string if not found
	
std::string LogLevels::FindLogLevelTag(const std::string &_log)
{
	const std::string log = LogLevels::ToUppercase(_log);

	for (size_t i = 0; i < levels.size(); ++i)
	{
		if (log.find(levels[i].tag) != std::string::npos)
		{
			return levels[i].tag;
		}
	}

	return "";
}


// Return the log level value in a log message; -1 if not found

int LogLevels::FindLogLevelVal(const std::string &_log)
{
	const std::string log = LogLevels::ToUppercase(_log);

	for(size_t i = 0; i < levels.size(); ++i)
	{
		if(log.find(levels[i].tag) != std::string::npos)
		{
			return levels[i].level;
		}
	}

	return -1;
}


std::string LogLevels::ToUppercase(const std::string &_str)
{
	std::string uppCase(_str);

	for (size_t i = 0, size = _str.length(); i < size; ++i)
	{
		if (_str[i] >= 'a' && _str[i] <= 'z')
			uppCase[i] = _str[i] + ('A' - 'a');
	}

	return uppCase;
}


} // LogViewer
