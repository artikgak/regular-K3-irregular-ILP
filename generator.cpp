#include "generator.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

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

std::string genRegularityCondition(const int n, const int r)
{
    std::string res;
    // regularity
    for (int i = 0; i < n; i++)
    {
		res += "deg" + to_string(i) + ": ";

        bool first = true;

        for (int j = 0; j < n; j++)
        {
            if (i == j)
                continue;

            int a = min(i, j);
            int b = max(i, j);

            if (!first)
                res += " + ";

            res += evar(a, b);

            first = false;
        }

        res += " = " + to_string(r) + "\n";
    }
    return res;
}

std::string genTrianglesK3Degs(const int n, const int r)
{
    std::string res;
    // triangle definition
    for (int i = 0; i < n - 2; i++)
    {
        for (int j = i + 1; j < n - 1; j++)
        {
            for (int k = j + 1; k < n; k++)
            {
                string t = tvar(i, j, k);

                res += "tri1_" + to_string(i) + "_" + to_string(j) + "_" + to_string(k) + ": "
                    + t + " - " + evar(i, j) + " <= 0\n";

                res += "tri2_" + to_string(i) + "_" + to_string(j) + "_" + to_string(k) + ": "
                    + t + " - " + evar(i, k) + " <= 0\n";

                res += "tri3_" + to_string(i) + "_" + to_string(j) + "_" + to_string(k) + ": "
                    + t + " - " + evar(j, k) + " <= 0\n";

                res += "tri4_" + to_string(i) + "_" + to_string(j) + "_" + to_string(k) + ": "
                    + t + " - " + evar(i, j) + " - "
                    + evar(i, k) + " - " + evar(j, k)
                    + " >= -2\n";
            }
        }
    }

    // K3-degree of vertices
    for (int v = 0; v < n; v++)
    {
        res += "k3deg" + to_string(v) + ": " + degv(v) + " - ";

        bool first = true;

        for (int i = 0; i < n - 2; i++)
        {
            for (int j = i + 1; j < n - 1; j++)
            {
                for (int k = j + 1; k < n; k++)
                {
                    if (i == v || j == v || k == v)
                    {
                        if (!first) res += " - ";
                        res += tvar(i, j, k);
                        first = false;
                    }
                }
            }
        }
        res += " = 0\n";
    }
    return res;
}

std::string genBinaryVars(const int n, const int r)
{
    std::string res;
    for (int i = 0; i < n - 1; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            res += " " + evar(i, j) + " ";
        }
        res += "\n";
    }

    for (int i = 0; i < n - 2; i++)
    {
        for (int j = i + 1; j < n - 1; j++)
        {
            for (int k = j + 1; k < n; k++)
            {
                res += " " + tvar(i, j, k) + "\n";
            }
        }
    }
    return res;
}


