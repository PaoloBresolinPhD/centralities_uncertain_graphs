#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

UncertainGraph load_uncertain_graph_no_probs(std::string file_path, double p) {
    
    // load the file
    std::ifstream input_file(file_path);

    // string that will be used to store the current line
    std::string line;

    // uncertain graph that will contain the edges in the file
    UncertainGraph uncertain_graph;
    uncertain_graph.m = 0;
    uncertain_graph.n = 0;

    // read the file line by line
    while (std::getline(input_file, line)) {

        // create a string stream with the current line
        std::istringstream string_stream(line);

        // define the two integers that will store the node ids, if present
        int u, v;

        // store the first two strings splitted by spaces if they are integers
        if (string_stream >> u >> v) {

            // ignore self-loops
            if (u == v) {
                continue;
            }

            // if one of the nodes has an id larger than the size of the adjacency vector, then we need to resize the vector
            int max_id = std::max(u, v);
            bool resized = false;
            if (max_id >= uncertain_graph.n) {
                uncertain_graph.n = max_id + 1;
                uncertain_graph.adj.resize(uncertain_graph.n);
                resized = true;
            }

            // check if (u, v) has already been added (if (u, v) is already present, then also (v, u) is present and viceversa)
            bool already_added = false;
            if (!resized) {
                for (const UncertainEdge &e : uncertain_graph.adj[u]) {
                    if (e.dst == v) {
                        already_added = true;
                        break;
                    }
                }
            }
            if (already_added) {
                continue;
            }

            // add (u, v) and (v, u)
            UncertainEdge uv;
            uv.dst = v;
            uv.id = uncertain_graph.m;
            uv.prob = p;
            uncertain_graph.adj[u].push_back(uv);
            UncertainEdge vu;
            vu.dst = u;
            vu.id = uncertain_graph.m;
            vu.prob = p;
            uncertain_graph.adj[v].push_back(vu);
            
            // update the number of edges
            ++uncertain_graph.m;
        }
    }

    // close the file
    input_file.close();

    return uncertain_graph;
}

