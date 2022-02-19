#include "env.h"

#include <map>
#include <utility>

#include "mis_experiments/src/prelude.cpp"
#include "mis_experiments/src/gf2rank.cpp"
#include "mis_experiments/src/linearRankWidth.cpp"

bool is_end(const Graph& g) {
    return g.num_nodes == g.num_nodesColored;
}

unsigned long long linearRankWidth(std::vector<int> ordering, const Graph& g, bool greedy) {
    vector<hoodtype> neighbourhoods;
    for (int i = 0; i < g.num_nodes; i++) {
        neighbourhoods.push_back(hoodtype());
    }
    // Add edges from graph to neighbourhoods
    for (auto& p : g.edge_list) {
        int u = p.first, v = p.second;
        neighbourhoods[u].set(v);
        neighbourhoods[v].set(u);
    }
    if (greedy) {
        sort(ordering.begin(), ordering.end(), 
            [&neighbourhoods](const int& a, const int& b) 
                { 
                    // return a < b;
                    return neighbourhoods[a].count() > neighbourhoods[b].count();
                });
    }
    return linearRankWidth(ordering, neighbourhoods, greedy);
}

Graph step(const Graph& g, int action, std::vector<int>& adj_black, std::vector<int>& adj_white, int& reward) {
    reward = 0;
    const unsigned int n = g.num_nodes;
    assert(n);
    assert((unsigned int)adj_black.size() == n);
    assert((unsigned int)adj_white.size() == n);
    int actual_node;
    bool alreadyColored = false;

    if ((unsigned int) action < n) {
        actual_node = action;
        if (adj_black[actual_node] || adj_white[actual_node]) {
            alreadyColored = true;
        }
        adj_black[actual_node] = 1;
        // reward += adj_white[actual_node];
        // for (int a : g.adj_list[actual_node]) {
        //     adj_black[a]++;
        // }
    } else {
        actual_node = action - n;
        if (adj_white[actual_node] || adj_black[actual_node]) {
            alreadyColored = true;
        }
        adj_white[actual_node] = 1;
        // reward += adj_black[actual_node];
        // for (int a : g.adj_list[actual_node]) {
        //     adj_white[a]++;
        // }
    }
    // adj_black.erase(adj_black.begin() + actual_node);
    // adj_white.erase(adj_white.begin() + actual_node);

    // white to world
    vector<hoodtype> neighbourhoodsA;
    // black to world
    vector<hoodtype> neighbourhoodsB;
    // white to black
    vector<hoodtype> neighbourhoodsC;

    hoodtype all;
    hoodtype white;
    hoodtype black;
    hoodtype world;
    int whiteCount = 0;
    int blackCount = 0;
    for (unsigned int i = 0; i < n; i++) {
        neighbourhoodsA.push_back(hoodtype());
        neighbourhoodsB.push_back(hoodtype());
        neighbourhoodsC.push_back(hoodtype());
        if (adj_black[i]) {
            blackCount++;
            black.set(i);
        }
        else if (adj_white[i]) {
            whiteCount++;
            white.set(i);
        }
        else {
            world.set(i);
        }
        all.set(i);
    }

    Graph ret(n);
    // ret.num_nodesColored = g.num_nodesColored + 1;
    ret.num_nodesColored = whiteCount + blackCount;
    
    for (auto& p : g.edge_list) {
        int u = p.first, v = p.second;
        if (adj_black[u] && adj_black[v]) {
            continue;
        }
        if (adj_white[u] && adj_white[v]) {
            continue;
        }
        ret.add_edge(u, v);

        if (u > v) {
            swap(u, v);
        }
        if (adj_white[u] && !adj_black[v]) {
            neighbourhoodsA[u].set(v);
            neighbourhoodsA[v].set(u);
        }
        if (!adj_white[u] && adj_black[v]) {
            neighbourhoodsB[u].set(v);
            neighbourhoodsB[v].set(u);
        }
        if (adj_white[u] && adj_black[v]) {
            neighbourhoodsC[u].set(v);
            neighbourhoodsC[v].set(u);
        }
        // if (u == actual_node || v == actual_node) continue;
        // if (u > actual_node) u--;
        // if (v > actual_node) v--;
    }

    vector<hoodtype> neighbourhoodsA_left;
    vector<hoodtype> neighbourhoodsB_left;
    vector<hoodtype> neighbourhoodsC_left;
    for (unsigned int i = 0; i < n; i++) {
        if (white.test(i)) {
            neighbourhoodsA_left.push_back(neighbourhoodsA[i]);
        }
        else if (black.test(i)) {
            neighbourhoodsB_left.push_back(neighbourhoodsB[i]);
        }
        else {
            neighbourhoodsC_left.push_back(neighbourhoodsC[i]);
        }
    }

    
    // if (alreadyColored) {
    //     reward = -100000;
    // } else {
    const unsigned int rankA = gf2_rank(neighbourhoodsA_left);
    const unsigned int rankB = gf2_rank(neighbourhoodsB_left);
    const unsigned int rankC = gf2_rank(neighbourhoodsC_left);
    reward = max(max(rankA, rankB), rankC);

    if (abs(whiteCount - blackCount) > n / 3) {
        reward += (abs(whiteCount - blackCount) - n / 3) * 100;
    } else {
        reward += abs(whiteCount - blackCount);
    }
    
    // Punish for not coloring
    // const int smallest = min(whiteCount, blackCount);
    // reward += (g.num_nodes - smallest) * 2;

    // reward = rankC;
    reward = -reward;
    // }

    return ret;
}
