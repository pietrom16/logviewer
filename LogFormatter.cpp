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
std::string LogFormatter::defaultFormats = "console";


LogFormatter::LogFormatter()
    : formatPlain(false), formatConsole(false), formatHtml(false), formatMarkdown(false)
{
	SetFormats(defaultFormats);
}


LogFormatter::LogFormatter(std::string &_formats)
    : formatPlain(false), formatConsole(false), formatHtml(false), formatMarkdown(false)
{
	SetFormats(_formats);
}


LogFormatter::LogFormatter(std::string &_formats,
                           const std::string &_title)
    : formatPlain(false), formatConsole(false), formatHtml(false), formatMarkdown(false)
{
	SetFormats(_formats);
	SetTitle(_title);
}


int LogFormatter::SetFormats(std::string &_formats)
{
	int nValidFormats = CheckFormats(_formats);

	if(nValidFormats > 0)
		formats = _formats;
	else
	{
		formats = defaultFormats;
		nValidFormats = -1;
	}

	if(formats.find("plain") != std::string::npos)  formatPlain = true;
	else                                            formatPlain = false;

	if(formats.find("console") != std::string::npos)  formatConsole = true;
	else                                              formatConsole = false;

	if(formats.find("HTML") != std::string::npos ||
	   formats.find("html") != std::string::npos)     formatHtml = true;
	else                                              formatHtml = false;

	if(formats.find("markdown") != std::string::npos)  formatMarkdown = true;
	else                                               formatMarkdown = false;

	return nValidFormats;
}


int LogFormatter::SetFormats(std::string &_formats, bool &_consoleOutput, bool &_textFileOutput, bool &_htmlOutput, bool &_markdownOutput)
{
	int r = SetFormats(_formats);

	_consoleOutput = formatConsole;
	_textFileOutput = formatPlain;
	_htmlOutput = formatHtml;
	_markdownOutput = formatMarkdown;

	return r;
}


int LogFormatter::SetFormats(const char *_formats)
{
	std::string f(_formats);
	return SetFormats(f);
}


// Filter invalid formats; return number of valid formats

int LogFormatter::CheckFormats(std::string &_formats) const
{
	int          nValidFormats = 0;
	std::string  validFormats;

	std::stringstream formats(_formats);
	std::string format;

	while(formats >> format)
	{
		if(availableFormats.find(format) != std::string::npos) {
			++nValidFormats;
			validFormats.append(format);
			validFormats.append(" ");
		}
	}

	_formats = validFormats;

	return nValidFormats;
}


// Log message formatters

std::string LogFormatter::Format(const std::string &_log,
								 int _level,
								 const std::string &_file,
								 char _tag,
								 int _logNumber) const
{
	if(formatPlain)         return FormatPlain(_log, _level, _file, _tag, _logNumber);
	else if(formatConsole)  return FormatConsole(_log, _level, _file, _tag, _logNumber);
	else if(formatHtml)     return FormatHTML(_log, _level, _file, _tag, _logNumber);
	else if(formatMarkdown) return FormatMarkdown(_log, _level, _file, _tag, _logNumber);

	return FormatConsole(_log, _level, _file, _tag, _logNumber);
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

	htmlLog += "\t\t";		// indent with the <body> block

	htmlLog += "<br>";

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
	           << "		<link rel="stylesheet" type="text/css" href="logviewer.css">\n"
	           << "		<meta charset=\"UTF-8\">\n"
	           << "		<title>" << title << "</title>\n"
	           << "	</head>\n"
	           << "\n"
	           << "	<body>\n";

	return htmlHeader.str();
}


std::string LogFormatter::TitleHTML() const
{
	using namespace textModeFormatting;

	std::stringstream htmlTitle;

	htmlTitle << "		<span style=\"color:grey;\">\n"
	          << "			<br>------------------------------------------------------------\n"
	          << "			<br>" << title << "\n"
	          << "			<br>------------------------------------------------------------\n"
	          << "		</span>\n";

	return htmlTitle.str();
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
