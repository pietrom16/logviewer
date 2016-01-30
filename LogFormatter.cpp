/******************************************************************************
 * LogFormatter.cpp
 *
 * Get a plain log message and format it according to its destination (e.g. text, HTML, ...).
 *
 * Copyright (C) 2012-2016 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/

#include "LogFormatter.hpp"
#include "textModeFormatting.h"
#include <sstream>


namespace LogViewer {


const std::string LogFormatter::availableFormats = "plain console HTML markdown";
const std::string LogFormatter::defaultFormat = "plain";


LogFormatter::LogFormatter()
{
	SetFormat(defaultFormat);
}


LogFormatter::LogFormatter(const std::string &_format)
{
	SetFormat(_format);
}


int LogFormatter::SetFormat(const std::string &_format)
{
	int success = 0;

	if(CheckFormat(_format))
		format = _format;
	else
	{
		format = defaultFormat;
		success = -1;
	}

	return success;
}


bool LogFormatter::CheckFormat(const std::string &_format) const
{
	if(availableFormats.find(_format) == std::string::npos)
		return false;

	return true;
}


// Formatters

std::string LogFormatter::Format(const std::string &_log,
								 int _level,
								 const std::string &_file,
								 char _tag,
								 int _logNumber) const
{
	if(format == "plain")         return FormatPlain(_log, _level, _file, _tag);
	else if(format == "console")  return FormatConsole(_log, _level, _file, _tag);
	else if(format == "HTML")     return FormatHTML(_log, _level, _file, _tag);
	else if(format == "markdown") return FormatMarkdown(_log, _level, _file, _tag);

	return FormatPlain(_log, _level, _file, _tag);
}


std::string LogFormatter::FormatPlain(const std::string &_log,
									  int _level,
									  const std::string &_file,
									  char _tag,
									  int _logNumber) const
{
	std::string fLog;

	if(!_file.empty())
		fLog += _file + ": ";

	fLog += _tag + _log;

	return fLog;
}


std::string LogFormatter::FormatConsole(const std::string &_log,
										int _level,
										const std::string &_file,
										char _tag,
										int _logNumber) const
{
	using namespace textModeFormatting;
	using textModeFormatting::Format;

	std::string fLog(Reset());

	if(!_file.empty())
		fLog += _file + ": ";

	fLog += std::string(Format(ny))
			+ _tag
			+ std::string(Format(_level))
			+ _log
			+ std::string(Reset());

	return fLog;
}


std::string LogFormatter::FormatHTML(const std::string &_log,
									 int _level,
									 const std::string &_file,
									 char _tag, int _logNumber) const
{
	//+TODO
	return _log;
}


std::string LogFormatter::FormatMarkdown(const std::string &_log,
										 int _level,
										 const std::string &_file,
										 char _tag,
										 int _logNumber) const
{
	//+TODO
	return _log;
}


} // LogViewer
