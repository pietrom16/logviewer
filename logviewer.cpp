/******************************************************************************
 * logviewer.cpp
 *
 * Utility to display log files in real time on the console.
 *
 * Copyright (C) 2012-2015 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/


/* Usage example:
 *
 *		logviewer --input example.log --minLevel 2 --levelCol 3
 *		logviewer -i example.log -m 2 -l 3
 *		logviewer --help
 */

/* TODO
	-- Automatic detection of the log level tag, without specifing its position.
	-- Change pause functionality: stop loading new logs, but keep interacting.
	-- Multiple input log files.
	-- Bug [Windows]: when the log grows, the new logs are not printed automatically.
	- Better randomize the colors in LogLevelMapping().
 */

#include "progArgs.h"
#include "ReadKeyboard.h"
#include "textModeFormatting.h"
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
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


const int version = 3, subversion = 2, subsubversion = 0;

struct Compare {
	string value;
	int column;
	bool comparison;	// false = less than; true = greater than
};

int nLogsReload = 20;			// number of logs to reload when 'r' is pressed

int    GetLevel (const string &_level);
string GetLevel (int _level);
int    LogLevelMapping (const string &_level);
string GetLogDate(const string &_logFile);
void   PrintHelp (const ProgArgs &_args, const char* _progName);
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


