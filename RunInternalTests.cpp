/// RunInternalTests.cpp

/**
	Launch the set of tests specified in the configuration.
 */

#include <iostream>


int RunInternalTests()
{
	int status = 0;

#ifdef LOGCONTEXT_TEST
	status += LogContext_test();
#endif

#ifdef READ_KEYBOARD_TEST
	status += ReadKeyboard_test();
#endif

	std::cout << "Internal tests result: " << status << std::endl;

	return status;
}
