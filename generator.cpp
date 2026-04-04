#include "generator.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>

using namespace std;

// rewrite with var templates
string evar(int i, int j)
{
    if (i > j) swap(i, j);
    return "x" + to_string(i) + "_" + to_string(j);
}

string tvar(int i, int j, int k)
{
    return "t" + to_string(i) + "_" + to_string(j) + "_" + to_string(k);
}

string degv(int i)
{
    return "d" + to_string(i);
}

void writeRegularityCondition(std::ostream& out, const GraphConfig& cfg)
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

            int a = min(i, j);
            int b = max(i, j);

            if (!first)
                out << " + ";

            out << evar(a, b);

            first = false;
        }

        out << " = " << to_string(cfg.r) << "\n";
    }
}

void wrtiteTrianglesK3Degs(std::ostream& out, const GraphConfig& cfg)
{
    // triangle definition
    for (int i = 0; i < cfg.n - 2; i++)
    {
        for (int j = i + 1; j < cfg.n - 1; j++)
        {
            for (int k = j + 1; k < cfg.n; k++)
            {
                string t = tvar(i, j, k);

                out << "tri1_" << to_string(i) << "_" << to_string(j) << "_" << to_string(k) << ": "
                    << t << " - " << evar(i, j) << " <= 0\n";

                out << "tri2_" << to_string(i) << "_" << to_string(j) << "_" << to_string(k) << ": "
                    << t << " - " << evar(i, k) << " <= 0\n";

                out << "tri3_" << to_string(i) << "_" << to_string(j) << "_" << to_string(k) << ": "
                    << t << " - " << evar(j, k) << " <= 0\n";

                out << "tri4_" << to_string(i) << "_" << to_string(j) << "_" << to_string(k) << ": "
                    << t << " - " << evar(i, j) << " - "
                    << evar(i, k) << " - " << evar(j, k)
                    << " >= -2\n";
            }
        }
    }

    // K3-degree of vertices
    for (int v = 0; v < cfg.n; v++)
    {
        out << "k3deg" << to_string(v) << ": " << degv(v) << " - ";

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
                        out << tvar(i, j, k);
                        first = false;
                    }
                }
            }
        }
        out << " = 0\n";
    }
}

void writeNoTrueTwinsCond(std::ostream& out, const GraphConfig& cfg)
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

                out << tvar(a, b, c);
                first = false;
            }
            // Максимум r-2 спільних сусідів (трикутників) на будь-якому ребрі
            out << " <= " << to_string(cfg.r - 2) << "\n";
        }
    }
}

void writeBinaryVars(std::ostream& out, const GraphConfig& cfg)
{
    for (int i = 0; i < cfg.n - 1; i++)
    {
        for (int j = i + 1; j < cfg.n; j++)
        {
            out << " " << evar(i, j) << " ";
        }
        out << "\n";
    }

    for (int i = 0; i < cfg.n - 2; i++)
    {
        for (int j = i + 1; j < cfg.n - 1; j++)
        {
            for (int k = j + 1; k < cfg.n; k++)
            {
                out << " " << tvar(i, j, k) << "\n";
            }
        }
    }
}

