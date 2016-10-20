/******************************************************************************
 * LogFormatter_html.cpp
 *
 * Transform a plain log message into HTML5 format.
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


std::string LogFormatter::HeaderHTML() const
{
	using namespace textModeFormatting;

	std::stringstream htmlHeader;

	htmlHeader << "<!DOCTYPE html>\n"
	           << "<html>\n"
	           << "\n"
	           << "	<head>\n"
	           << "		<link rel=\"stylesheet\" type=\"text/css\" href=\"logviewer.css\">\n"
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


std::string LogFormatter::FooterHTML() const
{
	std::stringstream htmlFooter;

	htmlFooter << "	</body>\n"
	           << "</html>\n"
	           << "\n";

	return htmlFooter.str();

}


} // log_viewer
