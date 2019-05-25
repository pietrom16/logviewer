# TODO - logviewer

--- Qt5 GUI
    . Check why Qt is not found.
		QTDIR=/opt/Qt/Qt5.8.0/5.8/clang_64
		PATH=$PATH:/opt/Qt/Qt5.8.0/5.8/clang_64/bin
		--> TODO: double check!
	. It must be optional:
	    . At build time (conditional compilation).
		. At run time (launch in text mode/graphics mode).
	. Based on: QPlainTextEdit (http://doc.qt.io/qt-5/qplaintextedit.html)
	    - https://forum.qt.io/topic/44640/using-qplaintextedit-as-a-log-viewer
	- Start from an example, and refine it:
	    - Copy the Qt bits from the test project to this one.

--- HTML5 output
    -- Add controls:
	    - Go back before the controls.
		- Minimum level: [1]-[7], up and down.
		- Input log file name.
		- Load custom log levels from file (format: tag value\n).
		- Parse the input file as a generic text, not as a log file.
		- Level above which an audio signal is produced (default = -1).

    -- Write commands from HTML into the command file.
	-- Read commands from the app.

- CMake:
	- Fix getting command line parameters.
	- Use a specific compiler.

- Split logviewer.cpp in multiple shorter files.
    . Separated HTML member functions.

- Check command line user interaction is complete enough.

- Add option to delete existing output file before adding new logs.


-- Show multiple log files.

- Use set of conditions to pick logs even if below the threshold when other logs are met (i.e. extend the context feature).
- Use conditions to pick specific logs on the basis of other logs.
    E.g.:
	- If a specific log is present, change min log level.
	- If a specific log is present, show all logs with specified token.
	- ...

- Change pause functionality: stop loading new logs, but keep interacting.

. ProgArgs: Check if multiple parameters with the same tag can coexist. OK.
	-- Allow to pass multiple values for each command line parameter (especially inclusion/exclusion tokens).

- Add logviewer aliases with default parameters for specific uses.

- Better randomize the colors in LogLevelMapping().

- LogFormatter: markdown formatting.

-- rdKb: destroied, but never recreated.


## BUGS

- Bug: Check if it is normal that destructors are not called before exit.
- Bug: Check timing in text view.
- Bug: After running in GUI mode, the text console behaves erroneously.


## TESTING

--input "./test/testText.txt" --logLevels "./test/testText_logLevels.txt" -ln --text -d ".;" -m 0
--input ./test.log -of HTML
--input ./test.log -of HTML -o /Volumes/Phoenix_test/test_log.html
--input ./test/testMultiline.log -m 0
