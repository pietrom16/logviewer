/******************************************************************************
 * LogContext.hpp
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

#ifndef LOGCONTEXT_HPP
#define LOGCONTEXT_HPP

#include <queue>
#include <string>


namespace LogViewer {


class LogContext
{
public:
	LogContext() :
		width(0), minLevelForContext(5 /*ERROR*/), minContextLevel(10 /*context disabled*/) {}

	int StorePastLog(const std::string &_log, int _level, int _minLevel);
	int ExtractPastLog(std::string &_log);

	int Width()              const { return width; }
	int MinLevelForContext() const { return minLevelForContext; }
	int MinContextLevel()    const { return minContextLevel; }
	int NPastLogs()          const { return pastLogs.size(); }

	int Width(int _w)                { return (width = (_w >= 0)?_w:width); }
	int MinLevelForContext(int _ml)  { return (minLevelForContext = (_ml >= 0)?_ml:minLevelForContext); }
	int MinContextLevel(int _ml)     { return (minContextLevel = (_ml >= 0)?_ml:minContextLevel); }

private:
	int  width;					// number of logs before and after the current one
	int  minLevelForContext;	// the minimum level a log must have to get a context
	int  minContextLevel;		// the minimum level a log must have to be part of the context

	std::queue<std::string>  pastLogs;
};


} // LogViewer


#endif // LOGCONTEXT_HPP
