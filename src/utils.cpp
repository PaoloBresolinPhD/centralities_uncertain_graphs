#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

UncertainGraph load_uncertain_graph_no_probs(const std::string &file_path, double p) {
    
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

void assign_node_ids(const std::string &input_dataset_path, const std::string &output_dataset_path, const std::string &output_map_path) {

    // create the input and output streams
    std::ifstream input_dataset(input_dataset_path);
    std::ofstream output_dataset(output_dataset_path);
    std::ofstream output_map(output_map_path);

    // map that will contain the relations between original node ids and assigned integer ids
    std::unordered_map<std::string, int> node_map;

    // inverse version of the map, which is the one that will be saved
    std::vector<std::string> inverse_node_map;

    // string that will contain the current input line
    std::string input_line;

    // initialize the next id to assing
    int id = 0;

    // iterate through the lines in the input dataset
    while (std::getline(input_dataset, input_line)) {

        // create a stream for the current line
        std::istringstream input_line_stream(input_line);

        // initialize the vector that will contain all strings separated by spaces in the line
        std::vector<std::string> line_strings;
        
        // separate the words in the input line by any number of space characters
        std::string word;
        while (input_line_stream >> word) {
            line_strings.push_back(word);
            if (line_strings.size() > 2) {
                break;
            }
        }

        // if the number of words in the line is not 2, then the line does not store an edge
        if (line_strings.size() != 2)
            continue;

        // if there are #, then the line is usually a header or a comment, so skip
        if (line_strings[0].find("#") != std::string::npos || line_strings[0].find("#") != std::string::npos)
            continue;

        // iterate through the two nodes to encode them if not present in the map
        for (const std::string &w_s : line_strings) {

            // check if the iterator reached the end, meaning that w_s in not present in the map
            if (node_map.find(w_s) == node_map.end()) {

                // assign the string to a new integer id and update the mappings
                node_map[w_s] = id++;
                inverse_node_map.push_back(w_s);
            }
        }

        // write the encoded edge in the output file
        output_dataset << node_map[line_strings[0]] << " " << node_map[line_strings[1]] << "\n";
    }

    // save the inverse map
    for (int i = 0; i < id; ++i) {
        output_map << i << " " << inverse_node_map[i] << "\n";
    }

    // close all streams
    input_dataset.close();
    output_dataset.close();
    output_map.close();

}

void assign_uniform_edge_probs(UncertainGraph &uncertain_graph) {
    
}