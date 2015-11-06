/// ReadKeyboard.cpp

#if defined(__unix__) || defined(__linux__) || \
	defined(BSD) || (defined (__APPLE__) && defined (__MACH__)) || defined(__bsdi__) || \
	defined(__minix) || defined(__CYGWIN__) || defined(__FreeBSD__)
#define POSIX 1
#endif

#ifdef POSIX

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

#include "ReadKeyboard.h"

namespace utilities {

	static struct termios oldt, newt;


	int ReadKeyboard::Get()
	{
		key = 0;
		struct timeval tv;
		fd_set read_fd;
		
		// Waiting time
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		
		// Initialize read_fd
		FD_ZERO(&read_fd);
		
		// Make select() ask if input is ready
		FD_SET(STDIN_FILENO, &read_fd);
		
		int r = select(STDIN_FILENO + 1, &read_fd, NULL /*No writes*/, NULL /*No exceptions*/, &tv);
		
		if(r < 0)
			return r;  // An error occured
		
		/* read_fd now holds a bit map of files that are
		 * readable. We test the entry for stdin */
		if(FD_ISSET(STDIN_FILENO, &read_fd))
			key = getchar();
		
		// If no key has been pressed, key = 0
		
		return key;
	}
	
	
	int ReadKeyboard::Getch()
	{
		struct termios oldt, newt;
		int ch;
		tcgetattr( STDIN_FILENO, &oldt );
		newt = oldt;
		newt.c_lflag &= ~( ICANON | ECHO );
		tcsetattr( STDIN_FILENO, TCSANOW, &newt );
		ch = getchar();
		tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
		return ch;
	}


	void ReadKeyboard::NonBlocking()
	{
		tcgetattr(STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
		blocking = true;
	}
	
	
	void ReadKeyboard::Blocking()
	{
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
		blocking = false;
	}
	

} // utilities

#elif defined _WIN32

#include <conio.h>
#include "ReadKeyboard.h"

namespace utilities {

	int ReadKeyboard::Get()
	{
		if(blocking || _kbhit())
		{
			return _getch();
		}
		
		return 0;	// no key pressed
	}
	
	
	int ReadKeyboard::Getch()
	{
		return _getch();
	}
	
	
	void ReadKeyboard::NonBlocking()
	{
		blocking = true;
	}
	
	
	void ReadKeyboard::Blocking()
	{
		blocking = false;
	}
	

} // utilities

#endif // _WIN32


