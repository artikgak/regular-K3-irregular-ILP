#include "generator.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>

using namespace std;

std::string getFileName(const GraphConfig& cfg)
{
    std::string res = "N" + std::to_string(cfg.n) + "_R" + std::to_string(cfg.r);
    res += "_K3_" + std::to_string(cfg.min_k3) + "_" + std::to_string(cfg.max_k3);

    if (cfg.use_split_AB)
        res += "_split_" + std::to_string(cfg.anchorK3);

    if (cfg.fixVertexInB)
    {
        res += "_fix" + std::to_string(cfg.k3degFixedInB) + "B_" + std::to_string(cfg.neighbours_of_fixed_vertex_in_B);
    }

    if (cfg.fixExactNumberOfNeighboursOfFixedInB)
    {
		res += "_exact";
    }

    if (cfg.fixRestNumberOfVerticesInA)
    {
		res += "_fixrestA";
    }

    if (cfg.useLemmas == false)
    {
		res += "_wo_lem";
    }

    res += ".lp";
    return res;
}

void writeRegularityCondition(std::ostream& out, const GraphConfig& cfg, GraphVarRegister& varRegister)
{
    // regularity
    for (int i = 0; i < cfg.n; i++)
    {
        out << "deg" << to_string(i) << ": ";

        bool first = true;

        for (int j = 0; j < cfg.n; j++)
        {
            if (i == j)
                continue;

            if (!first)
                out << " + ";

            out << varRegister.edge(i, j);

            first = false;
        }

        out << " = " << to_string(cfg.r) << "\n";
    }
	out << "\n";
}

void writeTrianglesK3Degs(std::ostream& out, const GraphConfig& cfg, GraphVarRegister& varRegister)
{
    // triangle definition
    for (int i = 0; i < cfg.n - 2; i++)
    {
        for (int j = i + 1; j < cfg.n - 1; j++)
        {
            for (int k = j + 1; k < cfg.n; k++)
            {
                const string t = varRegister.triangle(i, j, k);
				const string e_ij = varRegister.edge(i, j);
				const string e_ik = varRegister.edge(i, k);
				const string e_jk = varRegister.edge(j, k);

                out << "tri1_" << to_string(i) << "_" << to_string(j) << "_" << to_string(k) << ": "
                    << t << " - " << e_ij << " <= 0\n";

                out << "tri2_" << to_string(i) << "_" << to_string(j) << "_" << to_string(k) << ": "
                    << t << " - " << e_ik << " <= 0\n";

                out << "tri3_" << to_string(i) << "_" << to_string(j) << "_" << to_string(k) << ": "
                    << t << " - " << e_jk << " <= 0\n";

                out << "tri4_" << to_string(i) << "_" << to_string(j) << "_" << to_string(k) << ": "
                    << t << " - " << e_ij << " - " << e_ik << " - " << e_jk << " >= -2\n";
            }
        }
    }

	out << "\n";

    // K3-degree of vertices
    for (int v = 0; v < cfg.n; v++)
    {
        out << "k3deg" << to_string(v) << ": " << varRegister.k3deg(v) << " - ";

        bool first = true;

        for (int i = 0; i < cfg.n - 2; i++)
        {
            for (int j = i + 1; j < cfg.n - 1; j++)
            {
                for (int k = j + 1; k < cfg.n; k++)
                {
                    if (i == v || j == v || k == v)
                    {
                        if (!first) out << " - ";
                        out << varRegister.triangle(i, j, k);
                        first = false;
                    }
                }
            }
        }
        out << " = 0\n\n";
    }
	out << "\n";
}

void writeNoTrueTwinsCond(std::ostream& out, const GraphConfig& cfg, GraphVarRegister& varRegister)
{
    // Перебираємо всі можливі пари вершин (потенційні ребра)
    for (int i = 0; i < cfg.n - 1; i++)
    {
        for (int j = i + 1; j < cfg.n; j++)
        {
            out << "notruetwins_" << to_string(i) << "_" << to_string(j) << ": ";
            bool first = true;

            // Рахуємо всі можливі трикутники, які містять вершини i та j
            for (int k = 0; k < cfg.n; k++)
            {
                if (k == i || k == j) continue;

                if (!first) out << " + ";

                // Функція tvar очікує строго відсортовані індекси a < b < c
                int a = i, b = j, c = k;
                if (a > b) swap(a, b);
                if (b > c) swap(b, c);
                if (a > b) swap(a, b);

                out << varRegister.triangle(a, b, c);
                first = false;
            }
            // Максимум r-2 спільних сусідів (трикутників) на будь-якому ребрі
            out << " <= " << to_string(cfg.r - 2) << "\n";
        }
    }
	out << "\n";
}

