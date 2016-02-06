# logviewer

#### - Log files viewer & text highlighter.

Features:

- Text mode log viewer, runs everywhere after recompilation.

- Dynamic log level threshold: in the proximity of logs with high level,
  automatically lower the level threshold to provide context for critical cases.

- Log level based highlighting (on Linux, OS-X, Windows).
	- Levels can be numeric (1-7) or strings.
	- Custom level tags can be specified on a file.

- Log file format agnostic.
	- Log level tag position automatically found in the logs.

- Filtering capability.

- Text highlighter: specifying custom keywords with a priority level, highlights text files,
  shows context, and hides non relevant parts.

- Free software, GPL 3 license.


For better performance, consider logging to a ramdisk.


Requirements:
- C++11
- Default build system: CMake
