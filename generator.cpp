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

void writeRegularityCondition(std::ostream& out, const int n, const int r)
{
    // regularity
    for (int i = 0; i < n; i++)
    {
        out << "deg" << to_string(i) << ": ";

        bool first = true;

        for (int j = 0; j < n; j++)
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

        out << " = " << to_string(r) << "\n";
    }
}

void wrtiteTrianglesK3Degs(std::ostream& out, const int n, const int r)
{
    // triangle definition
    for (int i = 0; i < n - 2; i++)
    {
        for (int j = i + 1; j < n - 1; j++)
        {
            for (int k = j + 1; k < n; k++)
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
    for (int v = 0; v < n; v++)
    {
        out << "k3deg" << to_string(v) << ": " << degv(v) << " - ";

        bool first = true;

        for (int i = 0; i < n - 2; i++)
        {
            for (int j = i + 1; j < n - 1; j++)
            {
                for (int k = j + 1; k < n; k++)
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

void writeNoTrueTwinsCond(std::ostream& out, const int n, const int r)
{
    // Перебираємо всі можливі пари вершин (потенційні ребра)
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            out << "notruetwins_" << to_string(i) << "_" << to_string(j) << ": ";
            bool first = true;

            // Рахуємо всі можливі трикутники, які містять вершини i та j
            for (int k = 0; k < n; k++)
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
            out << " <= " << to_string(r - 2) << "\n";
        }
    }
}

void writeBinaryVars(std::ostream& out, const int n, const int r)
{
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            out << " " << evar(i, j) << " ";
        }
        out << "\n";
    }

    for (int i = 0; i < n - 2; i++)
    {
        for (int j = i + 1; j < n - 1; j++)
        {
            for (int k = j + 1; k < n; k++)
            {
                out << " " << tvar(i, j, k) << "\n";
            }
        }
    }
}

void writeAllDiffK3Degs(std::ostream& out, const int n, const int r, const int splitDeg, const int neighboursInB = -1)
{
    // ordering of K3-degrees inside A
    for (int i = 1; i < r; i++)
    {
        out << "ord" << to_string(i) << ": "
            << degv(i) << " - " << degv(i + 1)
            << " <= -1\n";
    }
    out << "\n";

    if (neighboursInB == -1)
    {
        // ordering of K3-degrees inside B
        for (int i = r + 1; i < n - 1; i++)
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
        for (int i = 1; i < neighboursInB; i++)
        {
            out << "ord" << to_string(r + 1 + i) << ": "
                << degv(r + 1 + i) << " - " << degv(r + 1 + i + 1)
                << " <= -1\n";
        }
        out << "\n";

        // order inside non-neighbours 0
        for (int i = r + 1 + neighboursInB + 1; i < n - 1; i++)
        {
            out << "ord" << to_string(i) << ": "
                << degv(i) << " - " << degv(i + 1)
                << " <= -1\n";
        }
        out << "\n";

        // order inbetween groups
        const int M = r*r; // use upper bound on max_k3_deg + epsilon
        for (int i = r + 1 + 1; i <= r + 1 + neighboursInB; i++)
        {
            for (int j = r + 1 + neighboursInB + 1; j < n; j++)
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
    int M = r*r; // use upper bound on max_k3_deg + epsilon
    for (int i = 1; i <= r; i++)
    {
		const int j_start = neighboursInB == -1 ? r + 1 : r + 1 + 1;
        for (int j = j_start; j < n; j++)
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
void writeLemma3_1(std::ostream& out, const int n, const int r, const int d)
{
    for (int i = 1; i <= r; ++i)
    {
        std::string sum_adj_edges = "";
        bool first = true;
        for (int j = 1; j <= r; j++)
        {
            if (i == j)
                continue;

            if (!first)
                sum_adj_edges += " + ";
            first = false;

            sum_adj_edges += evar(i, j);
        }
        out << "lemma3_1_" << to_string(i) << ": " << sum_adj_edges << " <= " << to_string(min({ r - 2, d })) << "\n";
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
void writeLemma3_4(std::ostream& out, const int n, const int r, const int d)
{
    const int EdgesToFixedK3Deg = r;
    const int edgesInsideA = d;
    const int edgesBetweenAB = r * (r - 1) - 2 * d;
    const int edgesInsideB = n * r / 2 - EdgesToFixedK3Deg - edgesInsideA - edgesBetweenAB;
    const int verticesInsideB = n - r - 1;
    const int nonEdgesInsideB = verticesInsideB * (verticesInsideB - 1) / 2 - edgesInsideB;

    for (int i = r + 1; i < n; ++i)
    {
        std::string sum_adj_edges = "";
        bool first = true;
        for (int j = r + 1; j < n; j++)
        {
            if (i == j)
                continue;

            if (!first)
                sum_adj_edges += " + ";
            first = false;

            sum_adj_edges += evar(i, j);
        }

        out << "lemma3_4_r" << to_string(i) << ": " << sum_adj_edges << " <= " << to_string(std::min({ r, verticesInsideB - 1, edgesInsideB })) << "\n"; // min (R1) (R2) (R3)
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
void writeExperimetalFixZeroInB(std::ostream& out, const int n, const int r, const int neighboursInB)
{
    if( neighboursInB == -1 )
		return;

    out << degv(r + 1) << " = 0\n";
    for (int i = 1; i <= neighboursInB; ++i)
    {
        out << evar(r + 1, r + 1 + i) << " = 1\n";
    }

    for (int i = 1; i <= neighboursInB - 1 && i < n - 1; ++i)
    {
        for (int j = i + 1; j <= neighboursInB && j < n; ++j)
        {
            out << evar(r + 1 + i, r + 1 + j) << " = 0\n";
        }
    }
}

// fix splitK3 - 0 index
// 1..r are neighbouns
// r+1..n-1 not neighbors
void generateSplitAB(const int n, const int r, const int min_k3_deg, const int max_k3_deg, const int splitK3, const std::string& filename, const int neighboursOfZeroInB)
{
    ofstream f(filename);

    f << "Minimize\n";
    f << " obj: 0\n\n";

    f << "Subject To\n\n";

    writeRegularityCondition(f, n, r);
    wrtiteTrianglesK3Degs(f, n, r);
    f << "\n";

    // Додаємо відсікання "справжніх близнюків"
	writeNoTrueTwinsCond(f, n, r);
    f << "\n";

	writeExperimetalFixZeroInB(f, n, r, neighboursOfZeroInB);
    f << "\n";

	writeAllDiffK3Degs(f, n, r, splitK3, neighboursOfZeroInB);

    // sum k3 = 3T
    f << "sum_k3: ";
    for (int i = 0; i < n - 1; i++)
    {
        f << degv(i) << " + ";
    }
    f << degv(n - 1) << " - 3 T = 0\n";

    for (int i = 1; i <= r; i++)
    {
        f << "fix0_" << i << ": x" << 0 << "_" << i << " = 1\n";
    }
    for (int i = r + 1; i < n; i++)
    {
        f << "fix0_" << i << ": x" << 0 << "_" << i << " = 0\n";
    }

    const int EdgesToFixedK3Deg = r;
    const int edgesInsideA = splitK3;
    const int edgesBetweenAB = r * (r - 1) - 2 * splitK3;
    const int edgesInsideB = n * r / 2 - EdgesToFixedK3Deg - edgesInsideA - edgesBetweenAB;

    if (splitK3 == 0)
    {
        // no edges inside A
        for (int i = 1; i <= r - 1; i++)
        {
            for (int j = i + 1; j <= r; j++)
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
        for (int i = 1; i < r - 1; i++)
        {
            for (int j = i + 1; j <= r; j++)
            {
                f << evar(i, j) << " + ";
            }
        }
        f << evar(r - 1, r) << " = " << to_string(edgesInsideA) << "\n";
    }

    // count edges inside B 
	f << "edges_inside_B: ";
    for (int i = r + 1; i < n - 1; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            if (i != n - 2 || j != n - 1)
                f << evar(i, j) << " + ";
        }
    }
    f << evar(n - 2, n - 1) << " = " << to_string(edgesInsideB) << "\n";

    // count edges between AB 
	f << "edges_between_AB: ";
    for (int i = 1; i <= r; i++)
    {
        for (int j = r + 1; j < n; j++)
        {
            if (i != r || j != n - 1)
                f << evar(i, j) << " + ";
        }
    }
    f << evar(r, n - 1) << " = " << to_string(edgesBetweenAB) << "\n\n";

	writeLemma3_1(f, n, r, splitK3);

    f << "\n\n\n";

	writeLemma3_4(f, n, r, splitK3);

    f << "\n\nBounds\n";

    assert(neighboursOfZeroInB == -1 || min_k3_deg == 0);
    const int updminBound = splitK3 == min_k3_deg || neighboursOfZeroInB != -1 ? min_k3_deg + 1 : min_k3_deg;
    const int updmaxBound = splitK3 == max_k3_deg ? max_k3_deg - 1 : max_k3_deg;
    // d0 is fixed to splitK3
    f << "d0 = " << to_string(splitK3) << "\n";
    for (int i = 1; i < n; i++)
    {
        if (neighboursOfZeroInB != -1 && i == r + 1)
        {
            // d_r+1 is fixed to 0
            f << degv(i) << " = 0\n";
            continue;
        }
        f << to_string(updminBound) << " <= " << degv(i) << " <= " << to_string(updmaxBound) << "\n";
    }

    int sum_min = n * min_k3_deg + n * (n - 1) / 2;
    int sum_max = n * max_k3_deg - n * (n - 1) / 2;

    int Tmin = ceil(static_cast<float>(sum_min) / 3.f);
    int Tmax = floor(static_cast<float>(sum_max) / 3.f);

    f << "tbounds: " << Tmin << " <= T <= " << Tmax << "\n";

    f << "\nBinary\n";

	writeBinaryVars(f, n, r);

    // b_ vars for A neq B
    {
        const int j_start = (neighboursOfZeroInB == -1) ? r + 1 : r + 1 + 1;
        for (int i = 1; i <= r; i++)
        {
            for (int j = j_start; j < n; j++)
            {
                f << "b_" << i << "_" << j << "\n";
            }
        }
    }

    // binary vars for ordering between neighbour/non-neighbour groups of zero vertex
    if (neighboursOfZeroInB != -1)
    {
        for (int i = r + 1 + 1; i <= r + 1 + neighboursOfZeroInB; i++)
        {
            for (int j = r + 1 + neighboursOfZeroInB + 1; j < n; j++)
            {
                f << "b_" << i << "_" << j << "\n";
            }
        }
    }

    f << "\n";

    f << "\nGeneral\n";

    for (int i = 0; i < n; i++)
    {
        f << " " << degv(i) << "\n";
    }

    f << " T" << "\n";

    f << "\nEnd\n";

    f.flush();
    f.close();

    cout << "LP file written to " << filename << "\n";
}