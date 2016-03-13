/******************************************************************************
 * logviewer.hpp
 *
 * Utility to display log files in real time on the console.
 *
 * Copyright (C) 2012-2016 Pietro Mele
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

#include <fstream>
#include <string>

namespace log_viewer {


struct Compare {
	std::string value;
	int         column;
	bool        comparison;   // false = less than; true = greater than
};


class LogViewer
{
public:
	LogViewer();
	LogViewer(const std::string &_logFile, int _minLogLevel = 0);
	LogViewer(int argc, char* argv[]);

	int Run();

	int SetDefaultValues();
	int SetLogFileName(const std::string &_logFile);
	int SetMinLogLevel(int _minLogLevel);

	int Start();
	int Pause();
	int Stop();

	std::string GetLogDate(const std::string &_logFile);

	void  PrintHelp(const Utilities::ProgArgs &_args, const char* _progName, LogLevels *_logLevels = 0);
	void  PrintVersion(const char* _progName);

	static const int  version = 4, subversion = 99, subsubversion = 2;
	/* Versioning conventions:
	 *	- Even subversion number: stable version.
	 *	- Odd subversion number: unstable/development version.
	 */

private:

	int SetCommandLineParams();
	int ReadCommandLineParams(int argc, char *argv[]);
	int GenerateLogHeader();
	int PrintExtraInfo();
	int ReadKeyboard(std::ifstream &ifs, std::streamoff &pos);

private:

	//+TMP - Variables to be moved/deleted
	std::string subString;      // Print the logs which contain the specified substring
	std::string notSubString;   // Print the logs which do not contain the specified substring
	int lessThan;               // Print the logs whose i-th token is less than the specified i_value
	int greaterThan;            // Print the logs whose i-th token is greater than the specified i_value
	int contextWidth;           // Number of context logs to show if the current log is above a threshold level (default = 0)
	int minLevelForContext;     // Minimum level a log must have to get a context (default = 5)
	int minContextLevel;        // Minimum level a log must have to be in the context (default = 2)
	//+? std::string logLevels;      // Load custom log levels from file (format: tag value\n)
	int outFileFormat;          //+ Format of the output log file: plain, console, (TODO: HTML, markdown)
	bool restore;               // Restore system in case of problems
	int help;                   // Help

///---

	Utilities::ProgArgs  progArgs;		// command line arguments

	// Files' details

	std::string   logFile;				// input log file name

	bool          logToFile;			// (default = false)
	std::string   outLogFile;			// file name for the output stream to redirect the logs (default = standard output)
	std::string   outLogFileFormat;		// OS shell highlighting, HTML, markdown, ...

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

	// Filter details

	const int     printAll = -1;
	bool          newLogsOnly;			// only print logs generated from now on
	int           nLatest;				// number of latest logs to be printed (-1 = all)
	int           nLatestChars;			// number of latest characters to be printed (-1 = all)

	std::vector<std::string>  includeStrings,
							  excludeStrings;	// sub strings to be included/excluded by the logs
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

	std::string cmdLineParams;			// command line parameters
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
