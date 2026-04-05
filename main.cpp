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
		.n = 24,
		.r = 9,
		.min_k3 = 3,
		.max_k3 = 26,
		.use_split_AB = true,
		.anchorK3 = 26,
		.fixVertexInB = false,
		.k3degFixedInB = 3,
		.neighbours_of_fixed_vertex_in_B = 6
	};

	cfg.validate();

	const std::string filename = getFileName(cfg);

	generateGraphLP(cfg, "tv2" + filename);

	return 0;
}