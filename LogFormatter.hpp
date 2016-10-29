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
	/** _formats: a space separated list of file formats (console, text, html, ...)
	 */

public:
	LogFormatter();
	LogFormatter(std::string &_formats);
	LogFormatter(std::string &_formats, const std::string &_title);

	void SetTitle(const std::string &_title) { title = _title; }

	int SetFormats(std::string &_formats);
	int SetFormats(std::string &_formats, bool &_consoleOutput, bool &_textFileOutput, bool &_htmlOutput, bool &_markdownOutput);
	int SetFormats(const char *_formats);
	std::string GetFormats() const { return formats; }

	std::string AvailableFormats() const { return availableFormats; }
	std::string DefaultFormats()   const { return defaultFormats;   }

	// Filter invalid formats; return number of valid formats
	int CheckFormats(std::string &_formats) const;

	// Log message formatters
	std::string Format(const std::string &_log, int _level, const std::string &_file, char _tag = ' ', int _logNumber = -1) const;
	std::string FormatPlain   (const std::string &_log, int _level, const std::string &_file, char _tag = ' ', int _logNumber = -1) const;
	std::string FormatConsole (const std::string &_log, int _level, const std::string &_file, char _tag = ' ', int _logNumber = -1) const;
	std::string FormatHTML    (const std::string &_log, int _level, const std::string &_file, char _tag = ' ', int _logNumber = -1) const;
	std::string FormatMarkdown(const std::string &_log, int _level, const std::string &_file, char _tag = ' ', int _logNumber = -1) const;

	// Headers
	std::string Header()         const;
	std::string HeaderPlain()    const { return title; }
	std::string HeaderConsole()  const { return title; }
	std::string HeaderHTML()     const;
	std::string TitleHTML()      const;
	std::string HeaderMarkdown() const { return title; }

	// Footers
	std::string Footer()         const;
	std::string FooterPlain()    const { return ""; }
	std::string FooterConsole()  const { return ""; }
	std::string FooterHTML()     const;
	std::string FooterMarkdown() const { return ""; }

private:
	static const std::string availableFormats;
	static std::string defaultFormats;

	bool formatPlain, formatConsole, formatHtml, formatMarkdown;

	std::string formats;
	std::string title;

	const std::string cssFile = "logviewer.css";
};


} // log_viewer


#endif // LOG_FORMATTER_HPP
