# logviewer

#### - Log files viewer.

Features:

- Text mode log viewer, runs everywhere after recompilation.

- Dynamic log level threshold: in the proximity of logs with high level,
  automatically lower the level threshold to provide context for critical cases.

- Log level based highlighting (on Linux, OS-X, Windows).
	- Levels can be numeric (1-7) or strings.

- Log file format agnostic.
	- Log level tag position automatically found in the logs.

- Filtering capability.

- Free software, GPL 3 license.


For better performance, consider logging to a ramdisk.


Requirements:
- C++11
- Default build system: CMake
