/******************************************************************************
 * logviewer.cpp
 *
 * Utility to display log files in real time on the console.
 *
 * Copyright (C) 2012-2016 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/


/* Usage example:
 *
 *		logviewer --input example.log --minLevel 2
 *		logviewer --input example.log --minLevel 2 --levelCol 3
 *		logviewer -i example.log -m 2 -l 3
 *		logviewer --help
 */

/* TODO
	. Derive a tool to make automatic summaries from text using user specified keywords.
		. BUG fixed: LogLevels::FindLogLevelVal() - Make strings comparison case insesitive.
		--> Regression: with log files, the pre-context is made of copies of the current log.
	-- LogFormatter: HTML and markdown formatting.
	- Group code blocks in separate functions/classes.
	- Allow to pass multiple values for each command line parameter.
	-- Change pause functionality: stop loading new logs, but keep interacting.
	- Bug [Windows]: when the log grows, the new logs are not printed automatically (ENTER must be pressed).
	- Better randomize the colors in LogLevelMapping().
 */

#include "LogContext.hpp"
#include "LogFormatter.hpp"
#include "logLevels.h"
#include "progArgs.h"
#include "ReadKeyboard.h"
#include "textModeFormatting.h"
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#if defined(__unix__) || defined(__linux__) || \
	defined(BSD) || (defined (__APPLE__) && defined (__MACH__)) || defined(__bsdi__) || \
	defined(__minix) || defined(__CYGWIN__) || defined(__FreeBSD__)
#define POSIX 1
#include <sys/stat.h>
#endif

#ifdef _WIN32
#include <tchar.h>
#include <strsafe.h>
#include <windows.h>
#endif

using namespace std;
using namespace textModeFormatting;
using namespace Utilities;

#ifndef _WIN32
	static const char slash = '/';
#else
	static const char slash = '\\';
#endif

namespace LogViewer {

const int version = 4, subversion = 3, subsubversion = 0;
/* Versioning conventions:
 *	- Even subversion number: stable version.
 *	- Odd subversion number: unstable/development version.
 */

struct Compare {
	string value;
	int column;
	bool comparison;	// false = less than; true = greater than
};


int nLogsReload = 20;			// number of logs to reload when 'r' is pressed

string GetLogDate(const string &_logFile);
void   PrintHelp (const ProgArgs &_args, const char* _progName, LogLevels *_logLevels = 0);
void   PrintVersion (const char* _progName);


struct ResetDefaults
{
	ResetDefaults() {}
	~ResetDefaults() {
		// Reset console colors
#ifdef _WIN32
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#endif
	}
};

ResetDefaults rd;

} // LogViewer


