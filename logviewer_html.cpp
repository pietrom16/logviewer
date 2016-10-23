/******************************************************************************
 * logviewer_Html.cpp
 *
 * Utility to display log files in real time on and HTML file.
 *
 * Copyright (C) 2012-2016 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/


#include "logviewer.hpp"
#include <cassert>
#include <fstream>
#include <iostream>

using std::endl;

namespace log_viewer {


int LogViewer::CheckLogFilesDiagnostic() const
{
	int status = 0;

	if(htmlOutStream.good() == true)       status  = 0;
	if(htmlOutStream.is_open() == false)   status -= 1;
	if(htmlOutStream.fail() == true)       status -= 10;
	if(htmlOutStream.bad()  == true)       status -= 100;
	if(htmlOutStream.eof()  == true)       status -= 1000;

	return status;
}


int LogViewer::WriteHeader_html()
{
	int n = 0;

	/* //+B+
	// Write the header only if the HTML file does not start with "<!DOCTYPE"

	htmlOutStream.seekg(0, std::ios_base::beg);
	std::string token;
	htmlOutStream >> token;

	cerr << "Token = " << token << endl; //+T+ok

	if(token != "<!DOCTYPE") {
		cerr << logFormatter.HeaderHTML() << endl; //+T+ok
		htmlOutStream.seekp(0, std::ios_base::beg);
		htmlOutStream << logFormatter.HeaderHTML() << endl;
		AddHtmlControls();
		++n;
	}
	*/

	// Write the header only if the HTML file is shorter than a fixed size
	htmlOutStream.seekg(0, std::ios_base::end);
	const std::streamoff size = htmlOutStream.tellg();

	if(size < 10) {
		htmlOutStream.seekp(0, std::ios_base::beg);
		htmlOutStream << logFormatter.HeaderHTML() << endl;
		AddHtmlControls();
		htmlOutStream << logFormatter.TitleHTML() << endl;
		++n;
	}

	return n;
}


int LogViewer::WriteFooter_html()
{
	MoveBackToEndLogsBlock_html();
	//+TODO AddHtmlControls();

	htmlOutStream << logFormatter.FooterHTML() << endl;

	return 1;
}


/// Log footer: move before HTML output file's closing tag

int LogViewer::MoveBackToEndLogsBlock_html()
{
	// Search backwards for the end of the logs block
	// Look for the last log message

	/** Assumed end of HTML file structure:
				...logs...
				<br> <span style="color:red;">Log message</span>
				<table> ... </table>
			</body>
		</html>
	*/

	using namespace std;

	string        token, line;
	const string  logsEndToken_span("</span>");
	const string  logsBegToken_table("<table>");
	const string  logsEndToken_body("</body>");
	size_t        posTokenLine      = 0;	// token position in the current line
	streamoff     pos               = 0;	// current position in the file
	streamoff     posNewLogs        = 0;	// position for the next new log
	streamoff     posEndToken_span  = 0;	// </span> token position in the file
	streamoff     posBegToken_table = 0;	// <table> token position in the file
	streamoff     posBegToken_body  = 0;	// </body> token position in the file

	long assumedFooterLength = 400;		// approximation in excess

	// Start from the end of the log file
	htmlOutStream.seekg(0, ios_base::end);
	htmlOutStream.seekp(0, ios_base::end);

	// Length of the current log file
	const streamsize size = htmlOutStream.tellg();

	if(size < assumedFooterLength)
		assumedFooterLength = size;

	bool logsShown = false;  // true when at least one log has been shown on the HTML page
	bool lastTable = false;  // to discriminate between first and last table

	// Go back a fixed number of characters
	htmlOutStream.seekg(-assumedFooterLength, ios_base::end);

	/// Find the point where to start adding new logs

	while(!htmlOutStream.eof())		//+TODO Update
	{
		pos = htmlOutStream.tellg();

		getline(htmlOutStream, line);

		cerr << "pos: " << pos << " log: " << line << endl; //+T+

		if(line.find(logsEndToken_span) != string::npos) {
			posEndToken_span = htmlOutStream.tellg();   // move after the element
			logsShown = true;
		}

		if(line.find(logsBegToken_table) != string::npos) {
			if(logsShown) {
				lastTable = true;
				posBegToken_table = pos;	// move before the element
			}
		}

		if(line.find(logsEndToken_body) != string::npos) {
			posBegToken_body = pos;		// move before the element
		}
	}

	if(posBegToken_table && lastTable)
		posNewLogs = posBegToken_table;
	else if(posBegToken_body)
		posNewLogs = posBegToken_body;
	else if(posEndToken_span)
		posNewLogs = posEndToken_span;
	else
		posNewLogs = 0;		// end of the logs block not found

	cerr << "</span> = " << posEndToken_span << "; <table> = " << posBegToken_table << "; </body> = " << posBegToken_body << endl; //+T+
	cerr << "Pos new logs = " << posNewLogs << endl; //+T+

	/// Move to posNewLogs	//+TODO

	// Reset htmlOutStream error state flags	//+?
	htmlOutStream.clear();

	if(posNewLogs != 0) {
		//+TEST
		cerr << "Status 1 = " << CheckLogFilesDiagnostic() << endl; //+B+ EOF & FAIL

		htmlOutStream.seekg(posNewLogs, ios_base::beg);
		htmlOutStream.seekp(posNewLogs, ios_base::beg);

		//+TEST
		cerr << "Status 2 = " << CheckLogFilesDiagnostic() << endl; //+B+ EOF & FAIL
		const streamoff posg = htmlOutStream.tellg();
		const streamoff posp = htmlOutStream.tellp();
		cerr << posg << " = " << posp << " = " << posNewLogs << " ???" << endl;
		assert(posg == posNewLogs);
		assert(posp == posNewLogs);
	}
	else {
		// End of the logs block not found
		htmlOutStream.seekg(-1, ios_base::end);
		htmlOutStream.seekp(-1, ios_base::end);
	}

	//htmlOutStream << "xxx" << flush; //+T+ OK

	// Reset htmlOutStream error state flags
	htmlOutStream.clear();

	return 0;
}


/// Add widgets to control/filter the HTML output

int LogViewer::AddHtmlControls()
{
	//+TODO - Put real controls here

	htmlOutStream
	   << "\t\t<table style=\"width:100%\">"
	   << "	<tr>"
	   << "		<th>Reload</th>"
	   << "		<th>Min log level = </th>"
	   << "		<th>Number</th>"
	   << "		<th>Verbose</th>"
	   << "		<th>Show file name</th>"
	   << "		<th>Include</th>"
	   << "		<th>Exclude</th>"
	   << "		<th>Context</th>"
	   << "		<th>Width</th>"
	   << "		<th>Min level for context</th>"
	   << "		<th>Min context level</th>"
	   << "		<th></th>"
	   << "	</tr>"
	   << "</table>"
	   << endl;

	return -1;
}


} // log_viewer
