#include "utils.hpp"
#include <iostream>

int main(int argc, char* argv[]) {

    // return an error if the number of arguments is not correct
    if (argc != 4) {
        std::cerr << "Error: invalid number of arguments.\n";
        std::cerr << "Usage: " << argv[0] << " <input_dataset_path> <output_dataset_path> <node_map_path>\n";
        return 1;
    }

    // extract the arguments
    std::string input_dataset_path = argv[1];
    std::string output_dataset_path = argv[2];
    std::string output_map_path = argv[3];

    // convert the ids of the nodes and save the converted graph
    std::cout << "Mapping the node ids from strings to int...\n";
    assign_node_ids(input_dataset_path, output_dataset_path, output_map_path);

    return 0;
}