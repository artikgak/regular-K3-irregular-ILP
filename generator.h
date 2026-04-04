#pragma once
#include <string>
#include <stdexcept>
#include <iostream>
#include <vector>

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
	int anchorK3 = -1; // only used if use_split_AB

	FixVertexInBMode fixVertexInBMode = FixVertexInBMode::NONE;
	int neighbours_of_fixed_vertex_in_B = - 1; // only used if fixVertexInBMode != NONE

	void validate() const
	{
		if ( n <= 0 || r <= 0 || r >= n )
			throw std::invalid_argument("Invalid graph dimentions (n, r).");
		if (min_k3 > max_k3 || min_k3 < 0 || max_k3 < 0)
			throw std::invalid_argument("Invalid k3 bounds.");
		if (use_split_AB && (anchorK3 < 0 || anchorK3 < min_k3 || anchorK3 > max_k3))
			throw std::invalid_argument("Invalid anchorK3 value.");
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
				if(min_k3 > 0 || anchorK3 == 0)
					throw std::invalid_argument("Invalid config: can't fix zero in B if min_k3 > 0 or anchorK3 == 0.");
				break;
			case FixVertexInBMode::ONE_IN_B:
				if (min_k3 > 1 || anchorK3 == 1)
					throw std::invalid_argument("Invalid config: can't fix one in B if min_k3 > 1 or anchorK3 == 1.");
				break;
			default:
				break;
			}
		}
		std::cout << "GraphConfig validated successfully.\n";
	}
};

// indexes in LP file
// 0 anchor vertex (if split_AB)
// 1..r vertices in A
// r+1..n-1 vertices in B
// Note: if fixVertexInBMode != NONE, then vertex r+1 is fixed in B 
// and has neighbours_of_fixed_vertex_in_B neighbours (r+2 .. r+1+neighbours_of_fixed_vertex_in_B) in B. 
// So it can't be in A and can't be the anchor vertex.
struct VertexSets {
	const GraphConfig& cfg;
	std::vector<int> verticesInA;
	std::vector<int> verticesInB;
	std::vector<int> neighOfFixedInB;
	std::vector<int> otherInB;

	VertexSets(const GraphConfig& cfg) : cfg(cfg) 
	{ 
		cfg.validate(); 

		if (cfg.use_split_AB)
		{
			// note 0 is anchor
			for (int i = 1; i <= cfg.r; i++)
				verticesInA.push_back(i);
			for (int i = cfg.r + 1; i < cfg.n; i++)
				verticesInB.push_back(i);

			if(cfg.fixVertexInBMode != FixVertexInBMode::NONE )
			{
				int fixedVertex = fixedVertexInB();
				neighOfFixedInB.reserve(cfg.neighbours_of_fixed_vertex_in_B);
				for (int i = fixedVertex + 1; i <= fixedVertex + cfg.neighbours_of_fixed_vertex_in_B; ++i)
				{
					neighOfFixedInB.push_back(i);
				}

				for (int i = fixedVertex + cfg.neighbours_of_fixed_vertex_in_B + 1; i < cfg.n; ++i)
				{
					otherInB.push_back(i);
				}
			}
		}
	}

	int anchorIndex() const { return cfg.use_split_AB ? 0 : throw "There is no anchor"; }
	bool isVertexAnchorIndex(int v) const { return cfg.use_split_AB && v == 0; }

	int anchorK3() const { return cfg.use_split_AB ? cfg.anchorK3 : throw "There is no anchor"; }
	bool isVertexAnchorK3(int v_k3) const { return cfg.use_split_AB && v_k3 == cfg.anchorK3; }

	bool isVertexInA(int v) const { return v >= 1 && v <= cfg.r; }
	bool isVertexInB(int v) const { return v >= cfg.r + 1 && v < cfg.n; }
	bool isVertexInFixedB(int v) const { return cfg.fixVertexInBMode != FixVertexInBMode::NONE && v == cfg.r + 1; }
	bool isNeighbourOfFixedB(int v) const { return cfg.fixVertexInBMode != FixVertexInBMode::NONE && v > cfg.r + 1 && v <= cfg.r + 1 + cfg.neighbours_of_fixed_vertex_in_B; }

	int fixedVertexInB() const { return cfg.fixVertexInBMode != FixVertexInBMode::NONE ? cfg.r + 1 : throw "There is no fixed vertex in B."; }

	const std::vector<int>& A() const { return verticesInA; }
	const std::vector<int>& B() const { return verticesInB; }
	const std::vector<int>& getNeighOfFixedInB() const { return neighOfFixedInB; }
	const std::vector<int>& getOtherInB() const { return otherInB; }
};

std::string getFileName(const GraphConfig& cfg);

void generateGraphLP(const GraphConfig& cfg, const std::string& filename);