// When anchor d is high 0 vertex can't belong to A. 
// So we can fix it in B and some edges in it's neighbourhood.
// d0 - fixed
// d1...d_r belong to A
// d_r+1 .. d_n-1 belong to B
// fix d_r+1 = 0
// neighboursInB in B (this goes from theory).
// Ex. d0=21 then 0 belong to B. And 0 has at max 3 edges to A and at least 5 edges to B (r=8)
// So we can fix zero neighborhood of 0 in B (P5)
void writeFixVertexInB(std::ostream& out, const GraphConfig& cfg, GraphVarRegister& varRegister)
{
    if(cfg.fixVertexInB == false)
		return;

	VertexSets vs(cfg);
	const int fixedVertex = vs.fixedVertexInB();
    const auto& fixedAdj = vs.getNeighOfFixedInB();
    const auto& otherInB = vs.getOtherInB();

	// fix vertex in B adjacency neighbourhood in B
    for (int i : fixedAdj)
    {
        out << varRegister.edge(fixedVertex, i) << " = 1\n";
    }

    // if we want to fix exact number of neighbours of fixed vertex in B, 
    // then we can also fix non-edges between vertex and non-neighbours in B
    if (cfg.fixExactNumberOfNeighboursOfFixedInB)
    {
        for (int v : otherInB)
        {
            out << varRegister.edge(fixedVertex, v) << " = 0\n";
		}
    }

    if (cfg.fixRestNumberOfVerticesInA)
    {
        const auto& neighOfFixedInBinA = vs.getNeigOfFixedInBinA();
        // fix neighbours of fixed vertex in B in A
        for (int v : neighOfFixedInBinA)
        {
            out << varRegister.edge(fixedVertex, v) << " = 1\n";
        }

        // fix non-neighbours of fixed vertex in B in A
        const auto& notNeigOfFixedInBinA = vs.getNotNeigOfFixedInBinA();
        for (int v : notNeigOfFixedInBinA)
        {
            out << varRegister.edge(fixedVertex, v) << " = 0\n";
        }

        out << "\n";
    }
}

void writeConditionOnNeighOfVertexInB(std::ostream& out, const GraphConfig& cfg, GraphVarRegister& varRegister)
{
    if (cfg.fixVertexInB == false)
        return;

    VertexSets vs(cfg);
    const int fixedVertex = vs.fixedVertexInB();
    const auto& fixedAllAdjOfFixedInB = vs.getAllNeighOfFixedInB();

    // empty neighbourhood of fixed vertex in B in B
    if (cfg.k3degFixedInB == 0)
    {
        for (int i = 0; i < fixedAllAdjOfFixedInB.size() - 1; ++i)
        {
            for (int j = i + 1; j < fixedAllAdjOfFixedInB.size(); ++j)
            {
                out << varRegister.edge(fixedAllAdjOfFixedInB[i], fixedAllAdjOfFixedInB[j]) << " = 0\n";
            }
        }
    }
    else
    {
        // sum of edges between neighbours of fixed vertex in B is <= k3degFixedInB
        out << "fix" << std::to_string(cfg.k3degFixedInB) << "B_tri: ";
        bool first = true;
        for (int i = 0; i < fixedAllAdjOfFixedInB.size() - 1; ++i)
        {
            for (int j = i + 1; j < fixedAllAdjOfFixedInB.size(); ++j)
            {
                if (!first) out << " + ";
                out << varRegister.edge(fixedAllAdjOfFixedInB[i], fixedAllAdjOfFixedInB[j]);
                first = false;
            }
        }
        if (cfg.fixExactNumberOfNeighboursOfFixedInB && cfg.fixRestNumberOfVerticesInA)
        {
            out << " = " << cfg.k3degFixedInB << "\n";
        }
        else
        {
            out << " <= " << cfg.k3degFixedInB << "\n";
        }
    }

    out << "\n";
}

