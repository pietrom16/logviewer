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

#include <fstream>
#include <string>

namespace log_viewer {


class LogViewer
{
public:
	LogViewer();
	LogViewer(const std::string &_logFile, int _minLogLevel = 0);
	LogViewer(int argc, char* argv[]);

	int SetDefaultValues();
	int SetLogFileName(const std::string &_logFile);
	int SetMinLogLevel(int _minLogLevel);

	int Start();
	int Pause();
	int Stop();


private:

	int levelCol;               // ID of the column which contains the log level (default = -1).
	int minLevel;               // Minimum level a log must have to be shown (default = 3).
	bool printNewLogsOnly;      // Print the new logs only.
	int nLatest;                // Print the latest n logs only (default = -1).
	int nLatestChars;           // Print the latest n characters only (default = -1).
	bool printLogFile;          // Print the log file name for each message (useful if multiple log files are shown simultaneously).
	bool printLogNumber;        // Print the log/line numbers.
	std::string subString;      // Print the logs which contain the specified substring.
	std::string notSubString;   // Print the logs which do not contain the specified substring.
	int lessThan;               // Print the logs whose i-th token is less than the specified i_value.
	int greaterThan;            // Print the logs whose i-th token is greater than the specified i_value.
	int contextWidth;           // Number of context logs to show if the current log is above a threshold level (default = 0).
	int minLevelForContext;     // Minimum level a log must have to get a context (default = 5).
	int minContextLevel;        // Minimum level a log must have to be in the context (default = 2).
	std::string logLevels;      // Load custom log levels from file (format: tag value\n).
	bool text;                  // Parse the input file as a generic text, not as a log file.
	std::string delimiters;     // Specify custom delimiters for the messages (default = new line).
	std::string outFile;        // Redirect the output to a file (default = standard output).
	int outFileFormat;          //+ Format of the output log file: plain, console, (TODO: HTML, markdown).
	int verbose;                // Print extra information.
	int beepLevel;              // Level above which an audio signal is produced (default = -1).
	int pause;                  // Pause (in seconds) among a check of the log file and the next (default = 1.0).
	bool restore;               // Restore system in case of problems..
	int help;                   // Help.
	std::string version;        // Version and license details.

///---

	std::string   logFile;

	std::ostream  logStream;			// generic output stream for the logs
	std::filebuf  fileBuffer;
	std::string   outLogFile;			// file name for the output stream
	std::string   outLogFileFormat;		// OS shell highlighting, HTML, markdown, ...
	bool          logToFile;

	std::string   delimiters;			// delimit the end of a log (\n included by default)

	std::string   logHeader;

	int           levelColumn;			// depends on the logs format (dynamic if < 0)
	int           minLevel;				// minimum level a log must have to be shown
	int           beepLevel;			// minimum level to get an audio signal (disabled if < 0)

	bool          printLogFile;			// print the log file name for each log message
	std::string   logFileField;			// log file name to be printed for each log message

	int           logNumber;			// log/line numbers
	int           logNumberField;		// log/line numbers to be printed for each log message
	bool          printLogNumber;		// print the log/line numbers

	bool          textParsing;			// parse the input file as normal text, not as a log file
	bool          warnUnknownLogLevel;	// warning for missing level in a log

	LogLevels     logLevels;

	LogFormatter  logFormatter;

	const int     printAll = -1;
	bool          newLogsOnly;			// only print logs generated from now on
	int           nLatest;				// number of latest logs to be printed (-1 = all)
	int           nLatestChars;			// number of latest characters to be printed (-1 = all)

	std::vector<std::string> includeStrings, excludeStrings;	// sub strings to be included/excluded by the logs
	std::string   tempStr;
	bool          incStrFlag, excStrFlag;		// flags to decide whether to check for substrings

	std::vector<Compare> compare;		// set of comparisons to be done

	LogViewer::LogContext context;

	std::chrono::milliseconds pause;

	int  verbose;

	utilities::ReadKeyboard rdKb;
	int                     key;
};


} // log_viewer


#endif // LOGVIEWER_HPP
