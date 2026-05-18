#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>

#include "generator.h"
#include "graphhelper.h"

int main()
{
	//solHelper();

	GraphConfig cfg = {
		.n = 18,
		.r = 8,
		.min_k3 = 1,
		.max_k3 = 20,
		.use_split_AB = true,
		.anchorK3 = 1,

		//.fixVertexInB = true,
		//.k3degFixedInB = 3,
		//.neighbours_of_fixed_vertex_in_B = 6,
		//.fixExactNumberOfNeighboursOfFixedInB = true,
		//.fixRestNumberOfVerticesInA = true,

		.fixVertexInA = false,
		.k3degFixedInA = 4,
		.neighbours_of_fixed_vertex_in_A_inside_A = 8,
		.fixRestNumberOfVerticesInB = false,
		
		.useLemmasOnEdgeDivision = false,
		.useLemmas31_34 = false,
		.usePolytopeMatrix = false
	};

	cfg.validate();

	const std::string filename = getFileName(cfg);
	//GeneratePresolve();
	generateGraphLP(cfg, filename);

	return 0;
}