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

namespace log_viewer {


class LogFormatter
{
public:
	LogFormatter();
	LogFormatter(const std::string &_format);

	int SetFormat(const std::string &_format);
	std::string GetFormat() const { return format; }

	std::string AvailableFormats() const { return availableFormats; }
	std::string DefaultFormat()    const { return defaultFormat;    }
	bool CheckFormat(const std::string &_format) const;

	// Formatters
	std::string Format(const std::string &_log, int _level, const std::string &_file, char _tag = ' ', int _logNumber = -1) const;

	std::string FormatPlain   (const std::string &_log, int _level, const std::string &_file, char _tag = ' ', int _logNumber = -1) const;
	std::string FormatConsole (const std::string &_log, int _level, const std::string &_file, char _tag = ' ', int _logNumber = -1) const;
	std::string FormatHTML    (const std::string &_log, int _level, const std::string &_file, char _tag = ' ', int _logNumber = -1) const;
	std::string FormatMarkdown(const std::string &_log, int _level, const std::string &_file, char _tag = ' ', int _logNumber = -1) const;

private:

	static const std::string availableFormats;
	static const std::string defaultFormat;

	std::string format;
};


} // log_viewer


#endif // LOG_FORMATTER_HPP