void generateUsual(const int n, const int r, const int min_k3_deg, const int max_k3_deg, const std::string& filename)
{
    ofstream f(filename);

    f << "Minimize\n";
    f << " obj: 0\n\n";

    f << "Subject To\n\n";

    std::string regCond = genRegularityCondition(n, r);
    f << regCond;

    std::string trCond = genTrianglesK3Degs(n, r);
	f << trCond;

    f << "\n";

    // ordering of K3-degrees
    for (int i = 0; i < n - 1; i++)
    {
        f << "ord" << i << ": "
            << degv(i) << " - " << degv(i + 1)
            << " <= -1\n";
    }

    // sum k3 = 3T
    f << "sum_k3: ";
    for (int i = 0; i < n - 1; i++)
    {
        f << degv(i) << " + ";
    }
    f << degv(n - 1) << " - 3 T = 0\n";

    f << "\nBounds\n";

    for (int i = 0; i < n; i++)
    {
        const int lb = min_k3_deg + i;
        const int ub = max_k3_deg - (n - 1 - i);

        f << lb << " <= " << degv(i) << " <= " << ub << "\n";
    }

    int sum_min = n * min_k3_deg + n * (n - 1) / 2;
    int sum_max = n * max_k3_deg - n * (n - 1) / 2;

    int Tmin = ceil(static_cast<float>(sum_min) / 3.f);
    int Tmax = floor(static_cast<float>(sum_max) / 3.f);

    f << "tbounds: " << Tmin << " <= T <= " << Tmax << "\n";

    f << "\nBinary\n";

    std::string binVars = genBinaryVars(n, r);
    f << binVars;

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

// fix 0 = K3 0
// 1..8 are neighbouns
// 9..21 not neighbors
void generateSpecialBounds(const int n, const int r, const int min_k3_deg, const int max_k3_deg, const std::string& filename)
{
    const int n = 22;
    ofstream f(filename);

    f << "Minimize\n";
    f << " obj: 0\n\n";

    f << "Subject To\n\n";

    std::string regCond = genRegularityCondition(n, r);
    f << regCond;

    std::string trCond = genTrianglesK3Degs(n, r);
    f << trCond;

    f << "\n";

    // ordering of K3-degrees
    for (int i = 1; i < 8; i++)
    {
        f << "ord" << i << ": "
            << degv(i) << " - " << degv(i + 1)
            << " <= -1\n";
    }
    for (int i = 9; i < n-1; i++)
    {
        f << "ord" << i << ": "
            << degv(i) << " - " << degv(i + 1)
            << " <= -1\n";
    }

    int M = 30;
    for (int i = 1; i <= 8; i++)
    {
        for (int j = 9; j < n; j++)
        {
            std::string b = "b_" + std::to_string(i) + "_" + std::to_string(j);

            // binary variable
            f << "bin_" << i << "_" << j << ": " << b << " <= 1\n";

            // d_i < d_j or d_j < d_i
            f << "neq1_" << i << "_" << j << ": "
                << "d" << i << " - d" << j
                << " - " << M << " " << b
                << " <= -1\n";

            f << "neq2_" << i << "_" << j << ": "
                << "d" << j << " - d" << i
                << " + " << M << " " << b
                << " <= " << M - 1 << "\n";
        }
    }


    // sum k3 = 3T
    f << "sum_k3: ";
    for (int i = 0; i < n - 1; i++)
    {
        f << degv(i) << " + ";
    }
    f << degv(n - 1) << " - 3 T = 0\n";


    for (int i = 1; i <= 8; i++)
    {
        f << "fix0_" << i << ": x_0_" << i << " = 1\n";
    }
    for (int i = 9; i <= 21; i++)
    {
        f << "fix0_" << i << ": x_0_" << i << " = 0\n";
    }
    for (int i = 1; i <= 7; i++)
    {
        for (int j = i + 1; j <= 8; j++)
        {
            f << "noedge_" << i << "_" << j
                << ": x_" << i << "_" << j
                << " = 0\n";
        }
    }


    for (int i = 9; i < n - 1; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            if (i != n - 2 || j != n - 1)
            f << evar(i, j) << " + ";
        }
    }
    f << evar(n-2, n-1) << " = 24\n";

    for (int i = 1; i <= 8; i++)
    {
        for (int j = 9; j < n; j++)
        {
            if(i != 8 || j != n-1)
                f << evar(i, j) << " + ";
        }
    }
    f << evar(8, n - 1) << " = 56\n";

    f << "\nBounds\n";

    for (int i = 0; i < n; i++)
    {
        f << 1 << " <= " << degv(i) << " <= " << 22 << "\n";
    }

    int sum_min = n * min_k3_deg + n * (n - 1) / 2;
    int sum_max = n * max_k3_deg - n * (n - 1) / 2;

    int Tmin = ceil(static_cast<float>(sum_min) / 3.f);
    int Tmax = floor(static_cast<float>(sum_max) / 3.f);

    f << "tbounds: " << Tmin << " <= T <= " << Tmax << "\n";

    f << "\nBinary\n";

    std::string binVars = genBinaryVars(n, r);
    f << binVars;

    for (int i = 1; i <= 8; i++)
    {
        for (int j = 9; j < n; j++)
        {
            f << "b_" << i << "_" << j << "\n";
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