#pragma once
#include <string>
#include <vector>
#include <set>
#include <boost/algorithm/string.hpp>
#include <boost/graph/isomorphism.hpp>
#include <boost/graph/connected_components.hpp>
#include "TypeTraits.h"
#include <iostream>

std::vector<std::vector<int>> parseExpression(const std::string& expression) {
	std::string input = expression;
	boost::trim(input);

	// Remove the outermost square brackets
	input = input.substr(1, input.length() - 2);

	// Initialize the result vector
	std::vector<std::vector<int>> result;

	// Create a string stream for tokenization
	std::istringstream ss(input);

	// Tokenize the string by commas and square brackets
	std::string token;
	while (std::getline(ss, token, ']')) {
		// Remove any leading or trailing whitespace
		token = token.substr(token.find_first_not_of(" ,["));
		// Create a new vector for the current token
		std::vector<int> sub_vector;
		std::istringstream sub_ss(token);
		std::string sub_token;
		while (std::getline(sub_ss, sub_token, ',')) {
			sub_vector.push_back(std::stoi(sub_token));
		}
		result.push_back(sub_vector);
	}

	return result;
}

template <class TGraph>
TGraph fromVecToGraph(const std::vector<std::vector<int>>& vectors)
{
	TGraph res(vectors.size());
	for (int u = 0; u < vectors.size(); ++u)
	{
		for (int v = 0; v < vectors[u].size(); ++v)
		{
			if (!edge(u, vectors[u][v], res).second)
			{
				add_edge(u, vectors[u][v], res);
			}
		}
	}

	return std::move(res);
}

std::vector<int> K3Irregullar(const UndirectedGraph& graph)
{
	std::vector<int> k3_degrees;
	UndirectedGraphVertexIterator v, vend;
	for (boost::tie(v, vend) = vertices(graph); v != vend; ++v)
	{
		int k3_deg = 0;

		const auto in_edges = adjacent_vertices(*v, graph); // ????
		std::vector<int> in_edges_vec(in_edges.first, in_edges.second);

		for (int i = 0; i < in_edges_vec.size() - 1; ++i)
		{
			for (int j = i + 1; j < in_edges_vec.size(); ++j)
			{
				if (edge(in_edges_vec[i], in_edges_vec[j], graph).second)
				{
					k3_deg++;
				}
			}
		}
		k3_degrees.push_back(k3_deg);
	}
	assert(k3_degrees.size() == num_vertices(graph));
	return k3_degrees;
}

std::string generatePresove(const UndirectedGraph& graph)
{
	const int n = num_vertices(graph);
	//const int E = num_edges(graph);
	std::string res;

	const std::vector<int> k3_degs = K3Irregullar(graph);

	std::vector<std::pair<int, int> > k3_degs_with_index;
	for (int i = 0; i < k3_degs.size(); ++i)
	{
		k3_degs_with_index.push_back(std::pair<int, int>(k3_degs[i], i));
	}

	// Сортуємо за зростанням K3-степеня. 
	// Тепер позиція в масиві (від 0 до n-1) - це і є наш MILP-індекс!
	std::sort(k3_degs_with_index.begin(), k3_degs_with_index.end());

	// 1. Фіксуємо всі K3-степені (змінні d_i)
	for (int milp_idx = 0; milp_idx < n; ++milp_idx)
	{
		res += "d" + std::to_string(milp_idx)
			+ " " + std::to_string(k3_degs_with_index[milp_idx].first) + "\n";
	}

	for (int u_milp = 0; u_milp < n - 1; ++u_milp)
	{
		for (int v_milp = u_milp + 1; v_milp < n; ++v_milp)
		{
			// Дістаємо їхні оригінальні індекси в Boost-графі
			int u_orig = k3_degs_with_index[u_milp].second;
			int v_orig = k3_degs_with_index[v_milp].second;

			// Перевіряємо, чи є ребро між цими вершинами в оригінальному графі
			// Функція edge() у Boost повертає пару, де .second - це bool (існує чи ні)
			auto e = edge(u_orig, v_orig, graph);
			int edge_exists = e.second ? 1 : 0;

			// Виводимо готове рівняння для LP файлу
			res += "x" + std::to_string(u_milp) + "_" + std::to_string(v_milp) + " " + std::to_string(edge_exists) + "\n";
		}
	}

	return res;
}

