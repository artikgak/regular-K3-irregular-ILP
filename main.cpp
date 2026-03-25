#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>

#include "generator.h"
#include "graphhelper.h"

using namespace std;

constexpr int N = 24;
constexpr int R = 9;
constexpr int MIN_K3_DEG = 3;
constexpr int MAX_K3_DEG = 26;
constexpr int SPLIT_K3 = 26;

const string FILENAME = "N" + to_string(N) + "_R" + to_string(R) + ".lp";
const string FILENAMESpec = "N" + to_string(N) + "_R" + to_string(R) + "_K3_" + to_string(MIN_K3_DEG) + "_" + to_string(MAX_K3_DEG) + "_split_" + to_string(SPLIT_K3) + ".lp";

int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	//string expression = "[[1,2,3,4,5,6,7,11,12],[0,2,3,4,5,6,7,9,10],[0,1,3,4,5,6,7,13,23],[0,1,2,4,5,7,8,19,23],[0,1,2,3,5,6,7,8,14],[0,1,2,3,4,6,7,9,23],[0,1,2,4,5,7,8,9,23],[0,1,2,3,4,5,6,10,23],[3,4,6,11,12,16,20,21,22],[1,5,6,11,12,13,20,21,22],[1,7,11,12,14,16,20,21,22],[0,8,9,10,14,15,16,17,18],[0,8,9,10,15,17,18,19,23],[2,9,14,15,16,18,20,21,22],[4,10,11,13,15,17,19,21,22],[11,12,13,14,16,18,19,20,21],[8,10,11,13,15,17,18,19,23],[11,12,14,16,18,19,20,21,22],[11,12,13,15,16,17,20,21,22],[3,12,14,15,16,17,20,21,22],[8,9,10,13,15,17,18,19,23],[8,9,10,13,14,15,17,18,19],[8,9,10,13,14,17,18,19,23],[2,3,5,6,7,12,16,20,22]]";
	//UndirectedGraph graph9 = fromVecToGraph<UndirectedGraph>(parseExpression(expression));
	//string presolveStr = generatePresoveSplitted(graph9, 26);
	//ofstream f("presolve9r_split26.mst");
	//f << presolveStr;
	//f.flush();
	//f.close();

	UndirectedGraph graph9 = loadGraphFromSCIPSolution("testsolSplit3.txt", 24);
	std::vector<int> k3degs = K3Irregullar(graph9);

	for (int i = 0; i < k3degs.size(); ++i)
	{
		cout << k3degs[i] << ' ';
	}
	cout << "\nSorted:\n";
	std::sort(k3degs.begin(), k3degs.end());
	for (int i = 0; i < k3degs.size(); ++i)
	{
		cout << k3degs[i] << ' ';
	}

	//generateUsual(N, R, MIN_K3_DEG, MAX_K3_DEG, FILENAME);
	//generateSplitAB(N, R, MIN_K3_DEG, MAX_K3_DEG, SPLIT_K3, FILENAMESpec);

	return 0;
}