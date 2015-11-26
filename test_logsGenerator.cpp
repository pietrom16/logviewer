/******************************************************************************
 * test_logsGenerator.cpp
 *
 * Test application to generate random logs.
 * The number of logs can be specified.
 * If no parameter is passed, it generates an 'infinite' number of logs.
 *
 * Copyright (C) 2012-2015 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

std::string LogTime();
std::string LogDate();


int main(int argc, char* argv[])
{
	bool endless = true;
	int  nLogs = 0;

	if(argc > 1) {
		nLogs = atoi(argv[1]);
		endless = false;
	}

	std::string log;
	std::string logFile = "test.log";
	std::ofstream ofs(logFile);

	// Log levels (a custom levels enum can be used)
	static const int nLogLevels = 8;
	const char       logLevelTags[nLogLevels][9] { "  ----  ", "VERBOSE ", "DETAIL  ", "INFO    ", "WARNING ", "ERROR   ", "CRITICAL", "FATAL   " };
	int              level = 0;

	std::chrono::milliseconds pause(1000);

	int i = 0;

	while(endless || i < nLogs)
	{
		log = std::string("Test log message - ");
		log += std::to_string(rand() % 1000);

		level = rand() % nLogLevels;

		ofs << LogTime() << LogDate() << logLevelTags[level] << " " << log << std::endl;

		std::cout << "." << std::flush;

		std::this_thread::sleep_for(pause);

		++i;
	}

	return 0;
}


std::string LogTime() {
	float t = float(std::clock())/CLOCKS_PER_SEC;
	const size_t sz = 16;
	char ct[sz];
	std::snprintf(ct, sz, "% 7.3f  ", t);
	return std::string(ct);
}


std::string LogDate() {
	std::time_t t = std::time(nullptr);
	char mbstr[32];
	std::strftime(mbstr, sizeof(mbstr), "%F %T  ", std::localtime(&t));
	return std::string(mbstr);
}

