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
#include <fstream>

namespace LogViewer {

using namespace textModeFormatting;


int LogLevels::InitLogLevels()
{
	/* Level tag (case insensitive), Level value */
	levels.push_back(TagLevel("VERBOSE",   1));
	levels.push_back(TagLevel("TRACE",     1));
	levels.push_back(TagLevel("DETAIL",    2));
	levels.push_back(TagLevel("DEBUG",     2));
	levels.push_back(TagLevel("INFO",      3));
	levels.push_back(TagLevel("NOTICE",    3));
	levels.push_back(TagLevel("WARNING",   4));
	levels.push_back(TagLevel("WARN",      4));
	levels.push_back(TagLevel("ERROR",     5));
	levels.push_back(TagLevel("ERR",       5));
	levels.push_back(TagLevel("CRITICAL",  6));
	levels.push_back(TagLevel("SEVERE",    6));
	levels.push_back(TagLevel("ALERT",     6));
	levels.push_back(TagLevel("FATAL",     7));
	levels.push_back(TagLevel("EMERGENCY", 7));

	return levels.size();
}


int LogLevels::InitLogLevels(const std::vector<TagLevel> &_levels)
{
	levels = _levels;
	return levels.size();
}


int LogLevels::AddLogLevels(const std::vector<TagLevel> &_levels)
{
	for(size_t i = 0; i < _levels.size(); ++i)
	{
		levels.push_back(_levels[i]);
	}

	return levels.size();
}


int LogLevels::AddLogLevel(const TagLevel &_level)
{
	levels.push_back(_level);
	return levels.size();
}


int LogLevels::InitLogLevels(const std::string &_levelsFName)
{
	{
		std::ifstream ifs(_levelsFName);
		if(ifs.fail())
			return err_fileNotFound;
	}

	ClearLogLevels();
	return AddLogLevels(_levelsFName);
}


int LogLevels::AddLogLevels(const std::string &_levelsFName)
{
	std::ifstream ifs(_levelsFName);
	std::string   tag;
	int           level;

	if(ifs.fail())
		return err_fileNotFound;

	while(ifs.good())
	{
		ifs >> tag;
		ifs >> level;
		levels.push_back(TagLevel(tag, level));
	}

	return levels.size();
}


int LogLevels::ClearLogLevels()
{
	levels.clear();
	return levels.size();
}


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


// Return the log level value in a log message; err_levelNotFound if not found

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

	return err_levelNotFound;
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
