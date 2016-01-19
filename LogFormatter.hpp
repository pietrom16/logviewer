/******************************************************************************
 * LogFormatter.hpp
 *
 * Get a plain log message and format it according to its destination (e.g. text, HTML, ...).
 *
 * Copyright (C) 2012-2016 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/

#ifndef LOG_FORMATTER_HPP
#define LOG_FORMATTER_HPP

#include <string>

namespace LogViewer {


class LogFormatter
{
public:
	LogFormatter();
	LogFormatter(const std::string &_format);

	int SetFormat(const std::string &_format);
	std::string GetFormat() const { return format; }

	std::string AvailableFormats() const { return availableFormats; }
	bool CheckFormat(const std::string &_format) const;

	// Formatters
	std::string operator[] (const std::string &_input) const;

	std::string FormatPlain(const std::string &_input) const;
	std::string FormatConsole(const std::string &_input) const;
	std::string FormatHTML(const std::string &_input) const;
	std::string FormatMarkdown(const std::string &_input) const;

private:

	static const std::string availableFormats;

	std::string format;
};


} // LogViewer


#endif // LOG_FORMATTER_HPP
