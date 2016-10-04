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


#include "logviewer.hpp"

#include "textModeFormatting.h"

#include <cassert>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <ios>
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
#include <locale>
#include <codecvt>
BOOL GetLastWriteTime(HANDLE hFile, TCHAR *lpszString, DWORD dwSize);
#endif

using namespace textModeFormatting;
using namespace Utilities;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;

#ifndef _WIN32
	static const char slash = '/';
#else
	static const char slash = '\\';
#endif

namespace log_viewer {


/// Ctor: Read command line parameters

LogViewer::LogViewer(int argc, char *argv[])
{
	rd = new ResetDefaults;

	SetDefaultValues();
	SetCommandLineParams();
	ReadCommandLineParams(argc, argv);

	// Copy argv[]
	for(size_t i = 0; i < argc; ++i) {
		cmdLineParams += argv[i];
		cmdLineParams += " ";
	}
}

LogViewer::~LogViewer()
{
	rdKb.~ReadKeyboard();
	rd->~ResetDefaults();
	cout << "logviewer stopped.\n" << endl;
}


/// Set default values for the class member variables

int LogViewer::SetDefaultValues()
{
	logFile = "";

	logToFile = false;
	outLogFile = "";
	outLogFileFormat = "";

	externalCtrl = false;
	cmdFile = "";

	delimiters = "";

	logHeader = "";

	printLogFile = false;
	logFileField = "";

	logNumber = 0;
	logNumberField = -1;
	printLogNumber = false;

	textParsing = false;

	levelColumn = -1;
	minLevel = 0;
	beepLevel = -1;
	warnUnknownLogLevel = false;

	verbose = 0;

	logLevels.EnableWarnings(warnUnknownLogLevel);

	consoleOutput = true;
	textFileOutput = false;
	htmlOutput = false;
	markdownOutput = false;

	logFormatter.SetFormats("console");

	newLogsOnly = false;
	nLatest = printAll;
	nLatestChars = printAll;

	incStrFlag = false;
	excStrFlag = false;

	context.Erase();

	pause = std::chrono::milliseconds(1000);

	key = 0;

	nLogsReload = 20;

	return 0;
}


/// Modify input log file

int LogViewer::SetLogFileName(const std::string &_logFile)
{
	if(_logFile.size() > 0)
	{
		logFile = _logFile;

		if(inLogFs.is_open()) {
			inLogFs.close();
			inLogFs.open(logFile);
		}
	}

	return 0;
}


int LogViewer::SetMinLogLevel(int _minLogLevel)
{
	if(_minLogLevel >= 0)
	{
		minLevel = _minLogLevel;
		std::cout << "New minimum log level threshold: " << minLevel << std::endl;
	}

	return minLevel;
}


int LogViewer::SetCommandFile(const std::string &_cmdFile)
{
	cmdFile = _cmdFile;
	externalCtrl = true;

	return 0;
}


/// Main loop

int LogViewer::Run()
{
	using namespace std;

	GenerateLogHeader();

	/// Open log file

	ifstream   iCmdFs;					// file stream for the external commands
	string     log, contextLog;
	string     line, token;
	string     command;

	streamoff  pos = 0;					// position of the current log
	streamoff  lastPrintedLogPos = 0;	//+? position of the last log with level above the threshold
	streamoff  prevLogContext = 0;		//+? cursor exploring previous logs to provide context

	int  distNextLogContext = 0;		// distance of a past log from the current one
	int  distPrevLogContext = 100;		// distance of a future log from the current one
	bool isPostContextLog = false;		// the current log is part of the context

	bool warning = true;

	int  nReadLogs = 0;			// number of read logs
	int  nPrintedLogs = 0;		// number of printed logs

	// Open file for external commands
	if(externalCtrl)
	{
		iCmdFs.open(cmdFile);

		if(iCmdFs.is_open() == false) {
			externalCtrl = false;
			cerr << "logviewer: warning: cannot open the command file: " << cmdFile
				 << "\n\tExternal control will not be available." << endl;
		}
	}

	/// Print log file

	int  level = 0, contextLevel = 0;
	char contextSign = ' ';
	bool printLog = false;
	bool newLine = false;

	// Multiple output log streams for text and HTML

	if(textFileOutput)
	{
		textOutStream.open(outLogFile + ".log", ios_base::out | ios_base::app);

		if(textOutStream.is_open() == false) {
			cerr << "logviewer: warning: cannot open the output log file: " << outLogFile + ".log" << endl;
		}
	}

	if(htmlOutput)
	{
		const string outLogFilename = outLogFile + ".log.html";

		{
			ifstream check(outLogFilename);
			if(check.good() == false) {
				// The file must exist to open it in read mode
				htmlOutStream.open(outLogFilename, ios_base::out);
				htmlOutStream << endl;
				htmlOutStream.close();
			}
		}

		htmlOutStream.open(outLogFilename, ios_base::in | ios_base::out);

		if(htmlOutStream.is_open() == false) {
			cerr << "logviewer: warning: cannot open the output log file: " << outLogFilename << endl;
			htmlOutput = false;
			cerr << "logviewer: warning: HTML output disabled." << endl;
		}
	}

	if(markdownOutput) {
		//+TODO
	}

	PrintExtraInfo();

	// Wait for the log file to be available
	while(true)
	{
		inLogFs.open(logFile);

		if(inLogFs.is_open())
			break;

		if(warning) {
			cerr << "logviewer: warning: cannot open the log file: " << logFile
			     << "\nWaiting..." << endl;
			warning = false;
		}

		this_thread::sleep_for(pause);
	}

	WriteHeader();
	WriteFooter();	// add footer now, so the file is readable

	if(newLogsOnly)
	{
		// Read only the logs generated from now on; discard the past

		// Reposition the cursor at the end of the file
		inLogFs.seekg(0, ios::end);
		pos = inLogFs.tellg();
	}
	else if(nLatestChars >= 0)
	{
		// Start reading from the last "nChars" characters

		// Reposition the cursor at the end of the file, and go back n bytes
		inLogFs.seekg(-nLatestChars, ios::end);
		pos = inLogFs.tellg();
	}
	else if(nLatest >= 0)
	{
		// Start reading from the last "nLatest" logs

		// Reposition the cursor at the end of the file, and go back counting the new lines

		inLogFs.seekg(-1, ios::end);

		int nLogs = 0;

		while(inLogFs.tellg() > 0)
		{
			if(inLogFs.peek() == '\n')
				++nLogs;

			if(nLogs > nLatest)
				break;

			inLogFs.seekg(-1, ios::cur);
		}

		pos = inLogFs.tellg();
	}

	// Main loop

	while(true)
	{
		int nNewLogs = 0;

		while(!inLogFs.eof())
		{
			if(inLogFs.tellg() != streampos(-1))
			{
				MoveBackToEndLogsBlock();

				getline(inLogFs, line);

				cerr << "line: " << line << endl; //+T+OK

				if(line.empty())
					break;

				++nReadLogs;
				++nNewLogs;

				string::size_type pos_beg = 0, pos_end = 0;

				while(pos_beg != string::npos)
				{
					pos_end = line.find_first_of(delimiters, pos_beg);

					if(pos_end != string::npos)
						if(line[pos_end] == '.') {
							// Check it is not a decimal point
							if(line.size() > pos_end) {
								if(line[pos_end + 1] >= '0' && line[pos_end + 1] <= '9') {
									// Go to the next delimiter
									pos_end = line.find_first_of(delimiters, pos_end + 1);
								}
							}
						}

					if(pos_end != string::npos) {
						log = line.substr(pos_beg, pos_end - pos_beg + 1);
						pos_beg = pos_end + 1;
					}
					else {
						log = line.substr(pos_beg);
						pos_beg = string::npos;
					}

					++logNumber;

					level = logLevels.FindLogLevel(log, !textParsing, levelColumn);

					if(level < context.MinContextLevel() &&
					   level < minLevel)
						continue;

					// To reduce disk stress, store context logs in memory
					if(level >= context.MinContextLevel() &&
					   level < context.MinLevelForContext() &&
					   level < minLevel &&
					   distPrevLogContext > context.Width())
					{
						context.StorePastLog(log, level, minLevel, logNumber);
						pos = inLogFs.tellg();
						continue;
					}

					// Check if this log's level is high enough to log the pre-context
					if(level >= context.MinLevelForContext())
					{
						// Log pre-context

						while(context.NPastLogs() > 0)
						{
							int logNumberPre = context.ExtractPastLog(contextLog);

							if(printLogNumber)
								logNumberField = logNumberPre;
							else
								logNumberField = -1;

							contextLevel = logLevels.FindLogLevel(contextLog, !textParsing, levelColumn);

							if(newLine) {
								cout << endl;
								newLine = false;
							}

							WriteLog(contextLog, contextLevel, logFileField, '-', logNumberField);

							++nPrintedLogs;
						}

						distNextLogContext = 0;		//+?
						prevLogContext = pos;
					}

					// Check if this log's level is high enough to log the post-context

					isPostContextLog = false;
					printLog = false;

					if(level >= context.MinLevelForContext())
						distPrevLogContext = 0;

					if(level >= minLevel)
					{
						// Normal log

						isPostContextLog = false;
						printLog = true;
						contextSign = ' ';
					}
					else if(level >= context.MinContextLevel())
					{
						// Post-context log

						++distPrevLogContext;

						if(distPrevLogContext <= context.Width())
						{
							isPostContextLog = true;
							printLog = true;
							contextSign = '+';
						}
					}

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

						if(printLogNumber)
							logNumberField = logNumber;
						else
							logNumberField = -1;

						if(log.size() > 0)
						{
							if(newLine) {
								cout << endl;
								newLine = false;
							}

							WriteLog(log, level, logFileField, contextSign, logNumberField);

							++nPrintedLogs;

							if(beepLevel >= 0 && level >= beepLevel)
								cout << char(7) << flush;	// beep
						}

						lastPrintedLogPos = pos;
					}

				}

				nextLine:
				pos = inLogFs.tellg();
			}
		}

		if(nNewLogs > 0) {
			WriteFooter();
		}

		inLogFs.clear();		// clear the eof state to keep reading the growing log file

		// Get user commands
		ReadKeyboard(inLogFs, pos);

		// Get external commands
		ReadExternalCommands(inLogFs, pos);

		// Take a break
		if(textParsing == false)
			this_thread::sleep_for(pause);

		if(verbose) {
			cout << "." << flush;
			newLine = true;
		}
	}

