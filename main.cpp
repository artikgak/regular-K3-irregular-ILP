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
		.n = 20,
		.r = 8,
		.min_k3 = 1,
		.max_k3 = 20,
		.use_split_AB = true,
		.anchorK3 = 20,
		.fixVertexInB = true,
		.k3degFixedInB = 1,
		.neighbours_of_fixed_vertex_in_B = 4
	};

	cfg.validate();

	const std::string filename = getFileName(cfg);

	generateGraphLP(cfg, filename);

	return 0;
}