/******************************************************************************
 * LogContext.hpp
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

#ifndef LOGCONTEXT_HPP
#define LOGCONTEXT_HPP


namespace LogViewer {


class LogContext
{
public:
	LogContext() :
		width(0), minLevelForContext(5 /*ERROR*/), minContextLevel(2 /*DETAIL*/) {}

	int StorePastLog(const std::string &_log, int _level)
	{
		if(width == 0)                return 0;
		if(_level < minContextLevel)  return 0;
		//+ if(_level >= minLevel)        return 0;		//+TODO - This should have been already printed
		if(pastLogs.size() >= width)  pastLogs.pop();
		pastLogs.push(_log);
		return pastLogs.size();
	}

private:
	int  width;					// number of logs before and after the current one
	int  minLevelForContext;	// the minimum level a log must have to get a context
	int  minContextLevel;		// the minimum level a log must have to be part of the context

	std::queue<std::string>  pastLogs;
};


} // LogViewer


#endif // LOGCONTEXT_HPP
