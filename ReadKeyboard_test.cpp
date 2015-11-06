/// ReadKeyboard_test.cpp

/**
	Test of the utilities::ReadKeyboard class.
 */

#ifdef READ_KEYBOARD_TEST_H

#include <iostream>
#include <vector>
#include <ctime>
#include "ReadKeyboard.h"


int main()
{
	using namespace std;
	using namespace utilities;
	
	int key = 0;
	int nKeyStrokes = 0;
	
	struct timespec sleepTime;
	struct timespec returnTime;
	sleepTime.tv_sec = 0;
	sleepTime.tv_nsec = 1000000;
	
	cout << "ReadKeyboard test, nonblocking mode: press 'q' to exit... " << flush;
	
	{	// In this block, read the keyboard in a non blocking way
	
		ReadKeyboard rdKb;
	
		while(key != 'q')
		{
			key = rdKb.Get();
			if(key) ++nKeyStrokes;
			cout << char(key) << flush;
			
			// Meaninful block of code:
			nanosleep(&sleepTime, &returnTime);
			cout << "." << flush;
		}
	
		cout << "\n\nDone. " << nKeyStrokes << " keys pressed.\n\n";
	}
	
	cout << "ReadKeyboard test, blocking mode: press any key... " << flush;
	
	{	// In this block, read the keyboard in blocking mode  //+B this does not work!
	
		ReadKeyboard rdKb;
		
		rdKb.Blocking();
	
		key = rdKb.Get();
		cout << char(key) << endl;
	}
	
	cout << "ReadKeyboard test, blocking mode 2: press any key... " << flush;
	
	{	// In this block, read the keyboard in blocking mode
	
		ReadKeyboard rdKb;
		
		key = rdKb.Getch();
		cout << char(key) << endl;
	}
	
	return 0;
}

#endif // READ_KEYBOARD_TEST_H
