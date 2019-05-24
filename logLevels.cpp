/******************************************************************************
 * logLevels.cpp
 *
 * Definitions of the log levels as string tokens and numerical values.
 *
 * Copyright (C) 2012-2016 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/

#include "logLevels.h"
#include "textModeFormatting.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

namespace log_viewer {

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

	pickFirstTag = false;
	warnUnknownLogLevel = false;

	return int(levels.size());
}


int LogLevels::InitLogLevels(const std::vector<TagLevel> &_levels)
{
	levels = _levels;
	MakeAllUppercase();

	warnUnknownLogLevel = false;
	return int(levels.size());
}


int LogLevels::AddLogLevels(const std::vector<TagLevel> &_levels)
{
	for(size_t i = 0; i < _levels.size(); ++i)
	{
		levels.push_back(_levels[i]);
	}

	MakeAllUppercase();

	return int(levels.size());
}


int LogLevels::AddLogLevel(const TagLevel &_level)
{
	levels.push_back(_level);
	levels.back().tag = LogLevels::ToUppercase(levels.back().tag);

	return int(levels.size());
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
		levels.back().tag = LogLevels::ToUppercase(levels.back().tag);
	}

	return int(levels.size());
}


int LogLevels::ClearLogLevels()
{
	levels.clear();
	return int(levels.size());
}


int LogLevels::GetVal(const std::string &_tag) const
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


std::string LogLevels::GetTag(int _val) const
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


std::string LogLevels::GetTags(int _val) const
{
	std::string tags;
	size_t      i = 0;

	while (i < levels.size())
	{
		if (_val == levels[i].level) {
			tags += levels[i].tag;
			tags += " ";
		}
		++i;
	}

	return tags;
}


// Return number of distinct value log levels

size_t LogLevels::NLevels() const
{
	std::set<int> vals;

	for(size_t i = 0; i < levels.size(); ++i)
	{
		vals.insert(levels[i].level);
	}

	return vals.size();
}


int LogLevels::LogLevelMapping(const std::string &_tag) const
{
	int colorCode = 0;

	for (size_t i = 0; i < _tag.size(); ++i)
	{
		colorCode += _tag[i];		//+TODO: more randomness
	}

	colorCode = colorCode % 7;		// use the first 7 colors only

	return colorCode;
}


// Return log level tag and value in a log message;
// empty string/negative value if not found

int LogLevels::FindLogLevel(const std::string &_log,
							std::string &_levelTag,
							bool _pickFirstTag,
							int _column)
{
	int levelVal = 0;

	if(_column >= 0)     // index based log level search
	{
		std::string token;
		std::stringstream str(_log);
		for(int i = 0; i < _column; ++i)
			str >> token;

		levelVal = GetVal(token);
		_levelTag = token;
	}
	else                 // tag based log level search
	{
		levelVal = FindLogLevelVal(_log, _pickFirstTag);

		if(levelVal < 0 && multiLineLogs) {
			levelVal = prevLevel;
		}
		else if(levelVal < 0 && warnUnknownLogLevel && _log.size() > 0) {
			levelVal = 4;
#ifdef _WIN32
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#endif
			std::cerr << "Found log with no recognized log level. Level set to WARNING/4." << std::endl;
		}

		_levelTag = levels[levelVal].tag;
	}

	prevLevel = levelVal;

	return levelVal;
}


// Return log level value in a log message;
// negative value if not found

int LogLevels::FindLogLevel(const std::string &_log,
							bool _pickFirstTag,
							int _column)
{
	int levelVal = 0;

	if(_column >= 0)     // index based log level search
	{
		std::string token;
		std::stringstream str(_log);
		for(int i = 0; i < _column; ++i)
			str >> token;

		levelVal = GetVal(token);
	}
	else                 // tag based log level search
	{
		levelVal = FindLogLevelVal(_log, _pickFirstTag);

		if(levelVal < 0 && multiLineLogs)
		{
			levelVal = prevLevel;
		}
		else if(levelVal < 0)
		{
			if(warnUnknownLogLevel && _log.size() > 0)
			{
				levelVal = 4;

#ifdef _WIN32
				SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#endif
				std::cerr << "Found log with no recognized log level. Level set to WARNING/4." << std::endl;
			}
			else
				levelVal = 0;
		}
	}

	prevLevel = levelVal;

	return levelVal;
}


// Return the log level tag in a log message; empty string if not found.
// In case of multiple levels, return the highest one.

std::string LogLevels::FindLogLevelTag(const std::string &_log,
									   bool _pickFirstTag,
									   int _column) const
{
	int val = -1, newVal = -1;
	std::string tag;

	const std::string log = LogLevels::ToUppercase(_log);

	for(size_t i = 0; i < levels.size(); ++i)
	{
		if(log.find(levels[i].tag) != std::string::npos)
		{
			newVal = levels[i].level;
			if(newVal > val) {
				val = newVal;
				tag = levels[i].tag;
			}
			if(_pickFirstTag)
				return tag;
		}
	}

	return tag;
}


// Return the log level value in a log message; err_levelNotFound if not found.
// In case of multiple levels, return the highest one.

int LogLevels::FindLogLevelVal(const std::string &_log,
							   bool _pickFirstTag,
							   int _column) const
{
	int val = -1, newVal = -1;

	const std::string log = LogLevels::ToUppercase(_log);

	for(size_t i = 0; i < levels.size(); ++i)
	{
		if(log.find(levels[i].tag) != std::string::npos)
		{
			newVal = levels[i].level;
			if(newVal > val)
				val = newVal;
			if(_pickFirstTag)
				return val;
		}
	}

	if(val >= 0)
		return val;

	return err_levelNotFound;
}


void LogLevels::MakeAllUppercase()
{
	for(size_t i = 0; i < levels.size(); ++i)
	{
		levels[i].tag = LogLevels::ToUppercase(levels[i].tag);
	}
}


int LogLevels::FindIndentation()
{
	indentation = 0;

	for(size_t i = 0; i < levels.size(); ++i)
	{
		if(int(levels[i].tag.size()) > indentation)
			indentation = int(levels[i].tag.size());
	}

	return indentation;
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


} // log_viewer
