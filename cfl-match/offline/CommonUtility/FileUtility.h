/**
* This file wraps the utility functions for string operations
*/
#ifndef FILE_UTILITY
#define FILE_UTILITY

#include "../../util/Util.h"

class FileUtility{
public:
		static std::ifstream::pos_type filesize(const char* filename);
};

#endif
