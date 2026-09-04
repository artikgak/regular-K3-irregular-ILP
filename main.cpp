#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>

#include "generator.h"
#include "graphhelper.h"

int main()
{
	GraphConfig cfg = {
		.n = 22,
		.r = 8,
		.min_k3 = 1,
		.max_k3 = 21,
		.use_split_AB = true,
		.anchorK3 = 21,

		.fixVertexInB = true,
		.k3degFixedInB = 1,
		.neighbours_of_fixed_vertex_in_B = 4,
		.fixExactNumberOfNeighboursOfFixedInB = true,
		.fixRestNumberOfVerticesInA = true,

		/*.fixVertexInA = true,
		.k3degFixedInA = 23,
		.neighbours_of_fixed_vertex_in_A_inside_A = 4,
		.fixRestNumberOfVerticesInB = true,*/

		/*.writeconditionOnDefectParts = true,
		.defectBound = 2,
		.useLemmas31_34 = false,
		.usePolytopeMatrix = false*/

		//.specialConditionFor9_20 = true,
	};

	cfg.validate();

	const std::string filename = getFileName(cfg);
	generateGraphLP(cfg, filename);

	// for sanity checks
	//solHelper();
	//GeneratePresolve();

	return 0;
}