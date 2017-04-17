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
#include <QPlainTextEdit>

namespace log_viewer {


class QLogViewer : public LogViewer, public QPlainTextEdit
{

};


} // log_viewer

#endif // QLOGVIEWER_HPP
