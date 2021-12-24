#include"MathUtility.h"


long Math_Utility::combinations(int n, int k)
{
     if (k > n)
         return 0;
     int r = 1;
     for (int d = 1; d <= k; ++d) {
         r *= n--;
         r /= d;
     }
     return r;
} 

long Math_Utility::factorial(int n)
{
  return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
}

void go(int offset, int k, std::vector<std::vector<int> > & combinations, std::vector<int> & people, std::vector<int> & combination) {
  if (k == 0) {
	combinations.push_back(combination);
    return;
  }
  for (int i = offset; i <= people.size() - k; ++i) {
    combination.push_back(people[i]);
    go(i+1, k-1, combinations, people, combination);
    combination.pop_back();
  }
}

void Math_Utility::combinations(int n, int k, std::vector<std::vector<int> > & combinations){
	std::vector<int> people;
	std::vector<int> combination;

	for (int i = 0; i < n; ++i) { people.push_back(i); }
	  go(0, k, combinations, people, combination);

}



