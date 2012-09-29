/******************************************************************************
 * logviewer.cpp
 *
 * Version 1.5.1
 *
 * Utility to display log files in real time.
 *
 * Copyright (C) Pietro Mele 2012
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
	- Multiple input logs.
	- Randomize better the colors in LogLevelMapping().
	- Allow to select logs on the basis of their ID.
	- In the header, add the date of the log file in Windows (done for POSIX).
 */

#include "progArgs.h"
#include "textModeFormatting.h"
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#if defined(__unix__) || defined(__linux__) || \
    defined(BSD) || (defined (__APPLE__) && defined (__MACH__)) || defined(__bsdi__) || \
    defined(__minix) || defined(__CYGWIN__) || defined(__FreeBSD__)
#define POSIX 1
#include <sys/stat.h>
#endif

using namespace std;
using namespace textModeFormatting;
using namespace Utilities;

#ifndef WIN32
	static const char slash = '/';
#else
	static const char slash = '\\';
#endif


const int version = 1, subversion = 5, subsubversion = 1;


int GetLevel(const string &_level);
int LogLevelMapping(const string &_level);
void PrintHelp(const ProgArgs &_args, const char* _progName);
void PrintVersion(const char* _progName);


int main(int argc, char* argv[])
{
	int levelColumn = 0;				// depends on the logs format
	int minLevel = 0;					// minimum level a log must have to be shown
	unsigned int beepLevel = -1;		// minimum level to get an audio signal
	
	const int printAll = -1;
	int nLatest = printAll;				// number of latest logs to be printed (-1 = all)
	int nLatestChars = printAll;		// number of latest characters to be printed (-1 = all)
	
	string includeString, excludeString;				// sub strings to be included/excluded by the logs
	bool   incStrFlag = false, excStrFlag = false;		// flags to decide whether to check for substrings
	
	struct timespec pause;
    pause.tv_sec  = 1;
    pause.tv_nsec = 0;
	
	int verbose = 0;

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
		exit(0);
	}
	
	if(arguments.GetValue("--version"))
	{
		PrintVersion(argv[0]);
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
		if(arguments.GetValue("--subString", includeString) >= 0)
			incStrFlag = true;
	
	if(arguments.GetValue("--notSubString"))
		if(arguments.GetValue("--notSubString", excludeString) >= 0)
			excStrFlag = true;
	
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
    pause.tv_sec  = trunc(fPause);
    pause.tv_nsec = 1e9 * (fPause - pause.tv_sec);

	/// Print header
	
	string logDate;		// time the log was generated
	
#ifdef POSIX
	struct stat st;
	if(stat(logFile.c_str(), &st) == 0)
	{
		time_t date = st.st_mtime;
		logDate = ctime(&date);		//+ ctime() deprecated
	}
	else
	{
		std::time_t t = std::time(NULL);
		logDate = std::asctime(std::localtime(&t));
	}
#else
	if(false)
	{
		//+TODO
		logDate = "\n";
	}
	else
	{
		std::time_t t = std::time(NULL);
		logDate = std::asctime(std::localtime(&t));
	}
#endif
	
	cout << string(80, '-') << "\n";
	cout << "LogViewer " << version << "." << subversion << " - "
	     << "Log file: " << logFile << " - " << logDate;
	cout << string(80, '-') << endl;
	
	/// Print extra info
	if(verbose)
	{
		cout << "Column ID containing the log level: " << levelColumn << endl;
		cout << "Minimum log level for a log to be shown: " << minLevel << endl;
		
		if(incStrFlag)
			cout << "Show logs which include the string: " << includeString << endl;
		if(excStrFlag)
			cout << "Hide logs which include the string: " << excludeString << endl;

		cout << "Interval between checks of the log file: " << pause.tv_sec + 1.0e-9*pause.tv_nsec << " seconds" << endl;
		
		if(nLatestChars >= 0)
			cout << "Showing the last " << nLatestChars << " characters of the existing log file." << endl;
		else if(nLatest >= 0)
			cout << "Showing the last " << nLatest << " logs of the existing log file." << endl;
		else
			cout << "Number of past logs to be shown: all" << endl;

		cout << string(80, '-') << endl;
	}
	
	/// Open log file
	
	ifstream ifs;
	
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
		
		nanosleep(&pause, NULL);
	}
	
	/// Print log file
	
	string log,
	       token;
	int    level = 0;
	
	ios::pos_type pos;
	
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
		while(getline(ifs, log))
		{
			stringstream str(log);
			for(int i = 0; i < levelColumn; ++i)
				str >> token;

			level = GetLevel(token);
			
			if(level >= minLevel)
			{
				if(incStrFlag) {
					if(log.find(includeString) == string::npos)
						continue;
				}
				
				if(excStrFlag) {
					if(log.find(excludeString) != string::npos)
						continue;
				}
				
#ifdef POSIX
				cout << Format(level) << log << Reset() << endl;
#else
				cout << log << endl;
#endif
				
				if(level >= beepLevel)
					cout << char(7) << flush;	// beep
			}
		}
		
		if(!ifs.eof()) {
			cerr << argv[0] << " error: could not read this log file till the end: " << logFile << endl;
			break;
		}
		
		ifs.clear();		// clear the eof state to keep reading the growing log file

		nanosleep(&pause, NULL);
	}
	
	return 0;
}


int GetLevel(const string &_level)
{
	if(isdigit(_level[0]))
		// A number, use it directly
		return atoi(_level.c_str()) % nLevels;
	
	if(_level[0] == 'L' || _level == "NO_LEVEL")
		// The 'L' special character
		return 0;
	
	// Check for string level
	
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


int LogLevelMapping(const string &_level)
{
	int colorCode = 0;
	
	for(size_t i = 0; i < _level.size(); ++i)
	{
		colorCode += _level[i];		//+TODO: more randomness
	}
	
	colorCode = colorCode % 6 + 1;		// use the first 6 colors only
	
	return colorCode;
}


void PrintHelp(const ProgArgs &_args, const char* _progName)
{
	string progName = _progName;

	size_t pos = progName.rfind(slash) + 1;
	
	cout << string(110, '-') << "\n";
	cout << "\n" << progName.substr(pos) << ": a text mode log file viewer." << endl;
	cout << "\n" << "Features:" << endl;
	cout << "\t- Log file format agnostic." << endl;
	cout << "\t- Log level based highlighting. Levels can be numeric (1-7) or strings." << endl;
	cout << "\t- Filtering capability." << endl;
	cout << "\t- Text mode, runs everywhere after recompilation." << endl;
	cout << "\t- Free software, BSD license." << endl;
	cout << "\nParameters:" << endl;
	_args.Help();
	cout << endl;

#ifdef POSIX
	cout << "Log levels highlighting: ";
	for(int level = 0; level < nLevels; ++level)
		cout << Format(level) << level << Reset() << " ";
	cout << endl;
#endif

	cout << string(110, '-') << "\n";
	cout << endl;
}


void PrintVersion(const char* _progName)
{
	cout << string(80, '-') << "\n";
	string progName = _progName;
	size_t p = progName.find_last_of(slash) + 1;
	cout << "\n\t" << progName.substr(p) << " version " << version << "." << subversion << "." << subsubversion << endl;
	cout << "\n\t" << "Copyright 2012 Pietro Mele" << endl;
	cout << "\n\t" << "Released under a GPL 3 license." << endl;
	cout << "\n\t" << "pietrom16@gmail.com"
	     << "\n\t" << "https://sites.google.com/site/pietrom16" << endl;
	cout << string(80, '-') << "\n";
	cout << endl;
}
