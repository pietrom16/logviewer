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
#include <iostream>
#include <string>
#ifdef QT_GUI
#include "QLogViewer.hpp"
#include <QApplication>
#endif

[[ noreturn ]] void PrintHelp();


int main(int argc, char* argv[])
{
	using namespace log_viewer;

	ResetDefaults rd;

#ifdef RUN_INTERNAL_TESTS
	return RunInternalTests();
#endif // RUN_INTERNAL_TESTS

	// Parameters parsing

	bool consoleMode = false;

	for(int i = 0; i < argc; ++i)
	{
		const std::string arg = argv[i];
		if(arg == "--gui" || arg == "--qtgui")
			consoleMode = false;
		if(arg == "--console" || arg == "-c")
			consoleMode = true;
		if(arg == "--help" || arg == "-h")
			PrintHelp();
	}

#ifndef QT_GUI
	LogViewer logViewer(argc, argv);
	logViewer.Run();

	exit(0);
#else
	QApplication app(argc, argv);
//+	QLogViewer *logViewer = new QLogViewer();
//+	if(logViewer->QtGui())
//+		logViewer->RunQt();
//+	else
//+	    logViewer->Run();

	return app.exec();
#endif
}


void PrintHelp()
{
	using std::cout;
	using std::endl;

	cout << "\n"
	    << "logviewer: Utility to display log files in real time on the console and on the GUI.\n"
	    << "\n"
	    << "Copyright (C) 2012-2016 Pietro Mele\n"
	    << "Released under a GPL 3 license.\n"
	    << "\n"
	    << "Parameters:\n"
	    << "\n"
	    << "--console, -c    run in the text mode console.\n"
	    << "--gui            run in the default GUI mode.\n"
	    << "--qtgui          run in the Qt GUI mode.\n"
	    << "--help, -h       print this help message.\n";

	cout << endl;

	exit(0);
}
