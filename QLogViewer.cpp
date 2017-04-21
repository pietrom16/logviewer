/******************************************************************************
 * QLogViewer.cpp
 *
 * Display log files in real time through the Qt GUI.
 *
 * Copyright (C) 2012-2017 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/

#include "QLogViewer.hpp"

namespace log_viewer {

int QLogViewer::RunQt()
{
	std::cout << "Launching LogViewer GUI..." << std::endl;

	auto mainDialog = new QPlainTextEdit;
	mainDialog->setPlainText("GUI is work in progress!");
	mainDialog->appendPlainText("Use the console mode in the meantime.");
	mainDialog->show();

	return 0;
}

} // log_viewer