void writeOrderTotalK3degCondition(std::ostream& out, const std::vector<int>& array, GraphVarRegister& varRegister)
{
    for (int i = 0; i < array.size() - 1; i++)
    {
        out << "ord" << to_string(array[i]) << "_" << to_string(array[i+1]) << ": "
            << varRegister.k3deg(array[i]) << " - " << varRegister.k3deg(array[i+1]) << " <= -1\n";
    }
}

void writePaiwiseDifferentK3degCondition(std::ostream& out, const std::vector<int>& arr1, const std::vector<int>& arr2, const GraphConfig& cfg, GraphVarRegister& varRegister)
{
    // paiwise different
    const int M = cfg.r * cfg.r; // use upper bound on max_k3_deg + epsilon
    for (int v : arr1)
    {
        for (int u : arr2)
        {
            const std::string b = varRegister.binary(v, u);
            const std::string dv = varRegister.k3deg(v);
            const std::string du = varRegister.k3deg(u);
            // binary variable
            out << "bin_" << to_string(v) << "_" << to_string(u) << ": "
                << b << " <= 1\n";
            // d_i < d_j or d_j < d_i
            out << "neq1_" << to_string(v) << "_" << to_string(u) << ": "
                << dv << " - " << du << " - " << to_string(M) << " " << b << " <= -1\n";
            out << "neq2_" << to_string(v) << "_" << to_string(u) << ": "
                << du << " - " << dv << " + " << to_string(M) << " " << b << " <= " << to_string(M - 1) << "\n";
        }
    }
}

void writeAllDiffK3Degs(std::ostream& out, const GraphConfig& cfg, GraphVarRegister& varRegister)
{
    const VertexSets vs(cfg);
    if (!cfg.use_split_AB)
    {
        writeOrderTotalK3degCondition(out, vs.getAllVertices(), varRegister);
        return;
    }
	
    const auto& verticesInA = vs.verticesInA;
    // ordering of K3-degrees inside A
    // if no fixed exact Rest in A we can do total order inside A
	// if not then we can only order neighbours of fixed in B in A, and 
    // non-neighbours of fixed in B in A, but not between them, because we don't know if they belong to A or B
	// and add pairwise check of not equality between them
    if (cfg.fixRestNumberOfVerticesInA == false)
    {
        writeOrderTotalK3degCondition(out, verticesInA, varRegister);
    }
    else
    {
        const auto& neighOfFixedInBinA = vs.getNeigOfFixedInBinA();
        writeOrderTotalK3degCondition(out, neighOfFixedInBinA, varRegister);

        out << "\n";

        const auto& notNeigOfFixedInBinA = vs.getNotNeigOfFixedInBinA();
        writeOrderTotalK3degCondition(out, notNeigOfFixedInBinA, varRegister);

		writePaiwiseDifferentK3degCondition(out, neighOfFixedInBinA, notNeigOfFixedInBinA, cfg, varRegister);
    }
    out << "\n";

    const auto& verticesInB = vs.verticesInB;
    if (cfg.fixVertexInB == false)
    {
        // ordering of K3-degrees inside B
        writeOrderTotalK3degCondition(out, verticesInB, varRegister);
    }
    else
    {
		const auto& fixedAdjInB = vs.getNeighOfFixedInB();
        writeOrderTotalK3degCondition(out, fixedAdjInB, varRegister);
        out << "\n";

        // order inside non-neighbours 0
		const auto& otherInB = vs.getOtherInB();
        writeOrderTotalK3degCondition(out, otherInB, varRegister);
        out << "\n";

        // order inbetween groups
        writePaiwiseDifferentK3degCondition(out, fixedAdjInB, otherInB, cfg, varRegister);
    }

    // TODO also fix if fixed vertex is not max min then we need pairwise check not equalit of if to other vertices
	// di < anchor || di > anchor and same for fixed vertex in B if it's not min or max in B

    // A neq B
    writePaiwiseDifferentK3degCondition(out, verticesInA, verticesInB, cfg, varRegister);
	out << "\n";
}

