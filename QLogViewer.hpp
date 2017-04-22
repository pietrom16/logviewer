/******************************************************************************
 * QLogViewer.hpp
 *
 * Display log files in real time through the Qt GUI.
 *
 * Copyright (C) 2012-2017 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/

#ifndef QLOGVIEWER_HPP
#define QLOGVIEWER_HPP

#include "logviewer.hpp"
#include <iostream>
#include <QPlainTextEdit>

namespace log_viewer {


class QLogViewer : public LogViewer
{
public:
	QLogViewer(int argc, char* argv[]) : LogViewer(argc, argv) {}
	virtual ~QLogViewer() {}

	virtual int RunQt();
};


} // log_viewer

#endif // QLOGVIEWER_HPP

/** References
 *  http://www.qtcentre.org/threads/51208-example-of-QPlainTextEdit-subclass-to-handle-large-text-files?s=dfb71c603e0e7db09af8f02331c95cd9
 *  https://github.com/Andersbakken/LazyTextEdit
 */
