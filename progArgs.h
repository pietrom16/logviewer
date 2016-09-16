/******************************************************************************
 *  progArgs.h
 *
 *  Read the parameters passed to main() and store them in an object
 *  as a set of strings.
 * 
 *  For an example see the "progArgs_test.cpp" file.
 * 
 *  Allows to have multiple parameters with the same tag:
 *		executable -par1 123 -par 234 -par 345
 * 
 ******************************************************************************/


#include <string>
#include <vector>

namespace Utilities {


extern bool IsInteger(const std::string &str);
extern bool IsNumber(const std::string &str);


class  ProgArgs
{
public:

	struct  Argument
	{
		std::string  tag, shortTag, desc, val, defaultValue;
		bool  optional;			// =true if the argument is optional
		bool  valueNeeded;		// =true if the argument needs a value, too
		bool  defaultAvailable;	// =true if a default value is available
		bool  present;			// =true if the argument is specified

		static const int  desc_dist_max = 32;

		Argument(std::string _tag = "", std::string _shortTag = "", std::string _desc = "",
				 bool _optional = true, bool _needed = false, std::string _default = "");

		int  Set(std::string _tag, std::string _shortTag, std::string _desc = "", 
				 bool _optional = true, bool _needed = false, std::string _default = "");

		void Set(const std::string &_val);

		void Reset();

		void Print() const;
		void Help() const;
	};

	ProgArgs();
	
	int  AddArg(const Argument&);
	
	int  Parse(int _argc, char *_argv[]);
	
	int  GetArg(int _i, Argument&) const;
	
	int  GetValue(const std::string &_tag, std::string &_val) const;
	
	// Form multiple values associated to the same name
	//	n is zero based.
	int  GetValue(const std::string &_tag, std::string &_val, int _n) const;
	
	bool GetValue(const std::string &_tag) const;

	void Print() const;
	void Help() const;

private:
	std::string  programName;
	std::vector<Argument>  args;
};


}  // Utilities
