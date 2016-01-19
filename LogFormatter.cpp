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


const std::string LogFormatter::availableFormats = "plain console HTML";


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


// Formatter
std::string LogFormatter::operator[] (const std::string &_input) const
{

	return ""; //+TODO
}


bool LogFormatter::CheckFormat(const std::string &_format) const
{
	if(availableFormats.find(_format))	//+TODO
		return true;

	return false;
}



} // LogViewer
