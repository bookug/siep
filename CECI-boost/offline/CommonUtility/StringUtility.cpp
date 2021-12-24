#include"StringUtility.h"


using namespace std;


std::string extract_ints(std::ctype_base::mask category, std::string str, std::ctype<char> const& facet)
{
	using std::strlen;

	char const *begin = &str.front(),
		*end   = &str.back();

	auto res = facet.scan_is(category, begin, end);

	begin = &res[0];
	end   = &res[strlen(res)];

	return std::string(begin, end);
}

std::string extract_ints(std::string str)
{
	return extract_ints(std::ctype_base::digit, str,
		std::use_facet<std::ctype<char> >(std::locale("")));
}


void String_Utility::readIntegersFromString(string stringContents, std::vector<int>& numbers){
	int integerNumber;
	numbers.clear();
	std::stringstream ss(extract_ints(stringContents));
	while(ss>>integerNumber){
		numbers.push_back(integerNumber);
	}
}
