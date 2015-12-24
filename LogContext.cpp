/******************************************************************************
 * LogContext.cpp
 *
 * Context of the current log, i.e. set of logs that may be shown if the
 * current log's level is above a specific threshold.
 *
 * Copyright (C) 2012-2015 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/

#include "LogContext.hpp"


namespace LogViewer {


int LogContext::StorePastLog(const std::string &_log,
							 int _level,
							 int _minLevel)
{
	if(width == 0)                return 0;			// no context
	if(_level < minContextLevel)  return 0;			// below context threshold
	if(_level >= _minLevel)       return 0;			// already printed
	if(pastLogs.size() >= width)  pastLogs.pop();	// flush oldest log
	pastLogs.push(_log);
	return pastLogs.size();
}


} // LogViewer