	rdKb.~ReadKeyboard();

	stringstream report("\nTotal number of logs so far: ");
	report << logNumber;
	WriteLog(report.str(), 1, logFileField);
	WriteFooter();

	return 0;
}



std::string LogViewer::GetLogDate(const std::string &_logFile)
{
	// Return the time the log was generated

	std::string logDate;
	time_t date;

#ifdef POSIX
	struct stat st;
	if(stat(_logFile.c_str(), &st) == 0)
	{
		date = st.st_mtime;

		char mbstr[100];
		if (std::strftime(mbstr, sizeof(mbstr), "%FT%T", std::localtime(&date))) {
			logDate = mbstr;
		}
		else {
			logDate = "?\n";
		}

		return logDate;
	}
	else
		return "?\n";

#else // _WIN32
	TCHAR szBuf[MAX_PATH];
	HANDLE hFile = CreateFile(_logFile.c_str(), GENERIC_READ, FILE_SHARE_READ,
							  NULL, OPEN_EXISTING, 0, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return "?\n";
	else {
		//+TODO
/*		if(GetLastWriteTime(hFile, szBuf, MAX_PATH)) {
			std::wstring wlogDate = std::wstring(szBuf);
			typedef std::codecvt_utf8<wchar_t> convert_typeX;
			std::wstring_convert<convert_typeX, wchar_t> converterX;
			logDate = converterX.to_bytes(wlogDate);
		}
		else
*/			logDate = "?\n";
		CloseHandle(hFile);
		return logDate;
	}
#endif
}


int LogViewer::CheckLogFilesDiagnostic() const
{
	int status = 0;

	if(htmlOutStream.good() == true)       status = 0;
	if(htmlOutStream.is_open() == false) --status;
	if(htmlOutStream.fail() == true)     --status;
	if(htmlOutStream.bad()  == true)     --status;
	if(htmlOutStream.eof()  == true)     --status;

	return status;
}


void LogViewer::PrintHelp(const ProgArgs &_args, const char* _progName, LogLevels *_logLevels)
{
	using std::cout;
	using std::endl;
	using std::string;

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
	cout << "\t- Controllable by a human and through a command file.\n";
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

	cout << "\nSyntax of the command file:\n";
	cout << "\n   min_level L    Change minimum log level of displayed logs to L in [1, 7] (no effect on their generation).";
	cout << "\n   reload_all     Move to the beginning of the log file, and reprint all the logs.";
	cout << "\n   reload         Move N logs backwards and reprint them from there.";
	cout << "\n   reload_n N     Set the number N of logs to reload.";
	cout << "\n   quit           Exit the program.\n";
	cout << "\n   Note: only one command per line.\n";

	//+TEST //+TODO: Windows version
	cout << "\n- To print multiple log files simultaneously, a script like this can be used \n"
			"   (the temporal sequence of logs from different sources is not guaranteed):\n";
	cout << "\t " << progName.substr(pos) << " -i file1.log -f &\n";
	cout << "\t " << progName.substr(pos) << " -i file2.log -f &\n";
	cout << "\t " << progName.substr(pos) << " -i file3.log -f &\n";

	cout << "\n" << string(110, '-') << "\n";
	cout << endl;
}


void LogViewer::PrintVersion(const char* _progName)
{
	using std::cout;
	using std::endl;
	using std::string;

	cout << string(80, '-') << "\n";
	string progName = _progName;
	size_t p = progName.find_last_of(slash);

	if(p == string::npos)
		p = 0;
	else {
		++p;
		progName = progName.substr(p);
	}

	// Check whether this executable name has been modified
	string aka;
	{
		const string realProgName = "logviewer";
		if(progName != realProgName) {
			aka = " (A.K.A. ";
			aka.append(realProgName);
			aka.append(") ");
		}
	}

	cout << "\n\t" << progName << aka << " version " << version << "." << subversion << "." << subsubversion << "\n";
	cout << "\n\t" << "Copyright 2012-2016 Pietro Mele" << "\n";
	cout << "\n\t" << "Released under a GPL 3 license." << "\n";
	cout << "\n\t" << "pietrom16@gmail.com"
		 << "\n\t" << "https://sites.google.com/site/pietrom16" << "\n";
	cout << "\n\t" << "Build date: " << __DATE__ << " " << __TIME__ << "\n";
	cout << string(80, '-') << "\n";
	cout << endl;
}


void LogViewer::PrintLogFilesDiagnostic(const std::string &_msg) const
{
	cerr << "Log files diagnostic: " << _msg << " = " << CheckLogFilesDiagnostic() << endl;
}


/// Set available command line parameters

int LogViewer::SetCommandLineParams()
{
	ProgArgs::Argument  arg;

	/* int Set(std::string _tag, std::string _shortTag, std::string _desc = "",
			   bool _optional = true, bool _needed = false, std::string _default = "");
	*/
	arg.Set("--input", "-i", "Input log file name", false, true);
	progArgs.AddArg(arg);
	arg.Set("--levelCol", "-l", "ID of the column which contains the log level", true, true, "-1");
	progArgs.AddArg(arg);
	arg.Set("--minLevel", "-m", "Minimum level a log must have to be shown", true, true, "3");
	progArgs.AddArg(arg);
	arg.Set("--printNewLogsOnly", "-nl", "Print the new logs only", true, false);
	progArgs.AddArg(arg);
	arg.Set("--nLatest", "-n", "Print the latest n logs only", true, true, "-1");
	progArgs.AddArg(arg);
	arg.Set("--nLatestChars", "-nc", "Print the latest n characters only", true, true, "-1");
	progArgs.AddArg(arg);
	arg.Set("--printLogFile", "-f", "Print the log file name for each message (useful if multiple log files are shown simultaneously)", true, false);
	progArgs.AddArg(arg);
	arg.Set("--printLogNumber", "-ln", "Print the log/line numbers", true, false);
	progArgs.AddArg(arg);
	arg.Set("--subString", "-s", "Print the logs which contain the specified substring", true, true);
	progArgs.AddArg(arg);
	arg.Set("--notSubString", "-ns", "Print the logs which do not contain the specified substring", true, true);
	progArgs.AddArg(arg);
	arg.Set("--lessThan", "-lt", "Print the logs whose i-th token is less than the specified i_value", true, true);
	progArgs.AddArg(arg);
	arg.Set("--greaterThan", "-gt", "Print the logs whose i-th token is greater than the specified i_value", true, true);
	progArgs.AddArg(arg);
	arg.Set("--contextWidth", "-cw", "Number of context logs to show if the current log is above a threshold level", true, true, "0");
	progArgs.AddArg(arg);
	arg.Set("--minLevelForContext", "-mlc", "Minimum level a log must have to get a context", true, true, "5");
	progArgs.AddArg(arg);
	arg.Set("--minContextLevel", "-mcl", "Minimum level a log must have to be in the context", true, true, "2");
	progArgs.AddArg(arg);
	arg.Set("--logLevels", "-ll", "Load custom log levels from file (format: tag value\\n)", true, true);
	progArgs.AddArg(arg);
	arg.Set("--text", "-t", "Parse the input file as a generic text, not as a log file", true, false);
	progArgs.AddArg(arg);
	arg.Set("--delimiters", "-d", "Specify custom delimiters for the messages (default = new line; in case a \';\' is needed, double quote it)", true, true);
	progArgs.AddArg(arg);
	arg.Set("--outFile", "-o", "Redirect the output to a file (default = standard output)", true, true);
	progArgs.AddArg(arg);
	arg.Set("--outFileFormat", "-of", "Format of the output log file: console, plain, HTML (TODO: markdown)", true, true);
	progArgs.AddArg(arg);
	arg.Set("--cmdFile", "-cf", "Command file name, to control the program from outside", true, true);
	progArgs.AddArg(arg);
	arg.Set("--verbose", "-vb", "Print extra information");
	progArgs.AddArg(arg);
	arg.Set("--beepLevel", "-bl", "Level above which an audio signal is produced", true, true, "-1");
	progArgs.AddArg(arg);
	arg.Set("--pause", "-p", "Pause (in seconds) among a check of the log file and the next", true, true, "1.0");
	progArgs.AddArg(arg);
	arg.Set("--restore", "-res", "Restore system in case of problems");
	progArgs.AddArg(arg);
	arg.Set("--help", "-h", "Help");
	progArgs.AddArg(arg);
	arg.Set("--version", "-v", "Version and license details");
	progArgs.AddArg(arg);

	return 0;
}


/// Read command line parameters, set corresponding members

int LogViewer::ReadCommandLineParams(int argc, char *argv[])
{
	using std::cerr;
	using std::endl;
	using std::string;

	int nUnknown = progArgs.Parse(argc, argv);

	if(nUnknown > 0) {
		cerr << "Warning: passed " << nUnknown << " unknown argument(s); they will be ignored." << endl;
	}

	progArgs.GetValue("--input", logFile);

	string levelCol;
	if(progArgs.GetValue("--levelCol", levelCol) >= 0)
		levelColumn = atoi(levelCol.c_str());

	if(progArgs.GetValue("--text")) {
		textParsing = true;
		warnUnknownLogLevel = false;
		logLevels.EnableWarnings(warnUnknownLogLevel);
	}

	if(progArgs.GetValue("--minLevel")) {
		string minLev;
		progArgs.GetValue("--minLevel", minLev);
		minLevel = logLevels.GetVal(minLev);
	}
	else if(textParsing)
		minLevel = 0;

	if(progArgs.GetValue("--printNewLogsOnly")) {
		newLogsOnly = true;
	}

	if(progArgs.GetValue("--nLatest")) {
		string nLogs;
		progArgs.GetValue("--nLatest", nLogs);
		nLatest = atoi(nLogs.c_str());
		if(nLatest < 0)
			nLatest = printAll;
	}

	if(progArgs.GetValue("--nLatestChars")) {
		string nChars;
		progArgs.GetValue("--nLatestChars", nChars);
		nLatestChars = atoi(nChars.c_str());
		if(nLatestChars < 0)
			nLatestChars = printAll;
	}

	if(progArgs.GetValue("--printLogFile")) {
		printLogFile = true;
		logFileField = logFile;
	}
	else {
		printLogFile = false;
		logFileField.clear();
	}

	if(progArgs.GetValue("--printLogNumber")) {
		printLogNumber = true;
	}

	if(progArgs.GetValue("--subString"))
	{
		int n = 0;
		while(n >= 0) {
			n = progArgs.GetValue("--subString", tempStr, n);
			if(n >= 0) {
				includeStrings.push_back(tempStr);
				incStrFlag = true;
			}
		}
	}

	if(progArgs.GetValue("--notSubString"))
	{
		int n = 0;
		while(n >= 0) {
			n = progArgs.GetValue("--notSubString", tempStr, n);
			if(n >= 0) {
				excludeStrings.push_back(tempStr);
				excStrFlag = true;
			}
		}
	}

	if(progArgs.GetValue("--lessThan"))
	{
		int n = 0;
		Compare cmp;
		while(n >= 0) {
			n = progArgs.GetValue("--lessThan", tempStr, n);
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

	if(progArgs.GetValue("--greaterThan"))
	{
		int n = 0;
		Compare cmp;
		while(n >= 0) {
			n = progArgs.GetValue("--greaterThan", tempStr, n);
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

	if(progArgs.GetValue("--contextWidth"))
	{
		string contextWidth;
		progArgs.GetValue("--contextWidth", contextWidth);
		context.Width(atoi(contextWidth.c_str()));
	}

	if(progArgs.GetValue("--minLevelForContext"))
	{
		string minLevelForContext;
		progArgs.GetValue("--minLevelForContext", minLevelForContext);
		context.MinLevelForContext(logLevels.GetVal(minLevelForContext));
	}

	if(progArgs.GetValue("--minContextLevel"))
	{
		string minContextLevel;
		progArgs.GetValue("--minContextLevel", minContextLevel);
		context.MinContextLevel(logLevels.GetVal(minContextLevel));
	}

	if(progArgs.GetValue("--logLevels"))
	{
		string logLevelsFileName;
		if(progArgs.GetValue("--logLevels", logLevelsFileName) < 0) {
			cerr << argv[0] << " - Error: log levels file not specified." << endl;
			rdKb.~ReadKeyboard();
			exit(LogLevels::err_fileNotFound);
		}

		cout << "Loading log levels from: " << logLevelsFileName << endl;

		if(logLevels.InitLogLevels(logLevelsFileName) == LogLevels::err_fileNotFound) {
			cerr << argv[0] << " - Error: log levels file " << logLevelsFileName << " not found." << endl;
			rdKb.~ReadKeyboard();
			exit(LogLevels::err_fileNotFound);
		}
	}

	if(progArgs.GetValue("--beepLevel")) {
		string level;
		progArgs.GetValue("--beepLevel", level);
		beepLevel = logLevels.GetVal(level);
	}

	if(progArgs.GetValue("--delimiters")) {
		progArgs.GetValue("--delimiters", delimiters);
	}

	if(progArgs.GetValue("--outFile")) {
		progArgs.GetValue("--outFile", outLogFile);
		logToFile = true;
		size_t extPos = outLogFile.find(".log");
		if(extPos != std::string::npos) {
			outLogFile.erase(extPos);
		}
	}

	if(progArgs.GetValue("--outFileFormat"))
	{
		progArgs.GetValue("--outFileFormat", outLogFileFormat);

		if(logFormatter.CheckFormats(outLogFileFormat) == 0)
		{
			std::cerr << "Warning: " << outLogFileFormat << " is an invalid output file format.\n"
			          << "              The default " << logFormatter.DefaultFormats() << " format will be used." << std::endl;
			outLogFileFormat = logFormatter.DefaultFormats();
		}

		logFormatter.SetFormats(outLogFileFormat, consoleOutput, textFileOutput, htmlOutput, markdownOutput);
	}

	if(progArgs.GetValue("--cmdFile")) {
		progArgs.GetValue("--cmdFile", cmdFile);
		externalCtrl = true;
	}

	if(progArgs.GetValue("--verbose")) {
		verbose = 1;
		if(textParsing == false)
			logLevels.EnableWarnings(true);
	}

	string sPause;
	progArgs.GetValue("--pause", sPause);
	float fPause = float(atof(sPause.c_str()));
	pause = std::chrono::milliseconds(int(1000 * fPause));

	if(progArgs.GetValue("--restore"))
	{
		std::cout << "Restoring the system..." << std::endl;
		rdKb.~ReadKeyboard();
		std::cout << "...done." << std::endl;
		exit(0);
	}

	if(progArgs.GetValue("--help"))
	{
		PrintHelp(progArgs, argv[0], &logLevels);
		rdKb.~ReadKeyboard();
		exit(0);
	}

	if(progArgs.GetValue("--version"))
	{
		PrintVersion(argv[0]);
		rdKb.~ReadKeyboard();
		exit(0);
	}

	if(textParsing == false)
		delimiters.append("\n");	// new line as default delimiter for logs

	return 0;
}


int LogViewer::WriteHeader()
{
	int n = 0;

	if(consoleOutput) {
		cout << logFormatter.HeaderConsole() << endl;
		++n;
	}

	if(textFileOutput) {
		textOutStream << logFormatter.HeaderPlain() << endl;
		++n;
	}

	if(htmlOutput)
	{
		/* //+B+
		// Write the header only if the HTML file does not start with "<!DOCTYPE"

		htmlOutStream.seekg(0, std::ios_base::beg);
		std::string token;
		htmlOutStream >> token;

		cerr << "Token = " << token << endl; //+T+ok

		if(token != "<!DOCTYPE") {
			cerr << logFormatter.HeaderHTML() << endl; //+T+ok
			htmlOutStream.seekp(0, std::ios_base::beg);
			htmlOutStream << logFormatter.HeaderHTML() << endl;
			AddHtmlControls();
			++n;
		}
		*/

		// Write the header only if the HTML file is shorter than a fixed size
		htmlOutStream.seekg(0, std::ios_base::end);
		const std::streamoff size = htmlOutStream.tellg();

		if(size < 10) {
			htmlOutStream.seekp(0, std::ios_base::beg);
			htmlOutStream << logFormatter.HeaderHTML() << endl;
			AddHtmlControls();
			++n;
		}
	}

	if(markdownOutput) {
		//+TODO
	}

	return n;
}


/// Write the log to a set of destinations (console, HTML file, ...)

int LogViewer::WriteLog(const std::string &_log, int _level, const std::string &_file, char _tag, int _logNumber)
{
	int n = 0;

	if(consoleOutput) {
		cout << logFormatter.FormatConsole(_log, _level, _file, _tag, _logNumber) << endl;
		++n;
	}

	if(textFileOutput) {
		textOutStream << logFormatter.FormatPlain(_log, _level, _file, _tag, _logNumber) << endl;
		++n;
	}

	if(htmlOutput) {
		htmlOutStream << logFormatter.FormatHTML(_log, _level, _file, _tag, _logNumber) << endl;
		++n;
	}

	if(markdownOutput) {
		//+TODO
	}

	WriteFooter();

	return n;
}


int LogViewer::WriteFooter()
{
	int n = 0;

	if(consoleOutput) {
		cout << logFormatter.FooterConsole();
		++n;
	}

	if(textFileOutput) {
		textOutStream << logFormatter.FooterPlain();
		++n;
	}

	if(htmlOutput) {
		MoveBackToEndLogsBlock();
		AddHtmlControls();
		htmlOutStream << logFormatter.FooterHTML() << endl;
		++n;
	}

	if(markdownOutput) {
		//+TODO
	}

	return n;
}



/// Logs header

int LogViewer::GenerateLogHeader()
{
	const std::string logDate = GetLogDate(logFile);		// time the log was generated

	std::stringstream header, tmp;
	tmp << "Log file: " << logFile << " - " << logDate << " - "
	    << "LogViewer " << version << "." << subversion <<  "." << subsubversion;

	logFormatter.SetTitle(tmp.str());

	const size_t barLen = tmp.str().length();

	header << std::string(barLen, '-') << "\n" << tmp.str() << "\n" << std::string(barLen, '-');

	logHeader = header.str();

	return 0;
}


/// Log footer: move before output file's log footer

int LogViewer::MoveBackToEndLogsBlock()
{
	using namespace std;

	if(htmlOutput)
	{
		// Search backwards for the end of the logs block
		// Look for the last log message

		/** //+TODO
		 *	- Look for last </span>, too.
		*/

		string        token, line;
		const string  logsEndToken_table("<table>");
		const string  logsEndToken_body("</body>");
		const string  logsEndToken_span("</span>");

		/** Assumed end of HTML file structure:
					...logs...
					<br> <span style="color:red;">Log message</span>
					<table> ... </table>
				</body>
			</html>
		*/

		long assumedFooterLength = 400;		// approximation in excess

		// Start from the end of the log file
		htmlOutStream.seekg(0, ios_base::end);
		htmlOutStream.seekp(0, ios_base::end);

		// Length of the current log file
		const streamsize size = htmlOutStream.tellg();

		if(size < assumedFooterLength)
			assumedFooterLength = size;

		streamoff pos = 0,
		          pos_end_body = 0, pos_beg_table = 0, pos_last_span = 0,
		          pos_new_logs = 0;

		// Go back a fixed number of characters
		htmlOutStream.seekg(-assumedFooterLength, ios_base::end);

		// Find the point where to start adding new logs
		while(!htmlOutStream.eof())
		{
			pos = htmlOutStream.tellg();

			getline(htmlOutStream, line);

			cerr << "pos: " << pos << " log: " << line << endl; //+T+

			if(line.find(logsEndToken_body) != string::npos)
				pos_end_body = pos;

			if(line.find(logsEndToken_table) != string::npos) {
				pos_beg_table = pos;
				break;
			}

			if(line.find(logsEndToken_span) != string::npos) {
				pos_last_span = htmlOutStream.tellg();
				break;
			}
		}

		if(pos_last_span)
			pos_new_logs = pos_last_span;
		else if(pos_beg_table)
			pos_new_logs = pos_beg_table;
		else
			pos_new_logs = pos_end_body;

		cerr << "pos_end_body = " << pos_end_body << ";  pos_beg_table = " << pos_beg_table << endl; //+T+
		cerr << "pos_new_logs = " << pos_new_logs << endl; //+T+

		if(pos_new_logs != 0) {
			htmlOutStream.seekg(pos_new_logs, ios_base::beg);
			htmlOutStream.seekp(pos_new_logs, ios_base::beg);
		}
		else {
			// End of the logs block not found
			htmlOutStream.seekg(-1, ios_base::end);
			htmlOutStream.seekp(-1, ios_base::end);
		}

		//htmlOutStream << "xxx" << flush; //+T+ OK

		// Reset htmlOutStream error state flags
		htmlOutStream.clear();
	}

	return 0;
}


/// Print extra info

int LogViewer::PrintExtraInfo()
{
	if(htmlOutput)
		cout << "With a browser, open: " << outLogFile << ".log.html" << endl;

	if(verbose == 0)
		return 0;

	if(consoleOutput)
		cout << "Showing the logs on the console." << endl;

	if(textFileOutput)
		cout << "Copying the text logs in: " << outLogFile << ".log" << endl;

	if(htmlOutput)
		cout << "Copying the HTML logs in: " << outLogFile << ".log.html" << endl;

	if(markdownOutput)
		; //+TODO

	if(newLogsOnly)
		cout << "Not showing past logs." << endl;
	else
		cout << "Showing past logs." << endl;

	cout << "\nLog levels highlighting:\n";
	for(int level = 1; level <= int(logLevels.NLevels()); ++level) {
		cout << "\t" << Format(level) << level << "\t" << logLevels.GetTags(level) << Reset() << "\n";
	}
	cout << Reset() << endl;

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

	cout << "Log message/text block delimiters: ";
	cout << "\\n (new line) ";
	for(size_t i = 0; i < delimiters.size(); ++i)
	{
		switch(delimiters[i]) {
		case '\t': cout << "\\t (tab) "; break;
		case '\n': cout << "\\n "; break;
		default: cout << delimiters[i] << " ";
		}
	}
	cout << endl;

	cout << "Command line parameters: " << cmdLineParams << endl;

	cout << std::string(100, '-') << endl;

	return 0;
}


/// Read the keyboard for real time user interaction

int LogViewer::ReadKeyboard(std::ifstream &ifs, std::streamoff &pos)
{
	key = rdKb.Get();

	// Pause logs display
	if(key == 'p' || key == 'P') {
		cout << "Paused... " << std::flush;
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
		ifs.seekg(-1, std::ios::end);
		int nLogs = 0;

		while(ifs.tellg() > 0)
		{
			if(ifs.peek() == '\n') ++nLogs;
			if(nLogs > nLogsReload) break;
			ifs.seekg(-1, std::ios::cur);
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
		rd->~ResetDefaults();
		cout << "logviewer stopped.\n" << endl;
		exit(0);
	}

	return 0;
}


/// Read commands for real time external control

int LogViewer::ReadExternalCommands(std::ifstream &ifs, std::streamoff &pos)
{
	using namespace std;

	string cmd, cmd_token, arg_token;
	queue<string> cmdQueue;

	// Read all commands from command file

	{
		ifstream cmdStr;
		cmdStr.open(cmdFile);

		if(cmdStr.is_open() == false) {
			//cerr << "No external commands to execute." << endl;
			return MSG_MISSING_COMMAND_FILE;
		}

		while(cmdStr.eof() == false) {
			getline(cmdStr, cmd);
			cmdQueue.push(cmd);
		}

		// cmdStr closed by destructor
	}

	// Erase the file ASAP
	remove(cmdFile.c_str());

	/// Execute commands
	while(cmdQueue.empty() == false)
	{
		cmd = cmdQueue.front();
		cmdQueue.pop();

		stringstream ss(cmd);

		ss >> cmd_token;

		if(cmd_token == "quit") {
			rdKb.~ReadKeyboard();
			exit(0);
		}
		else if(cmd_token == "min_level") {
			ss >> arg_token;
			int mLev = arg_token[0] - char('0');
			if(mLev >= 0 && mLev < 10) {
				minLevel = mLev;
				cout << "Info: minimum log level changed to: " << minLevel << endl;
			}
			else {
				cerr << "Error: invalid minimum log level required: " << mLev << endl;
			}
		}
		else if(cmd_token == "reload_all") {
			// Reload all logs
			ifs.seekg(0);
			pos = ifs.tellg();
			cout << "Info: log file reloaded." << endl;
		}
		else if(cmd_token == "reload") {
			// Reload last n logs
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
		else if(cmd_token == "reload_n") {
			// Set the number of logs to reload
			int n;
			ss >> n;
			if(n > 0) {
				nLogsReload = n;
				cout << "Info: number of logs to reload = " << nLogsReload << endl;
			}
		}
		else if(cmd_token == "switch_log") {
			ss >> arg_token;
			if(arg_token.size() > 0) {
				logFile = arg_token;
				inLogFs.close();
				inLogFs.open(logFile);
			}
		}
		//+TODO - Add commands here
		else {
			cerr << "Warning: command token not recognised: " << cmd_token << endl;
			cerr << "Warning: command not executed: " << cmd << endl;
		}

	}

	return 0;
}


/// Add widgets to control/filter the HTML output

int LogViewer::AddHtmlControls()
{
	//+TODO - CSS settings

	htmlOutStream
	   << "<table> style=\"width:100%\""
	   << "	<tr>"
	   << "		<th>Reload</th>"
	   << "		<th>Min log level = </th>"
	   << "		<th>Number</th>"
	   << "		<th>Verbose</th>"
	   << "		<th>Show file name</th>"
	   << "		<th>Include</th>"
	   << "		<th>Exclude</th>"
	   << "		<th>Context</th>"
	   << "		<th>Width</th>"
	   << "		<th>Min level for context</th>"
	   << "		<th>Min context level</th>"
	   << "		<th></th>"
	   << "	</tr>"
	   << "</table>"
	   << endl;

	return -1;
}


/// Platform specific code

#ifdef _WIN32
BOOL GetLastWriteTime(HANDLE hFile, TCHAR *lpszString, DWORD dwSize)
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
	
	return FALSE;
}
#endif //WIN32


} // log_viewer
