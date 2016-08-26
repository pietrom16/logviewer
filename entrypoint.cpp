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

#include "logviewer.hpp"
#include <iostream> //+T+

int main(int argc, char* argv[])
{

	if(1) { //+T+++OK
		std::ifstream ifs("./test.log");

		for(int i = 1; i < 20; ++i) {
			ifs.seekg(-i, std::ios_base::end);
			char c = char(ifs.get());
			std::cerr << c;
		}

		exit(0);
	}

	using namespace log_viewer;

	ResetDefaults rd;

#ifdef RUN_INTERNAL_TESTS
	return RunInternalTests();
#endif // RUN_INTERNAL_TESTS


	LogViewer logViewer(argc, argv);

	logViewer.Run();

	exit(0);
}
