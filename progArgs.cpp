//  progArgs.cpp

#include "progArgs.h"
#include <cstdlib>
#include <iostream>

using namespace std;

namespace Utilities {

	
	ProgArgs::Argument::Argument(string name_, string shortName_, string desc_, bool optional_, bool valueNeeded_, string defaultValue_)
	{
		Set(name_, shortName_, desc_, optional_, valueNeeded_, defaultValue_);
	}
	
	
	int ProgArgs::Argument::Set(string name_, string shortName_, string desc_, bool optional_, bool valueNeeded_, string defaultValue_)
	{
		name = name_;
		shortName = shortName_;
		desc = desc_;
		value = defaultValue_;
		defaultValue = defaultValue_;
		
		if (defaultValue_.empty())
			defaultAvailable = false;
		else
			defaultAvailable = true;
		
		valueNeeded = valueNeeded_;
		present = false;
		return  0;
	}
	
	
	
	void ProgArgs::Argument::Set(const std::string &value_)
	{
		value = value_;
		present = true;
	}
	
	
	
	
	void ProgArgs::Argument::Reset(void)
	{
		value = defaultValue;
	}
	
	
	
	
	void ProgArgs::Argument::Print(void) const
	{
		cout << "Name:           " << name << "\n"
			 << "Short name:     " << shortName << "\n"
			 << "Description:    " << desc << "\n";
		
		if(valueNeeded)
		{
			cout << "Value:          " << value << "\n"
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
	
	
	
	void ProgArgs::Argument::Help(void) const
	{
		// Alignment of the description field:
		int     desc_dist = desc_dist_max - (2 + shortName.size() + 2 + name.size() + 3);
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
		
		cout << "  " << shortName << ", " << name << tabulation << desc;
		
		if (defaultAvailable)
			cout << " (default = " << defaultValue << ")";
		
		cout << "." << endl;
	}
	
	
	
	ProgArgs::ProgArgs (void)
	{
		//...
	}
	
	
	
	int ProgArgs::AddArg(const Argument& arg)
	{
		args.push_back (arg);
		return  0;
	}
	
	
	
	// Parse: get the argv strings and copy their parameters into args[]
	
	int ProgArgs::Parse(int argc, char *argv[])
	{
		programName = argv[0];
		
		int i = 1;
		while (i < argc)
		{
			for (int j = 0; j < args.size(); ++j)
			{
				if (args[j].name == argv[i] || args[j].shortName == argv[i])
				{
					args[j].present = true;
					
					if (args[j].valueNeeded)  {
						++i;
						if (i >= argc)  {
							cerr << programName << " error:  the argument " << args[j].name <<
									" (" << args[j].shortName << ") needs a value." << endl;
							exit(1);
						}
						args[j].value = argv[i];
					}
					
					break;
				}
			}
			
			++i;
		}
		
		return  0;
	}
	
	
	
	int ProgArgs::GetArg(int i, Argument &arg) const
	{
		if (i < 0 || i >= args.size())
			return -1;
		
		arg = args[i];
		
		return 0;
	}
	
	
	
	int ProgArgs::GetValue(const string &name, string &value) const
	{
		for (int i = 0; i < args.size(); ++i)
		{
			if (name == args[i].name)  {
				value = args[i].value;
				return  i;
			}
		}
		
		return  -1;
	}
	
	
	
	bool ProgArgs::GetValue(const string &name) const
	{
		for (int i = 0; i < args.size(); ++i)
		{
			if (name == args[i].name)  {
				if (args[i].present)
					return  true;
				else
					return  false;
			}
		}
		
		return  false;
	}
	
	
	
	void ProgArgs::Print(void) const
	{
		cout << endl;
		
		for (int j = 0; j < args.size(); ++j)
		{
			args[j].Print();
		}
	}
	
	
	
	void ProgArgs::Help(void) const
	{
		cout << endl;
		
		for (int j = 0; j < args.size(); ++j)
		{
			args[j].Help();
		}
	}
	
	
}  // Utilities

