

#ifndef TIME_UTILITY
#define TIME_UTILITY

#include "../../util/Util.h"

class TimeUtility{
public:
	static void StartCounterMicro();
	static double GetCounterMicro();

	static void StartCounterMill();
	static double GetCounterMill();
};

#endif