void writeConditionsOnEdges(std::ostream& out, const GraphConfig& cfg, GraphVarRegister& varRegister)
{
    if (!cfg.use_split_AB)
        return;

	VertexSets vertexSets(cfg);

    for ( int a : vertexSets.verticesInA )
    {
        out << "fix0_" << a << ": " << varRegister.edge(0, a) << " = 1\n";
	}

    for (int b : vertexSets.verticesInB)
    {
        out << "fix0_" << b << ": " << varRegister.edge(0, b) << " = 0\n";
    }

    const int countEdgesForAnchor = cfg.r;
    const int countEdgesInsideA = cfg.anchorK3;
    const int countEdgesBetweenAB = cfg.r * (cfg.r - 1) - 2 * cfg.anchorK3;
    const int countEdgesInsideB = cfg.n * cfg.r / 2 - countEdgesForAnchor - countEdgesInsideA - countEdgesBetweenAB;

	const auto& verticesInA = vertexSets.verticesInA;
    const auto& verticesInB = vertexSets.verticesInB;

    if (vertexSets.anchorK3() == 0)
    {
        // no edges inside A
        for (int i = 0; i < verticesInA.size() - 1; i++)
        {
            for (int j = i + 1; j < verticesInA.size(); j++)
            {
                out << "noedge_" << verticesInA[i] << "_" << verticesInA[j] << ": " 
                    << varRegister.edge(verticesInA[i], verticesInA[j]) << " = 0\n";
            }
        }
    }
    else
    {
        // count edges inside A
        out << "edges_inside_A: ";
        bool first = true;
        for (int i = 0; i < verticesInA.size() - 1; i++)
        {
            for (int j = i + 1; j < verticesInA.size(); j++)
            {
                if (!first) {
                    out << " + ";
                }
                out << varRegister.edge(verticesInA[i], verticesInA[j]);
                first = false;
            }
        }
        out << " = " << to_string(countEdgesInsideA) << "\n";
    }

    {
        // count edges inside B 
        out << "edges_inside_B: ";
        bool first = true;
        for (int i = 0; i < verticesInB.size() - 1; i++)
        {
            for (int j = i + 1; j < verticesInB.size(); j++)
            {
                if (!first) {
                    out << " + ";
                }
                out << varRegister.edge(verticesInB[i], verticesInB[j]);
                first = false;
            }
        }
        out << " = " << to_string(countEdgesInsideB) << "\n";
    }

    {
        // count edges between AB 
        out << "edges_between_AB: ";
        bool first = true;
        for (int a : verticesInA)
        {
            for (int b : verticesInB)
            {
                if (!first) {
                    out << " + ";
                }
                out << varRegister.edge(a, b);
                first = false;
            }
        }
        out << " = " << to_string(countEdgesBetweenAB) << "\n\n";
    }
}

// =========================================================================
// ЛЕМА 3.1: Для кожної вершини a \in A, deg_{G[A]}(a) <= min{r - 2, d}
// d=anchorK3, A has 1...r vertices, B has r+1...n-1 vertices
// =========================================================================
void writeLemma3_1(std::ostream& out, const GraphConfig& cfg, GraphVarRegister& varRegister)
{
	VertexSets vertexSets(cfg);
	const auto& verticesInA = vertexSets.verticesInA;
    for (int v : verticesInA)
    {
        std::string sum_adj_edges = "";
        bool first = true;
        for (int u : verticesInA)
        {
            if (v == u)
                continue;

            if (!first)
                sum_adj_edges += " + ";
            first = false;

            sum_adj_edges += varRegister.edge(v, u);
        }
        out << "lemma3_1_" << to_string(v) << ": " << sum_adj_edges << " <= " << to_string(min({ cfg.r - 2, cfg.anchorK3 })) << "\n";
    }
	out << "\n\n";
}


