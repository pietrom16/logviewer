/******************************************************************************
 *  progArgs.h
 *
 *  Read the parameters passed to main() and store them in an object
 *  as a set of strings.
 *  For an example see the "progArgs_test.cpp" file.
 ******************************************************************************/

/* TODO
	- Help: print parameters.
 */


#include <string>
#include <vector>

namespace Utilities {
	
	
	
	class  ProgArgs
	{
	public:
		
		struct  Argument
		{
			std::string  name, shortName, desc, value, defaultValue;
			bool  optional;			// =true if the argument is optional
			bool  valueNeeded;		// =true if the argument needs a value, too
			bool  defaultAvailable;	// =true if a default value is available
			bool  present;			// =true if the argument is specified
			
			static const int  desc_dist_max = 32;
			
			Argument(std::string name_="", std::string shortName_="", std::string desc_="",
					 bool optional_=true, bool valueNeeded_=false, std::string defaultValue_="");
			int  Set(std::string name_, std::string shortName_, std::string desc_="", 
					 bool optional_=true, bool valueNeeded_=false, std::string defaultValue_="");
			void Set(const std::string &value_);
			void Reset(void);
			void Print(void) const;
			void Help(void) const;
		};
		
		ProgArgs(void);
		int  AddArg(const Argument&);
		int  Parse(int argc, char *argv[]);
		int  GetArg(int i, Argument&) const;
		int  GetValue(const std::string &name, std::string &value) const;
		bool GetValue(const std::string &name) const;
		void Print(void) const;
		void Help(void) const;
		
	private:
		std::string  programName;
		std::vector<Argument>  args;
		//std::map<std::string, std::string>  tagValue;
	};
	
	
	
}  // Utilities


