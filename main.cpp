#include <iostream>
#include <fstream>
#include <vector>

#include "generator.h"

using namespace std;

constexpr int N = 22;
constexpr int R = 8;
constexpr int MIN_K3_DEG = 0;
constexpr int MAX_K3_DEG = 22;

const string FILENAME = "reg_irreg_N" + to_string(N) + "_R" + to_string(R) + ".lp";
const string FILENAMESpec = "reg_irreg_N" + to_string(N) + "_R_spec" + to_string(R) + ".lp";

int main()
{
	//generateUsual(N, R, MIN_K3_DEG, MAX_K3_DEG, FILENAME);
	generateSpecialBounds(N, R, MIN_K3_DEG, MAX_K3_DEG, FILENAMESpec);

	return 0;
}