void writeAllDiffK3Degs(std::ostream& out, const GraphConfig& cfg)
{
    // ordering of K3-degrees inside A
    for (int i = 1; i < cfg.r; i++)
    {
        out << "ord" << to_string(i) << ": "
            << degv(i) << " - " << degv(i + 1)
            << " <= -1\n";
    }
    out << "\n";

    if (cfg.neighbours_of_fixed_vertex_in_B == -1)
    {
        // ordering of K3-degrees inside B
        for (int i = cfg.r + 1; i < cfg.n - 1; i++)
        {
            out << "ord" << to_string(i) << ": "
                << degv(i) << " - " << degv(i + 1)
                << " <= -1\n";
        }
    }
    else
    {
        // order inside neighbours 0
        // dr+1 = 0
        // r+2
        for (int i = 1; i < cfg.neighbours_of_fixed_vertex_in_B; i++)
        {
            out << "ord" << to_string(cfg.r + 1 + i) << ": "
                << degv(cfg.r + 1 + i) << " - " << degv(cfg.r + 1 + i + 1)
                << " <= -1\n";
        }
        out << "\n";

        // order inside non-neighbours 0
        for (int i = cfg.r + 1 + cfg.neighbours_of_fixed_vertex_in_B + 1; i < cfg.n - 1; i++)
        {
            out << "ord" << to_string(i) << ": "
                << degv(i) << " - " << degv(i + 1)
                << " <= -1\n";
        }
        out << "\n";

        // order inbetween groups
        const int M = cfg.r * cfg.r; // use upper bound on max_k3_deg + epsilon
        for (int i = cfg.r + 1 + 1; i <= cfg.r + 1 + cfg.neighbours_of_fixed_vertex_in_B; i++)
        {
            for (int j = cfg.r + 1 + cfg.neighbours_of_fixed_vertex_in_B + 1; j < cfg.n; j++)
            {
                std::string b = "b_" + std::to_string(i) + "_" + std::to_string(j);

                // binary variable
                out << "bin_" << to_string(i) << "_" << to_string(j) << ": " << b << " <= 1\n";

                // d_i < d_j or d_j < d_i
                out << "neq1_" << to_string(i) << "_" << to_string(j) << ": "
                    << "d" << to_string(i) << " - d" << to_string(j)
                    << " - " << to_string(M) << " " << b
                    << " <= -1\n";

                out << "neq2_" << to_string(i) << "_" << to_string(j) << ": "
                    << "d" << to_string(j) << " - d" << to_string(i)
                    << " + " << to_string(M) << " " << b
                    << " <= " << to_string(M - 1) << "\n";
            }
        }
    }

    // A neq B
    // some large constant for big-M constraints
    int M = cfg.r * cfg.r; // use upper bound on max_k3_deg + epsilon
    for (int i = 1; i <= cfg.r; i++)
    {
		const int j_start = cfg.neighbours_of_fixed_vertex_in_B == -1 ? cfg.r + 1 : cfg.r + 1 + 1;
        for (int j = j_start; j < cfg.n; j++)
        {
            std::string b = "b_" + std::to_string(i) + "_" + std::to_string(j);

            // binary variable
            out << "bin_" << to_string(i) << "_" << to_string(j) << ": " << b << " <= 1\n";

            // d_i < d_j or d_j < d_i
            out << "neq1_" << to_string(i) << "_" << to_string(j) << ": "
                << "d" << to_string(i) << " - d" << to_string(j)
                << " - " << to_string(M) << " " << b
                << " <= -1\n";

            out << "neq2_" << to_string(i) << "_" << to_string(j) << ": "
                << "d" << to_string(j) << " - d" << to_string(i)
                << " + " << to_string(M) << " " << b
                << " <= " << to_string(M - 1) << "\n";
        }
    }
}

// =========================================================================
// ЛЕМА 3.1: Для кожної вершини a \in A, deg_{G[A]}(a) <= min{r - 2, d}
// d=splitK3, A has 1...r vertices, B has r+1...n-1 vertices
// =========================================================================
void writeLemma3_1(std::ostream& out, const GraphConfig& cfg)
{
    for (int i = 1; i <= cfg.r; ++i)
    {
        std::string sum_adj_edges = "";
        bool first = true;
        for (int j = 1; j <= cfg.r; j++)
        {
            if (i == j)
                continue;

            if (!first)
                sum_adj_edges += " + ";
            first = false;

            sum_adj_edges += evar(i, j);
        }
        out << "lemma3_1_" << to_string(i) << ": " << sum_adj_edges << " <= " << to_string(min({ cfg.r - 2, cfg.splitK3 })) << "\n";
    }
}