int main(int argc, char* argv[])
{
	using namespace LogViewer;

#ifdef RUN_INTERNAL_TESTS
	return RunInternalTests();
#endif // RUN_INTERNAL_TESTS

	string  logFile;

	ostream  logStream(0);				// generic output stream for the logs
	filebuf  fileBuffer;
	string   outLogFile;				// file name for the output stream
	string   outLogFileFormat;			// OS shell highlighting, HTML, markdown, ...
	bool     logToFile = false;

	char delimiter = '\n';				// delimit the end of a log

	string logHeader;

	int  levelColumn = -1;				// depends on the logs format (dynamic if < 0)
	int  minLevel = 0;					// minimum level a log must have to be shown
	int  beepLevel = -1;				// minimum level to get an audio signal (disabled if < 0)
	bool printLogFile = false;			// print the log file name for each log message
	bool textParsing = false;			// parse the input file as normal text, not as a log file
	bool warnUnknownLogLevel = true;	// warning for missing level in a log

	LogLevels logLevels;
	logLevels.EnableWarnings(false);

	LogFormatter logFormatter("console");

	const int printAll = -1;
	int nLatest = printAll;				// number of latest logs to be printed (-1 = all)
	int nLatestChars = printAll;		// number of latest characters to be printed (-1 = all)

	vector<string> includeStrings, excludeStrings;		// sub strings to be included/excluded by the logs
	string tempStr;
	bool   incStrFlag = false, excStrFlag = false;		// flags to decide whether to check for substrings

	vector<Compare> compare;		// set of comparisons to be done

	LogViewer::LogContext context;

	std::chrono::milliseconds pause(1000);

	int verbose = 0;

	utilities::ReadKeyboard rdKb;
	int key = 0;

	// Reset console colors
#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#endif

	/// Command line parameters

	ProgArgs            arguments;
	ProgArgs::Argument  arg;

	// Set command line parameters:
	{
		/* int Set(std::string _tag, std::string _shortTag, std::string _desc = "",
				   bool _optional = true, bool _needed = false, std::string _default = "");
		*/
		arg.Set("--input", "-i", "Input log file", false, true);
		arguments.AddArg(arg);
		arg.Set("--levelCol", "-l", "ID of the column which contains the log level", true, true, "-1");
		arguments.AddArg(arg);
		arg.Set("--minLevel", "-m", "Minimum level a log must have to be shown", true, true, "3");
		arguments.AddArg(arg);
		arg.Set("--nLatest", "-n", "Print the latest n logs only", true, true, "-1");
		arguments.AddArg(arg);
		arg.Set("--nLatestChars", "-nc", "Print the latest n characters only", true, true, "-1");
		arguments.AddArg(arg);
		arg.Set("--printLogFile", "-f", "Print the log file name for each message (useful if multiple log files are shown simultaneously)", true, false);
		arguments.AddArg(arg);
		arg.Set("--subString", "-s", "Print the logs which contain the specified substring", true, true);
		arguments.AddArg(arg);
		arg.Set("--notSubString", "-ns", "Print the logs which do not contain the specified substring", true, true);
		arguments.AddArg(arg);
		arg.Set("--lessThan", "-lt", "Print the logs whose i-th token is less than the specified i_value", true, true);
		arguments.AddArg(arg);
		arg.Set("--greaterThan", "-gt", "Print the logs whose i-th token is greater than the specified i_value", true, true);
		arguments.AddArg(arg);
		arg.Set("--contextWidth", "-cw", "Number of context logs to show if the current log is above a threshold level", true, true, "0");
		arguments.AddArg(arg);
		arg.Set("--minLevelForContext", "-mlc", "Minimum level a log must have to get a context", true, true, "5");
		arguments.AddArg(arg);
		arg.Set("--minContextLevel", "-mcl", "Minimum level a log must have to be in the context", true, true, "2");
		arguments.AddArg(arg);
		arg.Set("--logLevels", "-ll", "Load custom log levels from file (format: tag value\\n)", true, true);
		arguments.AddArg(arg);
		arg.Set("--text", "-t", "Parse the input file as a generic text, not as a log file", true, false);
		arguments.AddArg(arg);
		arg.Set("--delimiter", "-d", "Specify a custom delimiter for the messages (default = new line)", true, true);
		arguments.AddArg(arg);
		arg.Set("--outFile", "-o", "Redirect the output to a file (default = standard output)", true, true);
		arguments.AddArg(arg);
		arg.Set("--outFileFormat", "-of", "Format of the output log file: plain, console, (TODO: HTML, markdown)", true, true);	//+TODO - Specify available formats
		arguments.AddArg(arg);
		arg.Set("--verbose", "-vb", "Print extra information");
		arguments.AddArg(arg);
		arg.Set("--beepLevel", "-bl", "Level above which an audio signal is produced", true, true, "-1");
		arguments.AddArg(arg);
		arg.Set("--pause", "-p", "Pause (in seconds) among a check of the log file and the next", true, true, "1.0");
		arguments.AddArg(arg);
		arg.Set("--help", "-h", "Help");
		arguments.AddArg(arg);
		arg.Set("--version", "-v", "Version and license details");
		arguments.AddArg(arg);
	}

	// Read command line parameters:
	{
		int nUnknown = arguments.Parse(argc, argv);
		if(nUnknown > 0) {
			cerr << "Warning: passed " << nUnknown << " unknown argument(s); they will be ignored." << endl;
		}

		arguments.GetValue("--input", logFile);

		string levelCol;
		if(arguments.GetValue("--levelCol", levelCol) >= 0)
			levelColumn = atoi(levelCol.c_str());

		string minLev;
		arguments.GetValue("--minLevel", minLev);
		minLevel = atoi(minLev.c_str());

		if(arguments.GetValue("--nLatest")) {
			string nLogs;
			arguments.GetValue("--nLatest", nLogs);
			nLatest = atoi(nLogs.c_str());
			if(nLatest < 0)
				nLatest = printAll;
		}

		if(arguments.GetValue("--nLatestChars")) {
			string nChars;
			arguments.GetValue("--nLatestChars", nChars);
			nLatestChars = atoi(nChars.c_str());
			if(nLatestChars < 0)
				nLatestChars = printAll;
		}

		if(arguments.GetValue("--printLogFile")) {
			printLogFile = true;
		}

		if(arguments.GetValue("--subString"))
		{
			int n = 0;
			while(n >= 0) {
				n = arguments.GetValue("--subString", tempStr, n);
				if(n >= 0) {
					includeStrings.push_back(tempStr);
					incStrFlag = true;
				}
			}
		}

		if(arguments.GetValue("--notSubString"))
		{
			int n = 0;
			while(n >= 0) {
				n = arguments.GetValue("--notSubString", tempStr, n);
				if(n >= 0) {
					excludeStrings.push_back(tempStr);
					excStrFlag = true;
				}
			}
		}

		if(arguments.GetValue("--lessThan"))
		{
			int n = 0;
			Compare cmp;
			while(n >= 0) {
				n = arguments.GetValue("--lessThan", tempStr, n);
				if(n >= 0) {
					if(tempStr.length() < 3) {
						cerr << "Error in the format of the --lessThan parameter." << endl;
						rdKb.~ReadKeyboard();
						exit(-1);
					}
					cmp.value = tempStr.substr(2);
					cmp.column = tempStr[0] - '0';
					cmp.comparison = false;
					compare.push_back(cmp);
				}
			}
		}

		if(arguments.GetValue("--greaterThan"))
		{
			int n = 0;
			Compare cmp;
			while(n >= 0) {
				n = arguments.GetValue("--greaterThan", tempStr, n);
				if(n >= 0) {
					if(tempStr.length() < 3) {
						cerr << "Error in the format of the --greaterThan parameter." << endl;
						rdKb.~ReadKeyboard();
						exit(-1);
					}
					cmp.value = tempStr.substr(2);
					cmp.column = tempStr[0] - '0';
					cmp.comparison = true;
					compare.push_back(cmp);
				}
			}
		}

		if(arguments.GetValue("--contextWidth"))
		{
			string contextWidth;
			arguments.GetValue("--contextWidth", contextWidth);
			context.Width(atoi(contextWidth.c_str()));
		}

		if(arguments.GetValue("--minLevelForContext"))
		{
			string minLevelForContext;
			arguments.GetValue("--minLevelForContext", minLevelForContext);
			context.MinLevelForContext(atoi(minLevelForContext.c_str()));
		}

		if(arguments.GetValue("--minContextLevel"))
		{
			string minContextLevel;
			arguments.GetValue("--minContextLevel", minContextLevel);
			context.MinContextLevel(atoi(minContextLevel.c_str()));
		}

		if(arguments.GetValue("--logLevels"))
		{
			string logFileName;
			if(arguments.GetValue("--logLevels", logFileName) < 0) {
				cerr << argv[0] << " - Error: log levels file not specified." << endl;
				rdKb.~ReadKeyboard();
				exit(LogLevels::err_fileNotFound);
			}

			cout << "Loading log levels from: " << logFileName << endl;

			if(logLevels.InitLogLevels(logFileName) == LogLevels::err_fileNotFound) {
				cerr << argv[0] << " - Error: log levels file " << logFileName << " not found." << endl;
				rdKb.~ReadKeyboard();
				exit(LogLevels::err_fileNotFound);
			}
		}

		if(arguments.GetValue("--beepLevel")) {
			string level;
			arguments.GetValue("--beepLevel", level);
			beepLevel = atoi(level.c_str());
		}

		if(arguments.GetValue("--text")) {
			textParsing = true;
			warnUnknownLogLevel = false;
			logLevels.EnableWarnings(false);
		}

		if(arguments.GetValue("--delimiter")) {
			string delim;
			arguments.GetValue("--delimiter", delim);
			delimiter = delim.front();
		}

		if(arguments.GetValue("--outFile")) {
			arguments.GetValue("--outFile", outLogFile);
			logToFile = true;
		}

		if(arguments.GetValue("--outFileFormat"))
		{
			arguments.GetValue("--outFileFormat", outLogFileFormat);

			if(logFormatter.CheckFormat(outLogFileFormat) == false)
			{
				std::cerr << "Warning: " << outLogFileFormat << " is an invalid output file format.\n"
						  << "              The default " << logFormatter.DefaultFormat() << " format will be used." << std::endl;
				outLogFileFormat = logFormatter.DefaultFormat();
			}

			logFormatter.SetFormat(outLogFileFormat);
		}

		if(arguments.GetValue("--verbose")) {
			verbose = 1;
			logLevels.EnableWarnings(true);
		}

		string sPause;
		arguments.GetValue("--pause", sPause);
		float fPause = float(atof(sPause.c_str()));
		pause = std::chrono::milliseconds(int(1000 * fPause));

		if(arguments.GetValue("--help"))
		{
			PrintHelp(arguments, argv[0], &logLevels);
			rdKb.~ReadKeyboard();
			exit(0);
		}

		if(arguments.GetValue("--version"))
		{
			PrintVersion(argv[0]);
			rdKb.~ReadKeyboard();
			exit(0);
		}
	}

	/// Logs header
	{
		string logDate = GetLogDate(logFile);		// time the log was generated

		stringstream header, tmp;
		tmp << "LogViewer " << version << "." << subversion <<  "." << subsubversion << " - "
			<< "Log file: " << logFile << " - " << logDate;

		int barLen = tmp.str().length();

		header << string(barLen, '-') << "\n" << tmp.str() << "\n" << string(barLen, '-');

		logHeader = header.str();
	}

	/// Print extra info
	if(verbose)
	{
		if(logToFile)
			cout << "Saving the logs on file: " << outLogFile << endl;
		else
			cout << "Showing the logs on the standard output." << endl;

		if(levelColumn >= 0)
			cout << "Column ID containing the log level: " << levelColumn << endl;
		else
			cout << "Column ID containing the log level: unspecified; search on the basis of prederfined tokens." << endl;

		cout << "Minimum log level for a log to be shown: " << minLevel << endl;

		cout << "Log context:  width = " << context.Width()
			 << "  minLevelForContext = " << context.MinLevelForContext()
			 << "  minContextLevel = " << context.MinContextLevel() << endl;

		if(incStrFlag) {
			cout << "Show logs which include the string(s): ";
			for(size_t i = 0; i < includeStrings.size(); ++i)
				cout << "\"" << includeStrings[i] << "\" ";
			cout << endl;
		}

		if(excStrFlag) {
			cout << "Hide logs which include the string(s): ";
			for(size_t i = 0; i < excludeStrings.size(); ++i)
				cout << "\"" << excludeStrings[i] << "\" ";
			cout << endl;
		}

		if(compare.empty() == false)
		{
			for(size_t i = 0; i < compare.size(); ++i) {
				cout << "Column " << compare[i].column << " must be ";
				if(compare[i].comparison == false)
					cout << "less";
				else
					cout << "greater";
				cout << " than " << compare[i].value << endl;
			}
		}

		cout << "Interval between checks of the log file: " << pause.count()/1000 << " seconds" << endl;

		if(nLatestChars >= 0)
			cout << "Showing the last " << nLatestChars << " characters of the existing log file." << endl;
		else if(nLatest >= 0)
			cout << "Showing the last " << nLatest << " logs of the existing log file." << endl;
		else
			cout << "Number of past logs to be shown: all" << endl;

		if(textParsing)
			cout << "Interpreting input file as plain text, not as a log file." << endl;

		cout << "Log message/text block delimiter: ";
		switch(delimiter) {
		case '\n': cout << "\\n"; break;
		case '\t': cout << "tab"; break;
		default: cout << delimiter;
		}
		cout << endl;

		cout << "Command line parameters: ";
		for(size_t i = 0; i < argc; ++i)
			cout << argv[i] << " ";
		cout << endl;


		cout << string(100, '-') << endl;
	}

	/// Open log file

	ifstream   ifs;
	string     log, token, contextLog;

	streamoff  pos = 0;					// position of the current log
	streamoff  lastPrintedLogPos = 0;	// position of the last log with level above the threshold
	streamoff  prevLogContext = 0;		// cursor exploring previous logs to provide context

	int  distNextLogContext = 0;		// distance of a past log from the current one
	int  distPrevLogContext = 0;		// distance of a future log from the current one
	bool isContextLog = false;			// the current log is part of the context

	bool warning = true;

	// Wait for the log file to be available
	while(true)
	{
		ifs.open(logFile.c_str());

		if(ifs.is_open())
			break;

		if(warning) {
			cerr << "logviewer: warning: cannot open the log file: " << logFile
				 << "\nWaiting..." << endl;
			warning = false;
		}

		this_thread::sleep_for(pause);
	}

	/// Print log file

	int  level = 0, contextLevel = 0;
	char contextSign = ' ';
	bool printLog = false;

	// Send the output either to cout or to a file
	if(outLogFile.empty()) {
		logStream.rdbuf(std::cout.rdbuf());
		logToFile = false;
	}
	else {
		fileBuffer.open(outLogFile.c_str(), std::ios_base::out | std::ofstream::app);
		logStream.rdbuf(&fileBuffer);
		logToFile = true;
	}

	logStream << logHeader << endl;

	if(nLatestChars >= 0)
	{
		// Start reading from the last "nChars" characters

		// Reposition the cursor at the end of the file, and go back n bytes
		ifs.seekg(-nLatestChars, ios::end);
		pos = ifs.tellg();
	}
	else if(nLatest >= 0)
	{
		// Start reading from the last "nLatest" logs

		// Reposition the cursor at the end of the file, and go back counting the new lines

		ifs.seekg(-1, ios::end);

		int nLogs = 0;

		while(ifs.tellg() > 0)
		{
			if(ifs.peek() == '\n')
				++nLogs;

			if(nLogs > nLatest)
				break;

			ifs.seekg(-1, ios::cur);
		}

		pos = ifs.tellg();
	}

	while(true)
	{
		if(ifs.seekg(pos))
		{
			while(getline(ifs, log, delimiter))
			{
				level = logLevels.FindLogLevel(log, levelColumn);

				printLog = false;

				if(level >= context.MinContextLevel())
				{
					// Check forward context
					{
						++distPrevLogContext;

						if(level >= context.MinLevelForContext())
							distPrevLogContext = 0;

						if(level >= minLevel)
						{
							printLog = true;
							isContextLog = false;
						}

						if(level < minLevel &&
						   level >= context.MinContextLevel() &&
						   distPrevLogContext <= context.Width())
						{
							printLog = true;
							isContextLog = true;
						}
					}

					// Log backward context

					if(printLog == false)
						// To reduce disk stress, store context logs in memory
						context.StorePastLog(log, level, minLevel);

					if(level >= context.MinLevelForContext())
					{
						while(context.NPastLogs() > 0) {
							context.ExtractPastLog(contextLog);
							contextLevel = logLevels.FindLogLevel(contextLog, levelColumn);
							logStream << logFormatter.Format(log, level, logFile, '-') << endl;
						}

						distNextLogContext = 0;
						prevLogContext = pos;
					}
				}

				if(level >= minLevel)
					printLog = true;

				if(printLog)
				{
					if(incStrFlag) {
						for(size_t s = 0; s < includeStrings.size(); ++s) {
							if(log.find(includeStrings[s]) == string::npos)
								goto nextLine;
						}
					}

					if(excStrFlag) {
						for(size_t s = 0; s < excludeStrings.size(); ++s) {
							if(log.find(excludeStrings[s]) != string::npos)
								goto nextLine;
						}
					}

					if(compare.empty() == false)
					{
						for(size_t c = 0; c < compare.size(); ++c)
						{
							stringstream str(log);
							for(int i = 0; i < compare[c].column; ++i)
								str >> token;

							if(compare[c].comparison == false) {	// check less than
								if(token >= compare[c].value)
									goto nextLine;
							}
							else {									// check greater than
								if(token <= compare[c].value)
									goto nextLine;
							}
						}
					}

					if(isContextLog)
						contextSign = '+';
					else
						contextSign = ' ';

					logStream << logFormatter.Format(log, level, logFile, contextSign) << endl;

					if(beepLevel >= 0 && level >= beepLevel)
						cout << char(7) << flush;	// beep

					lastPrintedLogPos = pos;
				}

				nextLine:
				pos = ifs.tellg();
			}
		}

		if(!ifs.eof()) {
			cerr << argv[0] << " error: could not read this log file till the end: " << logFile << endl;
			break;
		}

		ifs.clear();		// clear the eof state to keep reading the growing log file

		/// Read the keyboard for real time user interaction
		{
			key = rdKb.Get();

			// Pause logs display
			if(key == 'p' || key == 'P') {
				cout << "Paused... " << flush;
				getchar();
				cout << "Resumed" << endl;
			}

			// Change minimum log level
			if(key >= '1' && key <= '7') {
				minLevel = key - char('0');
				cout << "Minimum log level set to: " << minLevel << " - " << logLevels.GetTag(minLevel) << endl;
			}

			// Reload all logs
			if(key == 'R') {
				cout << "--- RELOAD LOG FILE ---" << endl;
				ifs.seekg(0);
				pos = ifs.tellg();
			}

			// Reload last n logs
			if(key == 'r') {
				cout << "--- RELOAD LAST " << nLogsReload << " LOGS ---" << endl;

				// Start reading from the last "nLogsReload" logs
				// Reposition the cursor at the end of the file, and go back counting the new lines
				ifs.seekg(-1, ios::end);
				int nLogs = 0;

				while(ifs.tellg() > 0)
				{
					if(ifs.peek() == '\n') ++nLogs;
					if(nLogs > nLogsReload) break;
					ifs.seekg(-1, ios::cur);
				}

				pos = ifs.tellg();
			}

			// Set the number of logs to reload
			if(key == 'n') {
				int n;
				cout << "\nCurrent number of logs to reload: " << nLogsReload << endl;
				cout <<   "New number of logs to reload:     ";
				rdKb.Blocking();
				cin >> n;
				rdKb.NonBlocking();
				if(n > 0)
					nLogsReload = n;
			}

			// Exit logviewer
			if(key == 'q' || key == 'Q') {
				cout << endl;
				rdKb.~ReadKeyboard();
				exit(0);
			}
		}

		// Take a break
		this_thread::sleep_for(pause);
	}

	rdKb.~ReadKeyboard();
	return 0;
}


