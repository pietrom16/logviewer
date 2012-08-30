/// textModeFormatting.h

// Version 1.1

/** Utility to produce colored output in a text mode window.
 *  It provides an intuitive way to use ANSI color codes (ISO/IEC 6429).
 *  It can be disabled at compile time with: #define TEXT_MODE_FORMATTING_OFF
 */

/* Copyright Pietro Mele 2012
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


namespace textModeFormatting
{
	// Predefined text formats
	const char
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
	const char level[][nLevels] = {
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

#else
	static const int nLevels = 8;
	
	// Predefined text formats as levels

	const char level[][nLevels] = {
		"7;37m",	/* NO_LEVEL_, normal, grey background */
		"0;37m",	/* VERBOSE_, detail, grey */
		"1m",		/* DETAIL_, bold, white */
		"0;32m",	/* INFO_, normal, green */
		"0;33m",	/* WARNING_, normal, yellow */
		"1;31m",	/* ERROR_, bold, red */
		"7;31m",	/* SEVERE_, red background */
		"5;31m"		/* FATAL_, blinking red */
	};

#endif

#ifndef TEXT_MODE_FORMATTING_OFF
    
	const char* Format(const char* _format) {
		static std::string format;
		format = "\033[";
		format.append(_format);
		return format.c_str();
	}
	
	const char* Format(int _formatId) {
		static std::string format;
		format = "\033[";
		format.append(level[_formatId%nLevels]);
		return format.c_str();
	}
	
	const char* Reset() {
		static char format[] = "\033[0m\0";
		return format;
	}
	
#else // TEXT_MODE_FORMATTING_OFF

	const char* Format(const char* _format) {
		static char format[] = "";
		return format;
	}
	
	const char* Format(int _formatId) {
		static char format[] = "";
		return format;
	}
	
	const char* Reset() {
		static char format[] = "";
		return format;
	}
    
#endif // TEXT_MODE_FORMATTING_OFF

} // textModeFormatting

#endif // TEXT_MODE_FORMATTING_H
