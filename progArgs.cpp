///  progArgs.cpp

#include "progArgs.h"
#include <cstdlib>
#include <iostream>

using namespace std;

namespace Utilities {


bool IsInteger(const std::string &str)
{
	if(str.empty())
		return false;

	string::size_type pos = 0;

	if(str[0] == '-')
		pos = 1;

	if(str.find_first_not_of("0123456789", pos) == string::npos)
		return true;

	return false;
}


bool IsNumber(const std::string &str)
{
	if(str.empty())
		return false;

	string::size_type pos = 0, dotPos = 0;

	// Check decimal point
	if((dotPos = str.find('.')) != string::npos)
		if((dotPos = str.find('.', dotPos)) != string::npos)
			return false;

	pos = 0;

	if(str[0] == '-')
		pos = 1;

	if(str.find_first_not_of(".0123456789", pos) == string::npos)
		return true;

	return false;
}


ProgArgs::Argument::Argument(
		std::string _tag, 
		std::string _shortTag,
		std::string _desc,
		bool _optional, 
		bool _needed, 
		std::string _default)
{
	Set(_tag, _shortTag, _desc, _optional, _needed, _default);
}


int ProgArgs::Argument::Set(
		std::string _tag, 
		std::string _shortTag, 
		std::string _desc, 
		bool _optional, 
		bool _needed, 
		std::string _default)
{
	tag = _tag;
	shortTag = _shortTag;
	desc = _desc;
	optional = _optional;
	val = _default;
	defaultValue = _default;

	if (_default.empty())
		defaultAvailable = false;
	else
		defaultAvailable = true;

	valueNeeded = _needed;
	present = false;
	return  0;
}


void ProgArgs::Argument::Set(const std::string &_val)
{
	val = _val;
	present = true;
}


void ProgArgs::Argument::Reset()
{
	val = defaultValue;
}


void ProgArgs::Argument::Print() const
{
	cout << "Name:           " << tag << "\n"
		 << "Short name:     " << shortTag << "\n"
		 << "Description:    " << desc << "\n";

	if(valueNeeded)
	{
		cout << "Value:          " << val << "\n"
			 << "Default value:  ";

		if (defaultAvailable)
			cout << defaultValue;

		cout << "\n";
	}

	cout << "Notes:          ";

	if(!valueNeeded)
		cout << "No value;  ";

	if(optional)
		cout << "Optional parameter;  ";
	else
		cout << "Necessary parameter;  ";

	if(present)
		cout << "Specified value.";
	else
		cout << "Unspecified value.";

	cout << "\n" << endl;
}


void ProgArgs::Argument::Help() const
{
	// Alignment of the description field:
	int     desc_dist = desc_dist_max - (2 + shortTag.size() + 2 + tag.size() + 3);
	string  tabulation;

	if (desc_dist > 0)
	{
		tabulation.append (desc_dist, ' ');
	}
	else
	{
		tabulation.append ("\n\r");
		tabulation.append (desc_dist_max, ' ');
	}

	cout << "  " << shortTag << ", " << tag << tabulation << desc;

	if (defaultAvailable)
		cout << " (default = " << defaultValue << ")";

	cout << "." << endl;
}


ProgArgs::ProgArgs()
{
	//...
}


int ProgArgs::AddArg(const Argument& _arg)
{
	args.push_back(_arg);
	return 0;
}


// Parse: get the _argv strings and copy their parameters into args[]

int ProgArgs::Parse(int _argc, char *_argv[])
{
	programName = _argv[0];

	size_t argc = size_t(_argc);

	size_t i = 1;
	while(i < argc)
	{
		for(size_t j = 0; j < args.size(); ++j)
		{
			if(args[j].tag == _argv[i] || args[j].shortTag == _argv[i])
			{
				if(args[j].present == false)
				{
					// First time this tag has been found

					args[j].present = true;

					if(args[j].valueNeeded) {
						++i;
						if(i >= argc) {
							cerr << programName << " error:  the argument " << args[j].tag <<
									" (" << args[j].shortTag << ") needs a value." << endl;
							exit(1);
						}
						args[j].val = _argv[i];
					}
				}
				else
				{
					// This tag has been found multiple times

					// Copy the j-th arg at the end of args
					args.push_back(args[j]);

					if(args[j].valueNeeded) {
						++i;
						if(i >= argc) {
							cerr << programName << " error:  the argument " << args[j].tag <<
									" (" << args[j].shortTag << ") needs a value." << endl;
							exit(1);
						}
						args.back().val = _argv[i];
					}
				}

				break;
			}
		}

		++i;
	}

	// Rescan the arguments, counting how many of them are unknown

	/* Warning: a value which is synctactically equal to an argument,
	 * must be enclosed in double quotes.
	 */

	int  nUnknownArgs = 0;

	//+TODO - Fix this code; it partially works
#if 0
	bool unknown = true;

	i = 1;
	while(i < argc)
	{
		unknown = true;

		cerr << "  _argv[" << i << "][0] = " << _argv[i][0]; //+T+++

		if(IsNumber(_argv[i]))
		{
			unknown = false;
		}
		else if(_argv[i][0] == '\'' || _argv[i][0] == '\"')
		{
			unknown = false;
		}
		else
		{
			for(size_t j = 0; j < args.size(); ++j)
			{
				if(args[j].tag == _argv[i] || args[j].shortTag == _argv[i])
				{
					unknown = false;
					break;
				}
			}
		}

		if(unknown)
			++nUnknownArgs;

		++i;
	}
#endif

	return nUnknownArgs;
}


int ProgArgs::GetArg(int _i, Argument &_arg) const
{
	const size_t i = size_t(_i);

	if(_i < 0 || i >= args.size())
		return -1;

	_arg = args[i];

	return 0;
}


int ProgArgs::GetValue(const string &_tag, string &_val) const
{
	for(size_t i = 0; i < args.size(); ++i)
	{
		if(_tag == args[i].tag)
		{
			_val = args[i].val;
			return int(i);
		}
	}

	return -1;
}


// Get the n-th value, in case of multiple equal tags
//	n is zero based.

int ProgArgs::GetValue(const std::string& _tag, std::string& _val, int _n) const
{
	int id = 0;

	for(size_t i = 0; i < args.size(); ++i)
	{
		if(_tag == args[i].tag)
		{
			if(id == _n)
			{
				_val = args[i].val;
				return(id + 1);
			}

			++id;
		}
	}

	return -1;
}


bool ProgArgs::GetValue(const string &_tag) const
{
	for(size_t i = 0; i < args.size(); ++i)
	{
		if(_tag == args[i].tag)  {
			if(args[i].present)
				return true;
			else
				return false;
		}
	}

	return false;
}


void ProgArgs::Print() const
{
	cout << endl;

	for(size_t j = 0; j < args.size(); ++j)
	{
		args[j].Print();
	}
}


void ProgArgs::Help() const
{
	cout << endl;

	for(size_t j = 0; j < args.size(); ++j)
	{
		args[j].Help();
	}
}


}  // Utilities

