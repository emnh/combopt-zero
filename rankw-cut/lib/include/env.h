#ifndef ENV_H
#define ENV_H

#include <vector>

#include "graph.h"

unsigned long long linearRankWidth(std::vector<int> ordering, const Graph& g, int partiallyOrdered, bool greedy);

bool is_end(const Graph& g);
Graph step(const Graph& g, int action, std::vector<int>& adj_black, std::vector<int>& adj_white, int& reward);

#endif
