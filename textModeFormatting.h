/// textModeFormatting.h

// Version 2.0

/** Utility to produce colored output in a text mode window.
 *  It provides an intuitive way to use ANSI color codes (ISO/IEC 6429).
 *  It can be disabled at compile time with: #define TEXT_MODE_FORMATTING_OFF
 */

/* Copyright (C) 2012-2015 Pietro Mele
 * Released under a BSD license.
 * pietrom16@gmail.com
 */

/* Usage example:

	#include "textModeFormatting.h"
	using namespace textModeFormatting;
	
	cout << Format(ny) << "Text..." << Reset();
	cout << Format(2)  << "Text..." << Reset();
*/

#ifndef TEXT_MODE_FORMATTING_H
#define TEXT_MODE_FORMATTING_H

#ifndef TEXT_MODE_FORMATTING_OFF
#include <string>
#endif // TEXT_MODE_FORMATTING_OFF

#ifdef _WIN32
#include <tchar.h>
#include <strsafe.h>
#include <windows.h>
#endif

namespace textModeFormatting
{

    // HTML format
    static const int nHtmlLevels = 32;
	static const char htmlLevel[][nHtmlLevels] = {
	    "color:grey",				/* not used */
	    "color:cyan",				/* VERBOSE_, detail, cyan */
	    "color:white",				/* DETAIL_, normal, white */
	    "color:green",				/* INFO_, normal, green */
	    "color:yellow",				/* WARNING_, normal, yellow */
	    "color:red",				/* ERROR_, bold, red */
	    "background-color:red",		/* SEVERE_, red background */
	    "color:red",				/* FATAL_, blinking red */
	    "background-color:grey"		/* NO_LEVEL_, normal, grey background */
	};

#ifndef _WIN32

	// Predefined text formats
	static const char
		d[] = "0;37m",		/* detail, grey */
		n[] = "0m",			/* normal, white */
		b[] = "1m",			/* bold, white */
		ng[] = "0;32m",		/* normal, green */
		bg[] = "1;32m",		/* bold, green */
		nc[] = "0;36m",		/* normal, cyan */
		bc[] = "1;36m",		/* bold, cyan */
		ny[] = "0;33m",		/* normal, yellow */
		by[] = "1;33m",		/* bold, yellow */
		nr[] = "0;31m",		/* normal, red */
		br[] = "1;31m",		/* bold, red */
		gb[] = "7;32m",		/* green background */
		yb[] = "7;33m",		/* yellow background */
		rb[] = "7;31m",		/* red background */
		bly[] = "5;33m",	/* blinking yellow */
		blr[] = "5;31m";	/* blinking red */

#ifdef TEXTMODEFORMATTING_DEFAULT_COLORS
	static const int nLevels = 16;
	
	// Predefined text formats as levels
	static const char level[][nLevels] = {
		"0;37m",	/* detail, grey */
		"0m",		/* normal, white */
		"1m",		/* bold, white */
		"0;32m",	/* normal, green */
		"1;32m",	/* bold, green */
		"0;36m",	/* normal, cyan */
		"1;36m",	/* bold, cyan */
		"0;33m",	/* normal, yellow */
		"1;33m",	/* bold, yellow */
		"0;31m",	/* normal, red */
		"1;31m",	/* bold, red */
		"7;32m",	/* green background */
		"7;33m",	/* yellow background */
		"7;31m",	/* red background */
		"5;33m",	/* blinking yellow */
		"5;31m"		/* blinking red */
	};

#else // TEXTMODEFORMATTING_DEFAULT_COLORS

	static const int nLevels = 8;
	
	// Predefined text formats as levels

	static const char level[][nLevels] = {
		"0m",		/* not used */
		"0;36m",	/* VERBOSE_, detail, cyan */
		"0m",		/* DETAIL_, normal, white */
		"0;32m",	/* INFO_, normal, green */
		"0;33m",	/* WARNING_, normal, yellow */
		"1;31m",	/* ERROR_, bold, red */
		"7;31m",	/* SEVERE_, red background */
		"5;31m",	/* FATAL_, blinking red */
		"7;37m"		/* NO_LEVEL_, normal, grey background */
	};

#endif // TEXTMODEFORMATTING_DEFAULT_COLORS

#else // _WIN32

	// Predefined text formats
	static const int
		d = 7,			/* detail, grey */
		n = 7,			/* normal, white */
		b = 7,			/* bold, white */
		ng = 10,		/* normal, green */
		bg = 10,		/* bold, green */
		nc = 11,		/* normal, cyan */
		bc = 11,		/* bold, cyan */
		ny = 14,		/* normal, yellow */
		by = 14,		/* bold, yellow */
		nr = 12,		/* normal, red */
		br = 12,		/* bold, red */
		gb = 10,		/* green background */
		yb = 14,		/* yellow background */
		rb = 12,		/* red background */
		bly = 14,		/* blinking yellow */
		blr = 12;	/* blinking red */

	static const int nLevels = 8;

	// Predefined text formats as levels

	static const int level[][nLevels] = {
		{ 0  },	/* not used */
		{ 11 },	/* VERBOSE_, cyan */
		{ 07 },	/* DETAIL_, white */
		{ 10 },	/* INFO_, green */
		{ 14 },	/* WARNING_, yellow */
		{ 12 },	/* ERROR_, red */
		{ 13 },	/* SEVERE_, purple */
		{ 13 },	/* FATAL_, purple */
		{ 07 }	/* NO_LEVEL_, white */
	};

#endif // _WIN32

#ifndef TEXT_MODE_FORMATTING_OFF

#ifndef _WIN32

	static const char* Format(const char* _format) {
		static std::string format;
		format = "\033[";
		format.append(_format);
		return format.c_str();
	}
	
	static const char* Format(int _formatId) {
		static std::string format;
		format = "\033[";
		format.append(level[_formatId%nLevels]);
		return format.c_str();
	}
	
	static const char* Reset() {
		static char format[] = "\033[0m\0";
		return format;
	}

#else // _WIN32

	static const char* Format(int _formatId) {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), level[_formatId%nLevels][0]);
		static char format[] = "";
		return format;
	}

	static const char* Reset() {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 07);
		static char format[] = "";
		return format;
	}

#endif // _WIN32

#else // TEXT_MODE_FORMATTING_OFF

	static const char* Format(const char* _format) {
		static char format[] = "";
		return format;
	}
	
	static const char* Format(int _formatId) {
		static char format[] = "";
		return format;
	}
	
	static const char* Reset() {
		static char format[] = "";
		return format;
	}
    
#endif // TEXT_MODE_FORMATTING_OFF

} // textModeFormatting

#endif // TEXT_MODE_FORMATTING_H
