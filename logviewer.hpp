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

#include <string>

namespace log_viewer {


class LogViewer
{
public:
	LogViewer();
	LogViewer(const std::string &_logFile, int _minLogLevel = 0);

	int SetDefaultValues();
	int SetLogFileName(const std::string &_logFile);
	int SetMinLogLevel(int _minLogLevel);

	int Start();
	int Pause();
	int Stop();


private:
	std::string input;          // Input log file.
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

};


} // log_viewer


#endif // LOGVIEWER_HPP
