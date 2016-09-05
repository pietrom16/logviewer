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


namespace log_viewer {


const std::string LogFormatter::availableFormats = "plain console HTML markdown";
const std::string LogFormatter::defaultFormats = "plain";


LogFormatter::LogFormatter()
{
	SetFormats(defaultFormats);
}


LogFormatter::LogFormatter(const std::string &_formats)
{
	SetFormats(_formats);
}


LogFormatter::LogFormatter(const std::string &_formats,
                           const std::string &_title)
{
	SetFormats(_formats);
	SetTitle(_title);
}


int LogFormatter::SetFormats(const std::string &_formats)
{
	int success = 0;

	if(CheckFormats(_formats))
		formats = _formats;
	else
	{
		formats = defaultFormats;
		success = -1;
	}

	return success;
}


bool LogFormatter::CheckFormats(const std::string &_formats) const
{
	std::stringstream formats(_formats);
	std::string format;

	while(formats >> format)
	{
		if(availableFormats.find(format) == std::string::npos)
			return false;
	}

	return true;
}


// Log message formatters

std::string LogFormatter::Format(const std::string &_log,
								 int _level,
								 const std::string &_file,
								 char _tag,
								 int _logNumber) const
{
	if(formats == "plain")         return FormatPlain(_log, _level, _file, _tag, _logNumber);
	else if(formats == "console")  return FormatConsole(_log, _level, _file, _tag, _logNumber);
	else if(formats == "HTML")     return FormatHTML(_log, _level, _file, _tag, _logNumber);
	else if(formats == "markdown") return FormatMarkdown(_log, _level, _file, _tag, _logNumber);

	return FormatPlain(_log, _level, _file, _tag, _logNumber);
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

	if(_logNumber > 0)
		fLog += std::to_string(_logNumber) + ": ";

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

	if(_logNumber > 0)
		fLog += std::to_string(_logNumber) + ": ";

#ifndef _WIN32
	fLog += std::string(Format(ny));
#endif

	fLog +=   _tag
			+ std::string(Format(_level))
			+ _log
			+ std::string(Reset());

	return fLog;
}


std::string LogFormatter::FormatHTML(const std::string &_log,
									 int _level,
									 const std::string &_file,
                                     char _tag,
                                     int _logNumber) const
{
	using namespace textModeFormatting;

	std::string htmlLog;

	if(!_file.empty())
		htmlLog += _file + ": ";

	if(_logNumber > 0)
		htmlLog += std::to_string(_logNumber) + ": ";

	htmlLog +=   _tag
	           + std::string("<span style=\"")
	           + htmlLevel[_level]
	           + std::string("\">")
	           + _log
	           + std::string("</span>");

	return htmlLog;
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


// Headers

std::string LogFormatter::Header() const
{
	if(formats == "plain")         return HeaderPlain();
	else if(formats == "console")  return HeaderConsole();
	else if(formats == "HTML")     return HeaderHTML();
	else if(formats == "markdown") return HeaderMarkdown();

	return HeaderPlain();
}


std::string LogFormatter::HeaderHTML() const
{
	using namespace textModeFormatting;

	std::stringstream htmlHeader;

	htmlHeader << "<!DOCTYPE html>\n"
	           << "<html>\n"
	           << "\n"
	           << "	<head>\n"
	           << "		<meta charset=\"UTF-8\">\n"
	           << "		<title>Log file</title>\n"
	           << "		<style>\n"
	           << "			body { background-color:black }\n"
	           << "			body { font-family: \"courier new\"; }\n"
	           << "		</style>\n"
	           << "	</head>\n"
	           << "\n"
	           << "	<body>\n"
	           << "		<span style=\"color:grey;\">\n"
	           << "			<br>------------------------------------------------------------\n"
	           << "			<br>" << title << "\n"
	           << "			<br>------------------------------------------------------------\n"
	           << "		</span>\n"
	           << "\n";

	return htmlHeader.str();
}


// Footers

std::string LogFormatter::Footer() const
{
	if(formats == "plain")         return FooterPlain();
	else if(formats == "console")  return FooterConsole();
	else if(formats == "HTML")     return FooterHTML();
	else if(formats == "markdown") return FooterMarkdown();

	return FooterPlain();
}


std::string LogFormatter::FooterHTML() const
{
	std::stringstream htmlFooter;

	htmlFooter << "	</body>\n"
	           << "</html>\n"
	           << "\n";

	return htmlFooter.str();

}


} // log_viewer
