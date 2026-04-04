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
		.min_k3 = 0,
		.max_k3 = 22,
		.use_split_AB = true,
		.anchorK3 = 22,
		.fixVertexInBMode = FixVertexInBMode::NONE,
		.neighbours_of_fixed_vertex_in_B = -1
	};

	cfg.validate();

	const std::string filename = getFileName(cfg);

	generateGraphLP(cfg, filename + "t");

	return 0;
}