std::string generatePresoveSplitted(const UndirectedGraph& graph, const GraphConfig& cfg)
{
	const int n = num_vertices(graph);
	std::string res;

	const std::vector<int> k3_degs = K3Irregullar(graph);

	// 1. Знаходимо anchor вершину із заданим K3-степенем (це наша MILP вершина 0)
	int v_anchor_orig = -1;
	for (int i = 0; i < n; ++i) {
		if (k3_degs[i] == cfg.anchorK3) {
			v_anchor_orig = i;
			break;
		}
	}

	// Захист від помилок: якщо такої вершини в графі немає
	if (v_anchor_orig == -1) {
		std::cerr << "Помилка: Вершину з K3-степенем " << cfg.anchorK3 << " не знайдено!" << std::endl;
		return "";
	}

	// 2. Розбиваємо решту вершин на Окіл (A) та Не-окіл (B)
	std::vector<std::pair<int, int>> neighborhood;
	std::vector<std::pair<int, int>> non_neighborhood;

	for (int i = 0; i < n; ++i) 
	{
		if (i == v_anchor_orig)
			continue;

		// Перевіряємо, чи є ребро між v_split та i
		auto e = edge(v_anchor_orig, i, graph);
		if (e.second) 
		{
			neighborhood.push_back({ k3_degs[i], i });
		}
		else 
		{
			non_neighborhood.push_back({ k3_degs[i], i });
		}
	}

	// 3. Сортуємо A групи незалежно за їхнім K3-степенем
	std::sort(neighborhood.begin(), neighborhood.end());

	// 4. Будуємо фінальний мапінг: MILP-індекс -> Оригінальний індекс C++
	std::vector<int> milp_to_orig(n);
	milp_to_orig[0] = v_anchor_orig;

	int current_milp_idx = 1;
	// Спочатку записуємо відсортованих сусідів (індекси 1..R)
	for (auto& p : neighborhood) 
	{
		milp_to_orig[current_milp_idx++] = p.second;
	}

	// Обробка B із урахуванням фіксації вершини в B (якщо потрібно)
	if (cfg.fixVertexInB)
	{
		// find fixed vertex in B (k3 deg = cfg.k3degFixedInB)
		int fixed_vertex_in_B_orig = -1;
		int fixed_idx_in_non_neighborhood = -1;

		for (int i = 0; i < non_neighborhood.size(); ++i) 
		{
			if (non_neighborhood[i].first == cfg.k3degFixedInB) 
			{
				fixed_vertex_in_B_orig = non_neighborhood[i].second;
				fixed_idx_in_non_neighborhood = i;
				break;
			}
		}

		if (fixed_vertex_in_B_orig == -1) 
		{
			std::cerr << "Error: vertex in B with K3-deg " << cfg.k3degFixedInB << " not found" << std::endl;
			return "";
		}

		// remove fixed vertex from non_neighborhood and put it in the correct MILP position (R+1)
		non_neighborhood.erase(non_neighborhood.begin() + fixed_idx_in_non_neighborhood);
		milp_to_orig[current_milp_idx++] = fixed_vertex_in_B_orig;

		// split rest of B into neighbours and non-neighbours of fixed vertex in B
		std::vector<std::pair<int, int>> neigh_of_fixed;
		std::vector<std::pair<int, int>> other_in_B;

		for (auto& p : non_neighborhood) 
		{
			auto e = edge(fixed_vertex_in_B_orig, p.second, graph);
			if (e.second) 
			{
				neigh_of_fixed.push_back(p);
			}
			else 
			{
				other_in_B.push_back(p);
			}
		}

		// Сортуємо окремо сусідів та не-сусідів фіксованої вершини в B
		std::sort(neigh_of_fixed.begin(), neigh_of_fixed.end());
		std::sort(other_in_B.begin(), other_in_B.end());

		// MIPL R+2..R+1+neighbours_of_fixed_vertex_in_B - сусіди фіксованої вершини в B
		for (auto& p : neigh_of_fixed) 
		{
			milp_to_orig[current_milp_idx++] = p.second;
		}

		// MIPL R+1+neighbours_of_fixed_vertex_in_B..N-1 - не-сусіди фіксованої вершини в B
		for (auto& p : other_in_B) 
		{
			milp_to_orig[current_milp_idx++] = p.second;
		}
	}
	else
	{
		// Потім відсортованих не-сусідів (індекси R+1..N-1)
		std::sort(non_neighborhood.begin(), non_neighborhood.end());
		for (auto& p : non_neighborhood) 
		{
			milp_to_orig[current_milp_idx++] = p.second;
		}
	}

	// Виводимо K3-степені (змінні d_i)
	for (int milp_idx = 0; milp_idx < n; ++milp_idx) 
	{
		int orig_idx = milp_to_orig[milp_idx];
		res += "d" + std::to_string(milp_idx)
			+ " " + std::to_string(k3_degs[orig_idx]) + "\n";
	}

	// виводимо ребра
	for (int u_milp = 0; u_milp < n - 1; ++u_milp)
	{
		for (int v_milp = u_milp + 1; v_milp < n; ++v_milp)
		{
			// Дістаємо їхні оригінальні індекси в Boost-графі
			int u_orig = milp_to_orig[u_milp];
			int v_orig = milp_to_orig[v_milp];

			// Перевіряємо, чи є ребро між цими вершинами в оригінальному графі
			// Функція edge() у Boost повертає пару, де .second - це bool (існує чи ні)
			auto e = edge(u_orig, v_orig, graph);
			int edge_exists = e.second ? 1 : 0;

			// Виводимо готове рівняння для LP файлу
			res += "x" + std::to_string(u_milp) + "_" + std::to_string(v_milp) + " " + std::to_string(edge_exists) + "\n";
		}
	}

	return res;
}


