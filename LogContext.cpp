/******************************************************************************
 * LogContext.cpp
 *
 * Context of the current log, i.e. set of logs that may be shown if the
 * current log's level is above a specific threshold.
 *
 * Copyright (C) 2012-2016 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/

#include "LogContext.hpp"
#include <iostream>


namespace LogViewer {


int LogContext::StorePastLog(const std::string &_log,
							 int _level,
							 int _minLevel,
							 int _logNumberPre)
{
	if(width == 0)                return 0;			// no context
	if(_level < minContextLevel)  return 0;			// below context threshold
	if(_level >= _minLevel)       return 0;			// already printed
	if(pastLogs.size() >= width)  pastLogs.pop();	// flush oldest log
	pastLogs.push(PastLog(_log, _logNumberPre));
	return pastLogs.size();
}


int LogContext::ExtractPastLog(std::string &_log)
{
	if(pastLogs.empty())
		return -1;

	_log = pastLogs.front().log;
	int logNum = pastLogs.front().logNumber;
	pastLogs.pop();

	return logNum;
}


void LogContext::Dump() const
{
	std::queue<PastLog>  pastLogsTmp(pastLogs);

	std::cout << "--- Log's past context: ---\n";

	while(pastLogsTmp.empty() == false)
	{
		std::cout << "Pre-Context: ["
				  << pastLogsTmp.front().logNumber
				  << "] "
				  << pastLogsTmp.front().log << "\n";
		pastLogsTmp.pop();
	}

	std::cout << "---------------------------" << std::endl;
}


} // LogViewer
