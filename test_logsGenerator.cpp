/******************************************************************************
 * test_logsGenerator.cpp
 *
 * Test application to generate random logs.
 * The number of logs can be specified.
 * If no parameter is passed, it generates an 'infinite' number of logs.
 *
 * Copyright (C) 2012-2016 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

std::string LogTime();
std::string LogDate();


int main(int argc, char* argv[])
{
	bool endless = true;
	int  gen = 0, nGen = 4;
	int  nLogs = 10;
	int  pauseSec = 0;

	if(argc > 1) {

		const std::string arg(argv[1]);

		if(arg == "-h" || arg == "--help") {
			std::cout << argv[0] << " [nLogs] [distribution] [pause]\n"
					  << "distribution values:\n"
					  << "  0. Random"
					  << "  1. Increasing"
					  << "  2. Spikes"
					  << "  3. Triangle wave"
					  << std::endl;
			exit(0);
		}

		nLogs = atoi(argv[1]);
		endless = false;
	}

	if(argc > 2) {
		gen = atoi(argv[2]);
		if(gen >= nGen)
			gen = 0;
	}

	if(argc > 3) {
		pauseSec = atoi(argv[3]);
	}

	std::string log;
	std::string logFile = "test.log";
	std::ofstream ofs(logFile);

	// Log levels (a custom levels enum can be used)
	static const int nLogLevels = 8;
	const char       logLevelTags[nLogLevels][9] { "  ----  ", "VeRbOsE ", "DETAIL  ", "Info    ", "warninG ", "error   ", "CriticaL", "fATAl   " };
	int              level = 0;

	std::chrono::milliseconds pause(1000*pauseSec);

	int i = 0;

	while (endless || i < nLogs)
	{
		log = std::string("Test log message - ");
		log += std::to_string(rand() % 1000);
		log += std::string(" - Log id = ");
		log += std::to_string(i);

		if(gen == 0) {						// Random
			level = rand() % nLogLevels;
		}
		else if(gen == 1) {					// Increasing
			if(level >= nLogLevels)
				level = 0;
			else
				++level;
		}
		else if(gen == 2) {					// Spikes
			if(i % 8 == 0)
				level = 6;
			else
				level = 1;
		}
		else if(gen == 3) {					// Triangle wave
			level = abs((i % 14) - 7);
		}

		int format = rand() % 3;
		switch (format) {
			case 0:
				ofs << LogTime() << LogDate() << logLevelTags[level] << " " << log << std::endl;
				break;
			case 1:
				ofs << LogDate() << logLevelTags[level] << LogTime() << " " << log << std::endl;
				break;
			case 2:
				ofs << logLevelTags[level] << LogTime() << LogDate() << " " << log << std::endl;
				break;
		}

		std::cout << "." << std::flush;

		std::this_thread::sleep_for(pause);

		++i;
	}

	std::cout << "\n" << i << " test logs generated in file: " << logFile << std::endl;

	return 0;
}


std::string LogTime()
{
	float t = float(std::clock())/CLOCKS_PER_SEC;
	const size_t sz = 16;
	char ct[sz];

#ifndef _WIN32
	std::snprintf(ct, sz, "% 7.3f  ", t);
#else
	_snprintf_s(ct, sz, "% 7.3f  ", t);
#endif

	return std::string(ct);
}


std::string LogDate() {
	std::time_t t = std::time(nullptr);
	char mbstr[32];

#ifndef _WIN32  //+B
	std::strftime(mbstr, sizeof(mbstr), "%F %T  ", std::localtime(&t));
#else
	mbstr[0] = '\0';
#endif

	return std::string(mbstr);
}

