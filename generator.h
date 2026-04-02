#pragma once
#include <string>

std::string evar(int i, int j);

std::string tvar(int i, int j, int k);

std::string degv(int i);

std::string genRegularityCondition(const int n, const int r);

std::string genTrianglesK3Degs(const int n, const int r);

std::string genBinaryVars(const int n, const int r);

void generateUsual(const int N, const int R, const int min_k3_deg, const int max_k3_deg, const std::string& filename);

// for n=22
void generateSplitAB(const int n, const int r, const int min_k3_deg, const int max_k3_deg, const int splitK3, const std::string& filename, const int neighboursOfZeroInB = -1);