/******************************************************************************
 * logviewer.hpp
 *
 * Utility to display log files in real time on the console.
 *
 * Copyright (C) 2012-2017 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/


#ifndef LOGVIEWER_HPP
#define LOGVIEWER_HPP

#include "LogContext.hpp"
#include "LogFormatter.hpp"
#include "logLevels.h"
#include "progArgs.h"
#include "ReadKeyboard.h"

#include <chrono>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace log_viewer {


struct Compare {
	std::string value;
	int         column;
	bool        comparison;   // false = less than; true = greater than
};


struct ResetDefaults;


class LogViewer
{
public:
	LogViewer();
	LogViewer(const std::string &_logFile, int _minLogLevel = 0);
	LogViewer(int argc, char* argv[]);
	~LogViewer();

	int Run();

	int SetDefaultValues();
	int SetLogFileName(const std::string &_logFile);
	int SetMinLogLevel(int _minLogLevel);
	int SetCommandFile(const std::string &_cmdFile);

	std::string GetLogDate(const std::string &_logFile);

	int CheckLogFilesDiagnostic() const;

	void  PrintHelp(const Utilities::ProgArgs &_args, const char* _progName, LogLevels *_logLevels = 0);
	void  PrintVersion(const char* _progName);
	void  PrintLogFilesDiagnostic(const std::string &_msg) const;

	static const int  version = 6, subversion = 4, subsubversion = 0;
	/* Versioning conventions:
	 *	- Even subversion number: stable version.
	 *	- Odd subversion number: unstable/development version.
	 */

	static const int MSG_MISSING_COMMAND_FILE = 1,
	                 WRN_HTML_OUTPUT_CORRUPTED = -5;

private:
	int SetCommandLineParams();
	int ReadCommandLineParams(int argc, char *argv[]);
	int WriteHeader();
	int WriteHeader_html();
	int WriteLog(const std::string &_log, int _level, const std::string &_file, char _tag = ' ', int _logNumber = -1);
	int WriteFooter();
	int WriteFooter_html();
	int GenerateLogHeader();
	int MoveBackToEndLogsBlock();
	int MoveBackToEndLogsBlock_html();
	int PrintExtraInfo();
	int ReadKeyboard(std::ifstream &ifs, std::streamoff &pos);
	int ReadExternalCommands(std::ifstream &ifs, std::streamoff &pos);
	int AddHtmlControls();

private:

	Utilities::ProgArgs  progArgs;		// command line arguments

	// Files' details

	std::string   logFile;				// input log file name
	std::ifstream inLogFs;				// input file stream where the logs come from

	bool          logToFile;			// (default = false)
	std::string   outLogFile;			// file name for the output stream to redirect the logs (extensions added by logviewer)
	std::string   outLogFileFormat;		// OS shell highlighting, HTML, markdown, ...

	std::fstream  textOutStream;		// text output stream
	std::fstream  htmlOutStream;		// HTML input/output stream

	bool          externalCtrl;
	std::string   cmdFile;				// command file name, to control the program from outside

	// Logs' details

	std::string   delimiters;			// Specify custom delimiters for the messages (default = new line)

	std::string   logHeader;

	bool          printLogFile;			// Print the log file name for each message (useful if multiple log files are shown simultaneously)
	std::string   logFileField;			// log file name to be printed for each log message

	int           logNumber;			// log/line numbers
	int           logNumberField;		// log/line numbers to be printed for each log message
	bool          printLogNumber;		// print the log/line numbers

	bool          textParsing;			// parse the input file as normal text, not as a log file

	// Log levels

	LogLevels     logLevels;			// custom log levels
	int           levelColumn;			// ID of the column which contains the log level (default = -1, i.e. dynamic)
	int           minLevel;				// minimum level a log must have to be shown
	int           beepLevel;			// minimum level to get an audio signal (disabled if < 0)
	bool          warnUnknownLogLevel;	// warning for missing level in a log

	// Output details

	int  verbose;						// amount of extra information to print

	LogFormatter  logFormatter;

	bool          consoleOutput,
	              textFileOutput,
	              htmlOutput,
	              markdownOutput;

	// Filter details

	const int     printAll = -1;
	bool          newLogsOnly;			// only print logs generated from now on
	int           nLatest;				// number of latest logs to be printed (-1 = all)
	int           nLatestChars;			// number of latest characters to be printed (-1 = all)

	std::vector<std::string>  includeStrings,	// must contain the specified substring
							  excludeStrings;	// must not contain the specified substring
	std::string   tempStr;
	bool          incStrFlag,
				  excStrFlag;			// flags to decide whether to check for substrings

	std::vector<Compare>  compare;		// set of comparisons to be done

	LogContext  context;				// logs belonging to the current context

	// Timing and user interaction

	std::chrono::milliseconds  pause;	// pause among a check of the log file and the next (default = 1000)

	utilities::ReadKeyboard  rdKb;
	int                      key;

	int  nLogsReload;					// number of logs to reload when the 'r' key is pressed

	std::string  cmdLineParams;			// command line parameters

	ResetDefaults  *rd;
};



struct ResetDefaults
{
	ResetDefaults() {
		// Reset console colors
#ifdef _WIN32
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#endif
	}

	~ResetDefaults() {
		// Reset console colors
#ifdef _WIN32
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#endif
	}
};


} // log_viewer


#endif // LOGVIEWER_HPP
