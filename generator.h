#pragma once
#include <string>

enum class FixVertexInBMode
{
	NONE,
	ZERO_IN_B,
	ONE_IN_B
};

struct GraphConfig
{
	int n;
	int r;

	int min_k3;
	int max_k3;

	bool use_split_AB = false;
	int splitK3 = -1; // only used if use_split_AB

	FixVertexInBMode fixVertexInBMode = FixVertexInBMode::NONE;
	int neighbours_of_fixed_vertex_in_B = - 1; // only used if fixVertexInBMode != NONE
};

std::string getFileName(const GraphConfig& cfg);

void generateGraphLP(const GraphConfig& cfg, const std::string& filename);