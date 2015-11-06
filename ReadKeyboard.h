/// ReadKeyboard.h

/** class ReadKeyboard
 *
 *	Non-blocking text-mode keyboard input.
 *	Allows to check if a key has been pressed without 
 *	stopping the execution of the current thread.
 *
 *	The ctor sets the input mode to non blocking, the dtor 
 *	sets it back to blocking.
 */

/* Copyright 2011-2012 Pietro Mele
 */

#ifndef READ_KEYBOARD_H
#define READ_KEYBOARD_H

namespace utilities {


	class ReadKeyboard
	{
		int  key;
		bool blocking;
		
	public:
		ReadKeyboard() : key(0) {
			NonBlocking();
		}

		~ReadKeyboard() {
			Blocking();
		}
		
		int Check() const { return key; }
		
		int Get();		// return the currently pressed key, 0 otherwise
		int Getch();	// blocking mode
		
		void NonBlocking();
		void Blocking();
		bool Status() const { return blocking; }
	};


} // utilities

#endif // READ_KEYBOARD_H


//TODO
// - Check if it works with multiple text windows used by the same application.
