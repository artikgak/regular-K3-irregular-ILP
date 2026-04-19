#pragma once
#include <string>
#include <ostream>
#include <tuple>
#include <set>
#include <map>

class GraphVarRegister
{
public:
	GraphVarRegister() = default;
	~GraphVarRegister() = default;

	GraphVarRegister(const GraphVarRegister&) = delete;
	GraphVarRegister& operator=(const GraphVarRegister&) = delete;
	GraphVarRegister(GraphVarRegister&&) = delete;
	GraphVarRegister& operator=(GraphVarRegister&&) = delete;

	// rewrite with var templates
	std::string edge(int i, int j)
	{
		if (i > j) std::swap(i, j);
		const std::string var = "x" + std::to_string(i) + "_" + std::to_string(j);
		edges_bin[{i, j}] = var;
		return var;
	}

	std::string triangle(int a, int b, int c)
	{
		if (a > b) std::swap(a, b);
		if (b > c) std::swap(b, c);
		if (a > b) std::swap(a, b);
		const std::string var = "t" + std::to_string(a) + "_" + std::to_string(b) + "_" + std::to_string(c);
		triangles_bin[{ a, b, c }] = var;
		return var;
	}

	std::string binary(int i, int j)
	{
		if (i > j) std::swap(i, j);
		const std::string var = "b_" + std::to_string(i) + "_" + std::to_string(j);
		other_bin[{i, j}] = var;
		return var;
	}

	std::string polytopeBin(int i, int j)
	{
		const std::string var = "z_" + std::to_string(i) + "_" + std::to_string(j);
		polytope_bin[{i, j}] = var;
		return var;
	}

	std::string k3deg(int k3deg)
	{
		const std::string var = "d" + std::to_string(k3deg);
		k3_deg_vars[k3deg] = var;
		return var;
	}

	std::string intvar(const std::string& var)
	{
		other_vars.insert(var);
		return var;
	}

	void printBinaryVars(std::ostream& out) const
	{
		int current_i = -1;
		for (const auto& [edge_pair, var_name] : edges_bin)
		{
			if (current_i != -1 && current_i != edge_pair.first)
			{
				out << "\n";
			}
			current_i = edge_pair.first;
			out << " " << var_name << " ";
		}

		out << "\n\n";

		for (const auto& [tri_tuple, var_name] : triangles_bin)
		{
			out << " " << var_name << "\n";
		}

		out << "\n";

		for (const auto& [bin_pair, var_name] : other_bin)
		{
			out << var_name << "\n";
		}

		out << "\n";

		for (const auto& [bin_pair, var_name] : polytope_bin)
		{
			out << var_name << "\n";
		}
	}

	void printIntVars(std::ostream& out) const
	{
		for (const auto& [k3deg, var_name] : k3_deg_vars)
		{
			out << " " << var_name << "\n";
		}
		out << "\n";
		for (const auto& var : other_vars)
		{
			out << " " << var << "\n";
		}
	}

private:

	// binary variables:
	std::map<std::pair<int, int>, std::string> edges_bin;
	std::map<std::tuple<int, int, int>, std::string> triangles_bin;
	std::map<std::pair<int, int>, std::string> other_bin;
	std::map<std::pair<int, int>, std::string> polytope_bin;

	// general integer variables:
	std::map<int, std::string> k3_deg_vars;
	std::set<std::string> other_vars;
};

inline std::ostream& operator<<(std::ostream& out, const GraphVarRegister& reg)
{
	out << "\nBinary\n";
	reg.printBinaryVars(out);
	out << "\nGeneral\n";
	reg.printIntVars(out);
	return out;
}