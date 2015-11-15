/******************************************************************************
 * test_logsGenerator.cpp
 *
 * Test application to generate random logs.
 *
 * Copyright (C) 2012-2015 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/

#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

std::string LogTime();
std::string LogDate();


int main()
{
	std::string log;
	std::string logFile = "test.log";
	//+OK std::ofstream ofs(logFile);

	// Log levels (a custom levels enum can be used)
	static const int nLogLevels = 8;
	const char       logLevelTags[nLogLevels][9] { "  ----  ", "VERBOSE ", "DETAIL  ", "INFO    ", "WARNING ", "ERROR   ", "CRITICAL", "FATAL   " };
	int              level = 0;

	struct timespec pause;
	pause.tv_sec  = 0;
	pause.tv_nsec = 0.25e9;


	while(true)
	{
		log = std::string("Test log message - ");
		log += std::to_string(rand() % 1000);

		level = rand() % nLogLevels;

		//+ ofs
		std::cout << LogTime() << LogDate() << logLevelTags[level] << " " << log << std::endl;

		std::cout << "." << std::flush;
		nanosleep(&pause, NULL);
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