UndirectedGraph loadGraphFromSCIPSolution(const std::string& filename, int numVertices)
{
	UndirectedGraph graph(numVertices);
	std::ifstream file(filename);

	if (!file.is_open()) {
		std::cerr << "Помилка: не вдалося відкрити файл " << filename << std::endl;
		return graph;
	}

	std::string line;
	while (getline(file, line))
	{
		// Видаляємо пробіли на початку рядка, якщо вони є
		line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
			return !std::isspace(ch);
			}));

		if (line.empty()) continue;

		// Нас цікавлять тільки змінні ребер, які починаються на 'x'
		// Також перевіряємо, що наступний символ - це цифра (щоб не сплутати з іншими можливими змінними)
		if (line[0] == 'x' && isdigit(line[1]))
		{
			std::stringstream ss(line);
			std::string varName;
			double value;

			// Зчитуємо ім'я змінної (наприклад, "x0_1") та її значення (наприклад, 1)
			// (об'єктивне значення "(obj:0)" ігнорується, бо ми зчитуємо лише перші два токени)
			ss >> varName >> value;

			// Якщо ребро існує (значення більше 0.5, щоб уникнути багів з float, хоча зазвичай там рівно 1)
			if (value > 0.5)
			{
				size_t underscorePos = varName.find('_');
				if (underscorePos != std::string::npos)
				{
					// Витягуємо індекси u та v
					// varName.substr(1, ...) бере рядок після 'x' до '_'
					int u = stoi(varName.substr(1, underscorePos - 1));
					// varName.substr(...) бере рядок після '_'
					int v = stoi(varName.substr(underscorePos + 1));

					// Додаємо ребро у Boost граф
					add_edge(u, v, graph);
				}
			}
		}
	}

	file.close();
	std::cout << "ТестіліоГраф успішно завантажено! Кількість ребер: " << num_edges(graph) << std::endl;

	return graph;
}

void solHelper() 
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	//std::string expression = "[[1,2,3,4,5,6,7,11,12],[0,2,3,4,5,6,7,9,10],[0,1,3,4,5,6,7,13,23],[0,1,2,4,5,7,8,19,23],[0,1,2,3,5,6,7,8,14],[0,1,2,3,4,6,7,9,23],[0,1,2,4,5,7,8,9,23],[0,1,2,3,4,5,6,10,23],[3,4,6,11,12,16,20,21,22],[1,5,6,11,12,13,20,21,22],[1,7,11,12,14,16,20,21,22],[0,8,9,10,14,15,16,17,18],[0,8,9,10,15,17,18,19,23],[2,9,14,15,16,18,20,21,22],[4,10,11,13,15,17,19,21,22],[11,12,13,14,16,18,19,20,21],[8,10,11,13,15,17,18,19,23],[11,12,14,16,18,19,20,21,22],[11,12,13,15,16,17,20,21,22],[3,12,14,15,16,17,20,21,22],[8,9,10,13,15,17,18,19,23],[8,9,10,13,14,15,17,18,19],[8,9,10,13,14,17,18,19,23],[2,3,5,6,7,12,16,20,22]]";
	//UndirectedGraph graph9 = fromVecToGraph<UndirectedGraph>(parseExpression(expression));
	//	GraphConfig cfg = {
	//	.n = 24,
	//	.r = 9,
	//	.min_k3 = 3,
	//	.max_k3 = 26,
	//	.use_split_AB = true,
	//	.anchorK3 = 3,
	//	.fixVertexInB = true,
	//	.k3degFixedInB = 26,
	//	.neighbours_of_fixed_vertex_in_B = 6
	//};
	//cfg.validate();
	//std::string presolveStr = generatePresoveSplitted(graph9, cfg);
	//std::ofstream f("rv2_presolve9r_split26_B3.mst");
	//f << presolveStr;
	//f.flush();
	//f.close();

	UndirectedGraph graph9 = loadGraphFromSCIPSolution("testSol.txt", 24);
	std::vector<int> k3degs = K3Irregullar(graph9);

	for (int i = 0; i < k3degs.size(); ++i)
	{
		std::cout << k3degs[i] << ' ';
	}
	std::cout << "\nSorted:\n";
	std::sort(k3degs.begin(), k3degs.end());
	for (int i = 0; i < k3degs.size(); ++i)
	{
		std::cout << k3degs[i] << ' ';
	}
}