// =========================================================================
// ЛЕМА 3.4: Для кожної вершини b \in B, max{1, |B|-1-|E(\overline{G[B]})} <= deg_{G[A]}(a) <= min{r, |E(B)|, |B| - 1}
// d=splitK3, A has 1...r vertices, B has r+1...n-1 vertices
// (R1) deg_B(b) <= r - maybe
// (R2) deg_b(b) <= |B| - 1 - trivial
// (R3) deg_b(b) <= |E(B)| - let's add this (good)
// (L1) deg_B(b) >= 1 - yes
// (L2) deg_B(b) >= |B| - 1 - |E(\overline{G[B]}) - yes
// =========================================================================
void writeLemma3_4(std::ostream& out, const GraphConfig& cfg)
{
    const int EdgesToFixedK3Deg = cfg.r;
    const int edgesInsideA = cfg.splitK3;
    const int edgesBetweenAB = cfg.r * (cfg.r - 1) - 2 * cfg.splitK3;
    const int edgesInsideB = cfg.n * cfg.r / 2 - EdgesToFixedK3Deg - edgesInsideA - edgesBetweenAB;
    const int verticesInsideB = cfg.n - cfg.r - 1;
    const int nonEdgesInsideB = verticesInsideB * (verticesInsideB - 1) / 2 - edgesInsideB;

    for (int i = cfg.r + 1; i < cfg.n; ++i)
    {
        std::string sum_adj_edges = "";
        bool first = true;
        for (int j = cfg.r + 1; j < cfg.n; j++)
        {
            if (i == j)
                continue;

            if (!first)
                sum_adj_edges += " + ";
            first = false;

            sum_adj_edges += evar(i, j);
        }

        out << "lemma3_4_r" << to_string(i) << ": " << sum_adj_edges << " <= " << to_string(std::min({ cfg.r, verticesInsideB - 1, edgesInsideB })) << "\n"; // min (R1) (R2) (R3)
        out << "lemma3_4_l" << to_string(i) << ": " << sum_adj_edges << " >= " << to_string(std::max({ 1, verticesInsideB - 1 - nonEdgesInsideB })) << "\n"; // max (L1) (L2)
    }
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
void writeExperimetalFixZeroInB(std::ostream& out, const GraphConfig& cfg)
{
    if( cfg.fixVertexInBMode == FixVertexInBMode::NONE )
		return;

    out << degv(cfg.r + 1) << " = 0\n";
    for (int i = 1; i <= cfg.neighbours_of_fixed_vertex_in_B; ++i)
    {
        out << evar(cfg.r + 1, cfg.r + 1 + i) << " = 1\n";
    }

    for (int i = 1; i <= cfg.neighbours_of_fixed_vertex_in_B - 1 && i < cfg.n - 1; ++i)
    {
        for (int j = i + 1; j <= cfg.neighbours_of_fixed_vertex_in_B && j < cfg.n; ++j)
        {
            out << evar(cfg.r + 1 + i, cfg.r + 1 + j) << " = 0\n";
        }
    }
}

std::string getFileName(const GraphConfig& cfg)
{
	std::string res = "N" + std::to_string(cfg.n) + "_R" + std::to_string(cfg.r);
    res += "_K3_" + std::to_string(cfg.min_k3) + "_" + std::to_string(cfg.max_k3);
    
    if (cfg.use_split_AB)
        res += "_split_" + std::to_string(cfg.splitK3);
         
    if (cfg.fixVertexInBMode != FixVertexInBMode::NONE)
    {
        switch (cfg.fixVertexInBMode)
        {
        case FixVertexInBMode::ZERO_IN_B:
            res += "_fix0B_";
		    break;
        case FixVertexInBMode::ONE_IN_B:
            res += "_fix1B_";
		    break;
        default:
			std::cerr << "Warning: Unknown FixVertexInBMode\n";
            res += "_fix__B_";
            break;
        }
    }

    res += ".lp";
    return res;
}

// fix splitK3 - 0 index
// 1..r are neighbouns
// r+1..n-1 not neighbors
void generateGraphLP(const GraphConfig& cfg, const std::string& filename)
{
    ofstream f(filename);

    f << "Minimize\n";
    f << " obj: 0\n\n";

    f << "Subject To\n\n";

    writeRegularityCondition(f, cfg);
    wrtiteTrianglesK3Degs(f, cfg);
    f << "\n";

    // Додаємо відсікання "справжніх близнюків"
	writeNoTrueTwinsCond(f, cfg);
    f << "\n";

	writeExperimetalFixZeroInB(f, cfg);
    f << "\n";

	writeAllDiffK3Degs(f, cfg);

    // sum k3 = 3T
    f << "sum_k3: ";
    for (int i = 0; i < cfg.n - 1; i++)
    {
        f << degv(i) << " + ";
    }
    f << degv(cfg.n - 1) << " - 3 T = 0\n";

    for (int i = 1; i <= cfg.r; i++)
    {
        f << "fix0_" << i << ": x" << 0 << "_" << i << " = 1\n";
    }
    for (int i = cfg.r + 1; i < cfg.n; i++)
    {
        f << "fix0_" << i << ": x" << 0 << "_" << i << " = 0\n";
    }

    const int EdgesToFixedK3Deg = cfg.r;
    const int edgesInsideA = cfg.splitK3;
    const int edgesBetweenAB = cfg.r * (cfg.r - 1) - 2 * cfg.splitK3;
    const int edgesInsideB = cfg.n * cfg.r / 2 - EdgesToFixedK3Deg - edgesInsideA - edgesBetweenAB;

    if (cfg.splitK3 == 0)
    {
        // no edges inside A
        for (int i = 1; i <= cfg.r - 1; i++)
        {
            for (int j = i + 1; j <= cfg.r; j++)
            {
                f << "noedge_" << i << "_" << j
                    << ": x" << i << "_" << j
                    << " = 0\n";
            }
        }
    }
    else
    {
        // count edges inside A
		f << "edges_inside_A: ";
        for (int i = 1; i < cfg.r - 1; i++)
        {
            for (int j = i + 1; j <= cfg.r; j++)
            {
                f << evar(i, j) << " + ";
            }
        }
        f << evar(cfg.r - 1, cfg.r) << " = " << to_string(edgesInsideA) << "\n";
    }

    // count edges inside B 
	f << "edges_inside_B: ";
    for (int i = cfg.r + 1; i < cfg.n - 1; i++)
    {
        for (int j = i + 1; j < cfg.n; j++)
        {
            if (i != cfg.n - 2 || j != cfg.n - 1)
                f << evar(i, j) << " + ";
        }
    }
    f << evar(cfg.n - 2, cfg.n - 1) << " = " << to_string(edgesInsideB) << "\n";

    // count edges between AB 
	f << "edges_between_AB: ";
    for (int i = 1; i <= cfg.r; i++)
    {
        for (int j = cfg.r + 1; j < cfg.n; j++)
        {
            if (i != cfg.r || j != cfg.n - 1)
                f << evar(i, j) << " + ";
        }
    }
    f << evar(cfg.r, cfg.n - 1) << " = " << to_string(edgesBetweenAB) << "\n\n";

	writeLemma3_1(f, cfg);

    f << "\n\n\n";

	writeLemma3_4(f, cfg);

    f << "\n\nBounds\n";

    assert(cfg.neighbours_of_fixed_vertex_in_B == -1 || cfg.min_k3 == 0);
    const int updminBound = cfg.splitK3 == cfg.min_k3 || cfg.neighbours_of_fixed_vertex_in_B != -1 ? cfg.min_k3 + 1 : cfg.min_k3;
    const int updmaxBound = cfg.splitK3 == cfg.max_k3 ? cfg.max_k3 - 1 : cfg.max_k3;
    // d0 is fixed to splitK3
    f << "d0 = " << to_string(cfg.splitK3) << "\n";
    for (int i = 1; i < cfg.n; i++)
    {
        if (cfg.neighbours_of_fixed_vertex_in_B != -1 && i == cfg.r + 1)
        {
            // d_r+1 is fixed to 0
            f << degv(i) << " = 0\n";
            continue;
        }
        f << to_string(updminBound) << " <= " << degv(i) << " <= " << to_string(updmaxBound) << "\n";
    }

    int sum_min = cfg.n * cfg.min_k3 + cfg.n * (cfg.n - 1) / 2;
    int sum_max = cfg.n * cfg.max_k3 - cfg.n * (cfg.n - 1) / 2;

    int Tmin = ceil(static_cast<float>(sum_min) / 3.f);
    int Tmax = floor(static_cast<float>(sum_max) / 3.f);

    f << "tbounds: " << Tmin << " <= T <= " << Tmax << "\n";

    f << "\nBinary\n";

	writeBinaryVars(f, cfg);

    // b_ vars for A neq B
    {
        const int j_start = (cfg.neighbours_of_fixed_vertex_in_B == -1) ? cfg.r + 1 : cfg.r + 1 + 1;
        for (int i = 1; i <= cfg.r; i++)
        {
            for (int j = j_start; j < cfg.n; j++)
            {
                f << "b_" << i << "_" << j << "\n";
            }
        }
    }

    // binary vars for ordering between neighbour/non-neighbour groups of zero vertex
    if (cfg.neighbours_of_fixed_vertex_in_B != -1)
    {
        for (int i = cfg.r + 1 + 1; i <= cfg.r + 1 + cfg.neighbours_of_fixed_vertex_in_B; i++)
        {
            for (int j = cfg.r + 1 + cfg.neighbours_of_fixed_vertex_in_B + 1; j < cfg.n; j++)
            {
                f << "b_" << i << "_" << j << "\n";
            }
        }
    }

    f << "\n";

    f << "\nGeneral\n";

    for (int i = 0; i < cfg.n; i++)
    {
        f << " " << degv(i) << "\n";
    }

    f << " T" << "\n";

    f << "\nEnd\n";

    f.flush();
    f.close();

    cout << "LP file written to " << filename << "\n";
}