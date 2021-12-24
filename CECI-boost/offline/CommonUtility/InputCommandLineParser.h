#ifndef INPUT_COMMAND_PARSER
#define INPUT_COMMAND_PARSER

#include "../../util/Util.h"

class InputCommandLineParser{

public:

	/* parse the input command line */
	static char* getCmdOption(int argc, char * argv[], const std::string & option);
	static bool cmdOptionExists(int argc, char * argv[], const std::string & option);

};
#endif
