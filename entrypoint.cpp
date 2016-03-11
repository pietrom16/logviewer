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

const int version = 4, subversion = 99, subsubversion = 0;
/* Versioning conventions:
 *	- Even subversion number: stable version.
 *	- Odd subversion number: unstable/development version.
 */


int main(int argc, char* argv[])
{
	using namespace log_viewer;

	ResetDefaults rd;

#ifdef RUN_INTERNAL_TESTS
	return RunInternalTests();
#endif // RUN_INTERNAL_TESTS


	LogViewer logViewer(argc, argv);

	logViewer.Run();

	exit(0);
}