// =========================================================================
// ЛЕМА 3.4: Для кожної вершини b \in B, max{1, |B|-1-|E(\overline{G[B]})} <= deg_{G[B]}(b) <= min{r, |E(B)|, |B| - 1}
// d=anchorK3, A has 1...r vertices, B has r+1...n-1 vertices
// (R1) deg_B(b) <= r - maybe
// (R2) deg_b(b) <= |B| - 1 - trivial
// (R3) deg_b(b) <= |E(B)| - let's add this (good)
// (L1) deg_B(b) >= 1 - yes
// (L2) deg_B(b) >= |B| - 1 - |E(\overline{G[B]}) - yes
// =========================================================================
void writeLemma3_4(std::ostream& out, const GraphConfig& cfg, GraphVarRegister& varRegister)
{
    const int EdgesToFixedK3Deg = cfg.r;
    const int edgesInsideA = cfg.anchorK3;
    const int edgesBetweenAB = cfg.r * (cfg.r - 1) - 2 * cfg.anchorK3;
    const int edgesInsideB = cfg.n * cfg.r / 2 - EdgesToFixedK3Deg - edgesInsideA - edgesBetweenAB;
    const int verticesInsideB = cfg.n - cfg.r - 1;
    const int nonEdgesInsideB = verticesInsideB * (verticesInsideB - 1) / 2 - edgesInsideB;

	const auto& verticesInB = VertexSets(cfg).verticesInB;

    for (int v : verticesInB)
    {
        std::string sum_adj_edges = "";
        bool first = true;
        for (int u : verticesInB)
        {
            if (v == u)
                continue;

            if (!first)
                sum_adj_edges += " + ";
            first = false;

            sum_adj_edges += varRegister.edge(v, u);
        }

        out << "lemma3_4_r" << to_string(v) << ": " << sum_adj_edges << " <= " << to_string(std::min({ cfg.r, verticesInsideB - 1, edgesInsideB })) << "\n"; // min (R1) (R2) (R3)
        out << "lemma3_4_l" << to_string(v) << ": " << sum_adj_edges << " >= " << to_string(std::max({ 1, verticesInsideB - 1 - nonEdgesInsideB })) << "\n"; // max (L1) (L2)
    }
	out << "\n\n";
}

void writeBounds(std::ostream& out, const GraphConfig& cfg, GraphVarRegister& varRegister)
{
	VertexSets vs(cfg);
    int updminBound = cfg.min_k3;
	int updmaxBound = cfg.max_k3;

    if (cfg.use_split_AB && cfg.anchorK3 != cfg.min_k3 && cfg.anchorK3 != cfg.max_k3)
    {
		std::cerr << "Warning: anchorK3 is not equal to min_k3 or max_k3, so bounds for other vertices won't be updated\n";
    }
    if(cfg.fixVertexInB && cfg.k3degFixedInB != cfg.min_k3 && cfg.k3degFixedInB != cfg.max_k3)
    {
        std::cerr << "Warning: k3degFixedInB is not equal to min_k3 or max_k3, so bounds for other vertices won't be updated\n";
	}

    // 1. Межі для якоря (якщо є розбиття)
    if (cfg.use_split_AB)
    {
        out << varRegister.k3deg(0) << " = " << to_string(cfg.anchorK3) << "\n";
        if (cfg.anchorK3 == cfg.min_k3)
			updminBound = cfg.min_k3 + 1;
		if (cfg.anchorK3 == cfg.max_k3)
			updmaxBound = cfg.max_k3 - 1;
    }

    // 2. Межі для фіксованої вершини в B (якщо є)
    if (cfg.fixVertexInB)
    {
        const int fixedVertex = vs.fixedVertexInB();

        out << varRegister.k3deg(fixedVertex) << " = " << std::to_string(cfg.k3degFixedInB) << "\n";

        if (updminBound == cfg.k3degFixedInB)
            updminBound++;
        else if (updmaxBound == cfg.k3degFixedInB)
            updmaxBound--;
	}

    // 3. Записуємо межі для всіх інших вершин
    for (int i = 0; i < cfg.n; i++)
    {
        if (cfg.use_split_AB && i == 0)
        {
            // d0 is fixed to anchorK3
            continue;
		}

        if (cfg.fixVertexInB && i == vs.fixedVertexInB())
        {
            // already handled fixed vertex in B
            continue;
		}
        
        out << to_string(updminBound) << " <= " << varRegister.k3deg(i) << " <= " << to_string(updmaxBound) << "\n";
    }

	// write bounds for T using sum of k3 degrees
	// TODO maybe update bounds based on anchorK3 and fixVertexInBMode
    int sum_min = cfg.n * cfg.min_k3 + cfg.n * (cfg.n - 1) / 2;
    int sum_max = cfg.n * cfg.max_k3 - cfg.n * (cfg.n - 1) / 2;

    int Tmin = ceil(static_cast<float>(sum_min) / 3.f);
    int Tmax = floor(static_cast<float>(sum_max) / 3.f);

    out << "tbounds: " << Tmin << " <= " << varRegister.intvar("T") << " <= " << Tmax << "\n";
}

