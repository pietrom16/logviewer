/******************************************************************************
 * entrypoint.cpp
 *
 * logviewer: Utility to display log files in real time on the console.
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

#include <string>
#include "logviewer.hpp"


int main(int argc, char* argv[])
{
	using namespace log_viewer;

#ifdef RUN_INTERNAL_TESTS
	return RunInternalTests();
#endif // RUN_INTERNAL_TESTS


	// Reset console colors
#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
#endif


	LogViewer logViewer(argc, argv);


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

		cout << "\nLog levels highlighting:\n";
		for(int level = 1; level <= logLevels.NLevels(); ++level) {
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

		cout << "Command line parameters: ";
		for(size_t i = 0; i < argc; ++i)
			cout << argv[i] << " ";
		cout << endl;


		cout << string(100, '-') << endl;
	}

	/// Open log file

	ifstream   ifs;
	string     line, log, token, contextLog;

	streamoff  pos = 0;					// position of the current log
	streamoff  lastPrintedLogPos = 0;	//+? position of the last log with level above the threshold
	streamoff  prevLogContext = 0;		//+? cursor exploring previous logs to provide context

	int  distNextLogContext = 0;		// distance of a past log from the current one
	int  distPrevLogContext = 100;		// distance of a future log from the current one
	bool isPostContextLog = false;		// the current log is part of the context

	bool warning = true;

	int  nReadLogs = 0;			// number of read logs
	int  nPrintedLogs = 0;		// number of printed logs

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
	bool newLine = false;

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

	if(newLogsOnly)
	{
		// Read only the logs generated from now on; discard the past

		// Reposition the cursor at the end of the file
		ifs.seekg(0, ios::end);
		pos = ifs.tellg();
	}
	else if(nLatestChars >= 0)
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

	// Main loop
	while(true)
	{
		while(!ifs.eof())
		{
			if(ifs.tellg() != -1)
			{
				getline(ifs, line);

				if(line.empty())
					break;

				++nReadLogs;

				string::size_type pos_beg = 0, pos_end = 0;

				while(pos_beg != string::npos)
				{
					pos_end = line.find_first_of(delimiters, pos_beg);		//+TODO - Include the delimiter

					if(pos_end != string::npos) {
						log = line.substr(pos_beg, pos_end - pos_beg + 1);		//+TODO test with pos_end = npos
						pos_beg = pos_end + 1;
					}
					else {
						log = line.substr(pos_beg);
						pos_beg = string::npos;
					}

					++logNumber;

					level = logLevels.FindLogLevel(log, levelColumn);

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
						pos = ifs.tellg();
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

							contextLevel = logLevels.FindLogLevel(contextLog, levelColumn);

							if(newLine) {
								cout << endl;
								newLine = false;
							}

							logStream << logFormatter.Format(contextLog, contextLevel, logFileField, '-', logNumberField) << endl;

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

							logStream << logFormatter.Format(log, level, logFileField, contextSign, logNumberField) << endl;

							++nPrintedLogs;

							if(beepLevel >= 0 && level >= beepLevel)
								cout << char(7) << flush;	// beep
						}

						lastPrintedLogPos = pos;
					}

				}

				nextLine:
				pos = ifs.tellg();
			}
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
		if(textParsing == false)
			this_thread::sleep_for(pause);

		if(verbose) {
			cout << "." << flush;
			newLine = true;
		}
	}

	rdKb.~ReadKeyboard();

	logStream << "\nTotal number of logs so far: " << logNumber << std::endl;

	return 0;
}


namespace log_viewer {


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
