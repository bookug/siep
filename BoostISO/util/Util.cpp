/*=============================================================================
# Filename: Util.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2016-10-24 17:23
# Description: 
=============================================================================*/

#include "Util.h"

using namespace std;

Util::Util()
{
}

Util::~Util()
{
}

long
Util::get_cur_time()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

string my_extract_ints(ctype_base::mask category, string str, ctype<char> const& facet)
{
	//using strlen;

	char const *begin = &str.front(),
		*end   = &str.back();

	auto res = facet.scan_is(category, begin, end);

	begin = &res[0];
	end   = &res[strlen(res)];

	return string(begin, end);
}

string my_extract_ints(string str)
{
	return my_extract_ints(ctype_base::digit, str,
		use_facet<ctype<char> >(locale("")));
}


void myTimeout(int signo)
{
    switch(signo)
    {
        case SIGALRM:
            printf("This query runs time out!\n");
            exit(1);
        default:
            break;
    }
}

void 
Util::timeLimit(int seconds)
{
    struct itimerval tick;
    //signal(SIGALRM, exit);
    signal(SIGALRM, myTimeout);
    memset(&tick, 0, sizeof(tick));
    //Timeout to run first time
    tick.it_value.tv_sec = seconds;
    tick.it_value.tv_usec = 0;
    //After first, the Interval time for clock
    tick.it_interval.tv_sec = seconds;
    tick.it_interval.tv_usec = 0;
    if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
        printf("Set timer failed!\n");
}

void 
Util::noTimeLimit()
{
    struct itimerval tick;
    memset(&tick, 0, sizeof(tick));
    if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
        printf("Withdraw timer failed!\n");
}

//NOTICE: we had better watch the virtual memory usage, i.e., vsize.
//https://blog.csdn.net/zyboy2000/article/details/50456764
//The unit of return values are both KB.
void 
Util::process_mem_usage(double& vm_usage, double& resident_set)
{
   using std::ios_base;
   using std::ifstream;
   using std::string;

   vm_usage     = 0.0;
   resident_set = 0.0;

   // 'file' stat seems to give the most reliable results
   ifstream stat_stream("/proc/self/stat",ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   unsigned long vsize;  // for virtual memory, the virtual size (B)
   long rss;    //for physical memory, number of real pages
   //In /proc/$pid/status, these corresponding to VmSize and VmRSS

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
   vm_usage     = vsize / 1024.0;
   resident_set = rss * page_size_kb;
}

//DEBUG: below will cause burst of memory cost.
//void Util::readIntegersFromString(string stringContents, vector<int>& numbers)
//{
	//int integerNumber;
	//numbers.clear();
	//stringstream ss(my_extract_ints(stringContents));
	//while(ss>>integerNumber){
		//numbers.push_back(integerNumber);
	//}
//}

void Util::readIntegersFromString(string stringContents, vector<int>& numbers)
{
	int integerNumber;
	double tmp_mem, tmp_rss;
	numbers.clear();
	/*
	Util::process_mem_usage(tmp_mem, tmp_rss);
	printf("Mem before ss: %8.0lf kB\n", tmp_mem);
	
	stringstream ss(my_extract_ints(stringContents));
	
	Util::process_mem_usage(tmp_mem, tmp_rss);
	printf("Mem before after ss: %8.0lf kB\n", tmp_mem);
	
	while(ss>>integerNumber){
		numbers.push_back(integerNumber);
	}
	*/
	char d[] = " ";
    int len = stringContents.length();
	char *str = new char[len+1];
	memcpy(str, stringContents.c_str(), sizeof(char)*stringContents.length());
    str[len] = 0;

	char *s = str;
    //WARN: s will be changed in strtok()
	char *p = strtok(s, d);
	while (p){
		int n = -1;
		if (!(*p>='0' && *p<='9') && !(*p>='a' && *p<='z')) break;
		sscanf(p, "%d", &n);
		if (n>=0 && n!=32767)
			numbers.push_back(n);
		p = strtok(NULL, d);
	}
	delete []str;
}


