/******************************************************************************
 * CSS_default.h
 *
 * Default CSS code for HTML output log files.
 *
 * Copyright (C) 2012-2016 Pietro Mele
 * Released under a GPL 3 license.
 *
 * pietrom16@gmail.com
 *
 *****************************************************************************/

#ifndef CSS_DEFAULT_H
#define CSS_DEFAULT_H

#include <string>


namespace log_viewer {


static const std::string css_default =
    #include "logviewer.css"
    ;


} // log_viewer



#endif // CSS_DEFAULT_H
