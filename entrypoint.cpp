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
#ifdef QT_GUI
#include "qlogviewer.hpp"
#endif

int main(int argc, char* argv[])
{
	using namespace log_viewer;

	ResetDefaults rd;

#ifdef RUN_INTERNAL_TESTS
	return RunInternalTests();
#endif // RUN_INTERNAL_TESTS


#ifndef QT_GUI
	LogViewer logViewer(argc, argv);
	logViewer.Run();
#else
	QLogViewer logViewer(argc, argv);
	if(logViewer.QtGui())
		logViewer.RunQt();
	else
		logViewer.Run();
#endif

	exit(0);
}