namespace LogViewer {


#ifdef WIN32
BOOL GetLastWriteTime(HANDLE hFile, LPTSTR lpszString, DWORD dwSize)
{
	FILETIME ftCreate, ftAccess, ftWrite;
	SYSTEMTIME stUTC, stLocal;
	DWORD dwRet;

	// Retrieve the file times for the file.
	if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
		return FALSE;

	// Convert the last-write time to local time.
	FileTimeToSystemTime(&ftWrite, &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

	// Build a string showing the date and time.
	dwRet = StringCchPrintf(lpszString, dwSize,
		TEXT("%02d/%02d/%d  %02d:%02d"),
		stLocal.wMonth, stLocal.wDay, stLocal.wYear,
		stLocal.wHour, stLocal.wMinute);

	if (S_OK == dwRet)
		return TRUE;
	else return FALSE;
}
#endif //WIN32


string GetLogDate(const string &_logFile)
{
	// Return the time the log was generated

	string logDate;
	time_t date;

#ifdef POSIX
	struct stat st;
	if(stat(_logFile.c_str(), &st) == 0)
		date = st.st_mtime;
	else
		return "?\n";
#else // _WIN32
	//+TEST
	TCHAR szBuf[MAX_PATH];
	HANDLE hFile = CreateFile(_logFile.c_str(), GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return "?\n";
	else {
		if (GetLastWriteTime(hFile, szBuf, MAX_PATH))
			logDate = szBuf;
		else
			logDate = "?\n";
		CloseHandle(hFile);
		return logDate;
	}
#endif

	char mbstr[100];
	if (std::strftime(mbstr, sizeof(mbstr), "%FT%T", std::localtime(&date))) {
		logDate = mbstr;
	}
	else {
		logDate = "?\n";
	}

	return logDate;
}


void PrintHelp(const ProgArgs &_args, const char* _progName, LogLevels *_logLevels)
{
	string progName = _progName;

	size_t pos = progName.rfind(slash) + 1;

	cout << string(110, '-') << "\n";
	cout << "\n" << progName.substr(pos) << ": a text mode log file viewer.\n";
	cout << "\n" << "Features:\n";
	cout << "\t- Log file format agnostic.\n";
	cout << "\t- Dynamic log level threshold: in the proximity of logs with high level,\n";
	cout << "\t  automatically lower the level threshold to provide context for critical cases.\n";
	cout << "\t- Log level based highlighting. Levels can be numeric (1-7) or strings.\n";
	cout << "\t- Filtering capability.\n";
	cout << "\t- Text mode, runs everywhere after recompilation.\n";
	cout << "\t- Free software, GPL 3 license.\n";
	cout << "\nParameters:\n";
	_args.Help();
	cout << endl;

	cout << "Log levels highlighting: \n";
	for(int level = 1; level < nLevels; ++level) {
		if(_logLevels == 0)
			cout << Format(level) << level << Reset() << " ";
		else
			cout << "\t" << Format(level) << level << "\t" << _logLevels->GetTags(level) << Reset() << "\n";
	}
	cout << Reset() << endl;

	cout << "\nExamples:\n\n";
	cout << "Print the logs in the specified file, with minimum level 1, with level placed in\n"
			"the second column, which include the substrings \"abc def\" and \"123\", which do\n"
			"not include the substring \"ghi\", with timestamp between 0.123 and 0.125 seconds,\n"
			"done after 2012-10-08T14:11:09, in verbose mode:\n\n";
	cout << progName.substr(pos)
		 << " -i /path/to/test.log -m 1 -l 2 -s \"abc def\" -s \"123\" -ns \"ghi\""
		 << " -gt 1_0.123 -lt 1_0.125 -gt 3_2012-10-08T14:11:09 -vb\n\n";
	cout << "Print the logs in the specified file, with minimum level 4, with level in any column,\n"
			"and 6 context logs with level higher than 2 when the current level is higher than 4:\n\n";
	cout << progName.substr(pos)
		 << " -i /path/to/test.log -m 4 -cw 6 -mlc 4 -mcl 2" << endl;

	cout << "\nKeystroke runtime commands:\n\n";
	cout << "\t [P]       Pause/resume logs display.\n";
	cout << "\t [1]-[7]   Change minimum log level of displayed logs (no effect on their generation).\n";
	cout << "\t [R]       Reload all the logs and display them with the current criteria.\n";
	cout << "\t [r]       Reload the last " << nLogsReload << " logs and display them with the current criteria.\n";
	cout << "\t [n]       Set the number of logs to reload (default is " << nLogsReload << ").\n";
	cout << "\t [Q]       Exit logviewer.\n";

	//+TEST //+TODO: Windows version
	cout << "\n- To print multiple log files simultaneously, a script like this can be used \n"
			"   (the temporal sequence of logs from different sources is not guaranteed):\n";
	cout << "\t " << progName.substr(pos) << " -i file1.log -f &\n";
	cout << "\t " << progName.substr(pos) << " -i file2.log -f &\n";
	cout << "\t " << progName.substr(pos) << " -i file3.log -f &\n";

	cout << "\n" << string(110, '-') << "\n";
	cout << endl;
}


void PrintVersion(const char* _progName)
{
	cout << string(80, '-') << "\n";
	string progName = _progName;
	size_t p = progName.find_last_of(slash) + 1;
	cout << "\n\t" << progName.substr(p) << " version " << version << "." << subversion << "." << subsubversion << endl;
	cout << "\n\t" << "Copyright 2012-2016 Pietro Mele" << endl;
	cout << "\n\t" << "Released under a GPL 3 license." << endl;
	cout << "\n\t" << "pietrom16@gmail.com"
		 << "\n\t" << "https://sites.google.com/site/pietrom16" << endl;
	cout << string(80, '-') << "\n";
	cout << endl;
}

} // LogViewer