int main(int argc, char* argv[])
{
	int levelColumn = -1;				// depends on the logs format (dynamic if < 0)
	int minLevel = 0;					// minimum level a log must have to be shown
	unsigned int beepLevel = -1;		// minimum level to get an audio signal

	const int printAll = -1;
	int nLatest = printAll;				// number of latest logs to be printed (-1 = all)
	int nLatestChars = printAll;		// number of latest characters to be printed (-1 = all)

	vector<string> includeStrings, excludeStrings;		// sub strings to be included/excluded by the logs
	string tempStr;
	bool   incStrFlag = false, excStrFlag = false;		// flags to decide whether to check for substrings

	vector<Compare> compare;		// set of comparisons to be done

	std::chrono::milliseconds pause(1000);

	int verbose = 0;

	utilities::ReadKeyboard rdKb;
	int key = 0;

	// Reset console colors
#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#endif

	/// Read parameters

	ProgArgs            arguments;
	ProgArgs::Argument  arg;

	// Set main() parameters:
	arg.Set("--input", "-i", "Input log file", false, true);
	arguments.AddArg(arg);
	arg.Set("--levelCol", "-l", "ID of the column which contains the log level", true, true, "1");
	arguments.AddArg(arg);
	arg.Set("--minLevel", "-m", "Minimum level a log must have to be shown", true, true, "3");
	arguments.AddArg(arg);
	arg.Set("--nLatest", "-n", "Print the latest n logs only", true, true, "-1");
	arguments.AddArg(arg);
	arg.Set("--nLatestChars", "-nc", "Print the latest n characters only", true, true, "-1");
	arguments.AddArg(arg);
	arg.Set("--subString", "-s", "Print the logs which contain the specified substring", true, true);
	arguments.AddArg(arg);
	arg.Set("--notSubString", "-ns", "Print the logs which do not contain the specified substring", true, true);
	arguments.AddArg(arg);
	arg.Set("--lessThan", "-lt", "Print the logs whose i-th token is less than the specified i_value", true, true);
	arguments.AddArg(arg);
	arg.Set("--greaterThan", "-gt", "Print the logs whose i-th token is greater than the specified i_value", true, true);
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

	arguments.Parse(argc, argv);

	if(arguments.GetValue("--help"))
	{
		PrintHelp(arguments, argv[0]);
		rdKb.~ReadKeyboard();
		exit(0);
	}

	if(arguments.GetValue("--version"))
	{
		PrintVersion(argv[0]);
		rdKb.~ReadKeyboard();
		exit(0);
	}

	string logFile;
	arguments.GetValue("--input", logFile);

	string levelCol;
	arguments.GetValue("--levelCol", levelCol);
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

	if(arguments.GetValue("--beepLevel")) {
		string level;
		arguments.GetValue("--beepLevel", level);
		beepLevel = atoi(level.c_str());
	}

	if(arguments.GetValue("--verbose"))
		verbose = 1;

	string sPause;
	arguments.GetValue("--pause", sPause);
	float fPause = atof(sPause.c_str());
	pause = std::chrono::milliseconds(int(1000 * fPause));


	/// Print header

	string logDate = GetLogDate(logFile);		// time the log was generated

	stringstream header;
	header << "LogViewer " << version << "." << subversion <<  "." << subsubversion << " - "
		   << "Log file: " << logFile << " - " << logDate;

	int barLen = header.str().length();

	cout << string(barLen, '-') << "\n";
	cout << header.str() << "\n";
	cout << string(barLen, '-') << endl;

	/// Print extra info
	if(verbose)
	{
		if(levelColumn >= 0)
			cout << "Column ID containing the log level: " << levelColumn << endl;
		else
			cout << "Column ID containing the log level: unspecified; search on the basis of prederfined tokens." << endl;

		cout << "Minimum log level for a log to be shown: " << minLevel << endl;

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

		cout << string(80, '-') << endl;
	}

	/// Open log file

	ifstream   ifs;
	string     log, token;
	streamoff  pos = 0;

	bool warning = true;

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

	int level = 0;

	if(nLatestChars >= 0)
	{
		// Start reading from the last "nChars" characters

		// Reposition the cursor at the end of the file, and go back n bytes
		ifs.seekg(-nLatestChars, ios::end);
	}
	else if(nLatest >= 0)
	{
		// Start reading from the last "nLatest" logs

		// Reposition the cursor at the end of the file, and go back counting the new lines

		ifs.seekg(-1, ios::end);

		int nLogs = 0;

		while(ifs.tellg() > 0)
		{
			if(ifs.peek() == '\n') ++nLogs;

			if(nLogs > nLatest) break;

			ifs.seekg(-1, ios::cur);
		}
	}

	while(true)
	{
		if(ifs.seekg(pos))
		{
			while(getline(ifs, log))
			{
				stringstream str(log);
				for(int i = 0; i < levelColumn; ++i)
					str >> token;

				level = GetLevel(token);

				if(level >= minLevel)
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

					cout << Format(level) << log << Reset() << endl;

					if(level >= beepLevel)
						cout << char(7) << flush;	// beep
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
			cout << "Minimum log level set to: " << minLevel << " - " << GetLevel(minLevel) << endl;
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

		// Take a break
		this_thread::sleep_for(pause);
	}

	rdKb.~ReadKeyboard();
	return 0;
}


int GetLevel(const string &_level)
{
	if(isdigit(_level[0]))
		// A number, use it directly
		return atoi(_level.c_str()) % nLevels;

	if(_level[0] == 'L' || _level == "NO_LEVEL")
		// The 'L' special character
		return nLevels - 1;

	// Check for string level

	// Omit level 0

	if(_level == "VERBOSE" || _level == "TRACE")
		return 1;

	if(_level == "DETAIL" || _level == "DEBUG" || _level == "Debug")
		return 2;

	if(_level == "INFO" || _level == "Info" || _level == "Notice")
		return 3;

	if(_level == "WARNING" || _level == "Warning" || _level == "WARN")
		return 4;

	if(_level == "ERROR" || _level == "Error")
		return 5;

	if(_level == "SEVERE" || _level == "CRITICAL" || _level == "Critical" || _level == "Alert")
		return 6;

	if(_level == "FATAL" || _level == "Emergency")
		return 7;

	// Nothing found; use a random mapping

	return LogLevelMapping(_level);
}


string GetLevel(int _level)
{
	switch(_level)
	{
	case 0: return "";
	case 1: return "VERBOSE";
	case 2: return "DETAIL";
	case 3: return "INFO";
	case 4: return "WARNING";
	case 5: return "ERROR";
	case 6: return "SEVERE";
	case 7: return "FATAL";
	}

	return "FATAL";
}


int LogLevelMapping(const string &_level)
{
	int colorCode = 0;

	for(size_t i = 0; i < _level.size(); ++i)
	{
		colorCode += _level[i];		//+TODO: more randomness
	}

	colorCode = colorCode % 7;		// use the first 7 colors only

	return colorCode;
}


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

#ifdef POSIX
	struct stat st;
	if (stat(_logFile.c_str(), &st) == 0)
	{
		time_t date = st.st_mtime;
		logDate = ctime(&date);		//+ ctime() deprecated
	}
	else
		logDate = "?\n";
#else // _WIN32
	//+TEST
	TCHAR szBuf[MAX_PATH];
	HANDLE hFile = CreateFile(_logFile.c_str(), GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		logDate = "?\n";
	else {
		if (GetLastWriteTime(hFile, szBuf, MAX_PATH))
			logDate = szBuf;
		else
			logDate = "?\n";
		CloseHandle(hFile);
	}
#endif

	return logDate;
}


void PrintHelp(const ProgArgs &_args, const char* _progName)
{
	string progName = _progName;

	size_t pos = progName.rfind(slash) + 1;

	cout << string(110, '-') << "\n";
	cout << "\n" << progName.substr(pos) << ": a text mode log file viewer.\n";
	cout << "\n" << "Features:\n";
	cout << "\t- Log file format agnostic.\n";
	cout << "\t- Log level based highlighting. Levels can be numeric (1-7) or strings.\n";
	cout << "\t- Filtering capability.\n";
	cout << "\t- Text mode, runs everywhere after recompilation.\n";
	cout << "\t- Free software, GPL 3 license.\n";
	cout << "\nParameters:\n";
	_args.Help();
	cout << endl;

#ifdef POSIX
	cout << "Log levels highlighting: ";
	for(int level = 0; level < nLevels; ++level)
		cout << Format(level) << level << Reset() << " ";
	cout << endl;
#endif

	cout << "\nExample:\n";
	cout << "Print the logs in the specified file, with minimum level 1, "
			"with level placed in the second column, which include the "
			"substrings \"abc def\" and \"123\", which do not include the "
			"substring \"ghi\", with timestamp between 0.123 and 0.125 seconds, "
			"done after 2012-10-08T14:11:09, "
			"in verbose mode:\n\n";
	cout << progName.substr(pos)
		 << " -i /path/to/test.log -m 1 -l 2 -s \"abc def\" -s \"123\" -ns \"ghi\""
		 << " -gt 1_0.123 -lt 1_0.125 -gt 3_2012-10-08T14:11:09 -vb" << endl;

	cout << "\nKeystroke runtime commands:\n\n";
	cout << "\t [P]       Pause/resume logs display.\n";
	cout << "\t [1]-[7]   Change minimum log level of displayed logs (no effect on their generation).\n";
	cout << "\t [R]       Reload all the logs and display them with the current criteria.\n";
	cout << "\t [r]       Reload the last " << nLogsReload << " logs and display them with the current criteria.\n";
	cout << "\t [n]       Set the number of logs to reload (default is " << nLogsReload << ").\n";
	cout << "\t [Q]       Exit logviewer.\n";

	cout << "\n" << string(110, '-') << "\n";
	cout << endl;
}


void PrintVersion(const char* _progName)
{
	cout << string(80, '-') << "\n";
	string progName = _progName;
	size_t p = progName.find_last_of(slash) + 1;
	cout << "\n\t" << progName.substr(p) << " version " << version << "." << subversion << "." << subsubversion << endl;
	cout << "\n\t" << "Copyright 2012-2015 Pietro Mele" << endl;
	cout << "\n\t" << "Released under a GPL 3 license." << endl;
	cout << "\n\t" << "pietrom16@gmail.com"
		 << "\n\t" << "https://sites.google.com/site/pietrom16" << endl;
	cout << string(80, '-') << "\n";
	cout << endl;
}