void writeBinaryVars(std::ostream& out, const GraphConfig& cfg, GraphVarRegister& varRegister)
{
	VertexSets vs(cfg);

    // edges
    for (int i = 0; i < cfg.n - 1; i++)
    {
        for (int j = i + 1; j < cfg.n; j++)
        {
            out << " " << varRegister.edge(i, j) << " ";
        }
        out << "\n";
    }

    // triangles
    for (int i = 0; i < cfg.n - 2; i++)
    {
        for (int j = i + 1; j < cfg.n - 1; j++)
        {
            for (int k = j + 1; k < cfg.n; k++)
            {
                out << " " << varRegister.triangle(i, j, k) << "\n";
            }
        }
    }

	const auto& verticesInA = vs.verticesInA;
	const auto& verticesInB = vs.verticesInB;

    // b_ vars for A neq B
    if (cfg.use_split_AB)
    {
        // a) Змінні b_i_j між множиною A та множиною B

        for (int v : verticesInA)
        {
            for (int u : verticesInB)
            {
				if (cfg.fixVertexInB && u == vs.fixedVertexInB())
					continue; // skip fixed vertex in B
				out << varRegister.binary(v, u) << "\n";
            }
        }
    }

    // binary vars for ordering between neighbour/non-neighbour groups of zero vertex
    if (cfg.fixVertexInB)
    {
        const auto& neighOfFixed = vs.getNeighOfFixedInB();
        const auto& otherInB = vs.getOtherInB();
        for (int v : neighOfFixed)
        {
            for (int u : otherInB)
            {
                out << varRegister.binary(v, u) << "\n";
            }
        }
    }

    out << "\n";
}

// fix anchorK3 - 0 index
// 1..r are neighbouns
// r+1..n-1 not neighbors
void generateGraphLP(const GraphConfig& cfg, const std::string& filename)
{
	GraphVarRegister varRegister;

    ofstream f(filename);

    f << "Minimize\n";
    f << " obj: 0\n\n";

    f << "Subject To\n\n";

    writeRegularityCondition(f, cfg, varRegister);

    writeTrianglesK3Degs(f, cfg, varRegister);

	writeNoTrueTwinsCond(f, cfg, varRegister);

    if (cfg.use_split_AB && cfg.fixVertexInB)
    {
	    writeFixVertexInB(f, cfg, varRegister);
		writeConditionOnNeighOfVertexInB(f, cfg, varRegister);
    }

	writeAllDiffK3Degs(f, cfg, varRegister);

    // sum k3 = 3T
    f << "sum_k3: ";
    for (int i = 0; i < cfg.n - 1; i++)
    {
        f << varRegister.k3deg(i) << " + ";
    }
    f << varRegister.k3deg(cfg.n - 1) << " - 3 " << varRegister.intvar("T") << " = 0\n";

    if (cfg.use_split_AB)
    {
        writeConditionsOnEdges(f, cfg, varRegister);
        if (cfg.useLemmas)
        {
	        writeLemma3_1(f, cfg, varRegister);
	        writeLemma3_4(f, cfg, varRegister);
        }
    }

    f << "\nBounds\n";
    writeBounds(f, cfg, varRegister);

	// write all vars at the end: binary vars, then general vars
    f << varRegister;

    f << "\nEnd\n";

    f.flush();
    f.close();

    std::cout << "LP file written to " << filename << "\n";
}