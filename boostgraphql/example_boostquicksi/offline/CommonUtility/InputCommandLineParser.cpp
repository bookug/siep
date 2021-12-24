
#include"InputCommandLineParser.h"

char* InputCommandLineParser::getCmdOption(int argc, char * argv[], const std::string & option)
{
    char ** itr = std::find(argv, argv + argc, option);
    if (itr != argv + argc && ++itr != argv + argc){
        return *itr;
    }
    return 0;
}

bool InputCommandLineParser::cmdOptionExists(int argc, char * argv[], const std::string & option){
    return std::find(argv, argv + argc, option) != argv + argc;
}
