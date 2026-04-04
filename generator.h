#pragma once
#include <string>
#include <stdexcept>
#include <iostream>

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

	void validate() const
	{
		if ( n <= 0 || r <= 0 || r >= n )
			throw std::invalid_argument("Invalid graph dimentions (n, r).");
		if (min_k3 > max_k3 || min_k3 < 0 || max_k3 < 0)
			throw std::invalid_argument("Invalid k3 bounds.");
		if (use_split_AB && (splitK3 < 0 || splitK3 < min_k3 || splitK3 > max_k3))
			throw std::invalid_argument("Invalid splitK3 value.");
		if (!use_split_AB && fixVertexInBMode != FixVertexInBMode::NONE)
			throw std::invalid_argument("fixVertexInBMode can only be used with split_AB.");
		if (fixVertexInBMode != FixVertexInBMode::NONE) {
			if (neighbours_of_fixed_vertex_in_B < 0 || neighbours_of_fixed_vertex_in_B >= r)
			{
				throw std::invalid_argument("Invalid neighbours_of_fixed_vertex_in_B value.");
			}
			switch (fixVertexInBMode)
			{
			case FixVertexInBMode::NONE:
				break;
			case FixVertexInBMode::ZERO_IN_B:
				if(min_k3 > 0 || splitK3 == 0)
					throw std::invalid_argument("Invalid config: can't fix zero in B if min_k3 > 0 or splitK3 == 0.");
				break;
			case FixVertexInBMode::ONE_IN_B:
				if (min_k3 > 1 || splitK3 == 1)
					throw std::invalid_argument("Invalid config: can't fix one in B if min_k3 > 1 or splitK3 == 1.");
				break;
			default:
				break;
			}
		}
		std::cout << "GraphConfig validated successfully.\n";
	}
};

std::string getFileName(const GraphConfig& cfg);

void generateGraphLP(const GraphConfig& cfg, const std::string& filename);