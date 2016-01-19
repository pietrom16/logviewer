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


namespace LogViewer {


const std::string LogFormatter::availableFormats = "plain console HTML markdown";


LogFormatter::LogFormatter()
{
	SetFormat("plain");
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
		format = "plain";
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

std::string LogFormatter::operator[] (const std::string &_input) const
{
	if(format == "plain")         return FormatPlain(_input);
	else if(format == "console")  return FormatConsole(_input);
	else if(format == "HTML")     return FormatHTML(_input);
	else if(format == "markdown") return FormatMarkdown(_input);

	return FormatPlain(_input);
}


std::string LogFormatter::FormatPlain(const std::string &_input) const
{
	return _input;
}


//+TODO
std::string LogFormatter::FormatConsole(const std::string &_input) const{return _input;}
std::string LogFormatter::FormatHTML(const std::string &_input) const{return _input;}
std::string LogFormatter::FormatMarkdown(const std::string &_input) const{return _input;}


} // LogViewer
