# TODO - logviewer

-- Multiline logs
	- Automatic recognition of multiline logs
		- If the next log line has no timestamp and log level, treat it as a continuation of the previous log.
		- No need to specify a command line parameter to recognize multiline logs.

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

- Split logviewer.cpp in multiple shorter files.
    . Separated HTML member functions.

- Check command line user interaction is complete enough.

- Add option to delete existing output file before adding new logs.

- Use set of conditions to pick logs even if below the threshold when other logs are met (i.e. extend the context feature).

-- Show multiple log files.

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

- Check if it is normal that destructors are not called before exit.
- Check timing in text view.


## TESTING

--input "/Users/pietro/myProgs/logger/logviewer/test/testText.txt" --logLevels "/Users/pietro/myProgs/logger/logviewer/test/testText_logLevels.txt" -ln --text -d ".;" -m 0
--input ./test.log -of HTML
--input ./test.log -of HTML -o /Volumes/Phoenix_test/test_log.html
