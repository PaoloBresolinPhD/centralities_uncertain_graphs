import os
import argparse
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

def parse_args():

    # define the parser
    parser = argparse.ArgumentParser()

    # add the arguments
    parser.add_argument("--input_node", required=False, type=str, default=None, help="Path to the file with all the results for a sampled node in a sampled possible world.")
    parser.add_argument("--input_world", required=False, type=str, default=None, help="Path to the file with all the results for all nodes in a sampled possible world.")
    parser.add_argument("--input_uncertain", required=False, type=str, default=None, help="Path to the file with all the results for all nodes across several sampled possible worlds.")
    parser.add_argument("--n_worlds", required=False, type=int, default=None, help="Number of sampled possible worlds.")
    parser.add_argument("-o", "--output_dir", required=True, type=str, help="Path to the directory where to store the plots.")

    return parser.parse_args()

def plot_distances_distribution(input_df, title, output_path, expected=False):

    # plot the dataframe
    plt.close()

    sns.set_theme("paper")

    sns.barplot(data=input_df, x="distance", y="count")

    plt.title(title)
    if expected:
        plt.xlabel("Expected Distance")
    else:
        plt.xlabel("Distance")
    plt.ylabel("Count")
    
    # save the plot
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    plt.savefig(output_path)

if __name__ == "__main__":

    # parse command line arguments
    args = parse_args()

    # load and plot the results
    if args.input_node is not None:
        input_df = pd.read_csv(args.input_node)
        plot_distances_distribution(input_df, "Distances Distribution of Random Node in Random World", os.path.join(args.output_dir, "single_node_single_world.pdf"))
    if args.input_world is not None:
        input_df = pd.read_csv(args.input_world)
        plot_distances_distribution(input_df, "Distances Distribution of All Nodes in Random World", os.path.join(args.output_dir, "all_nodes_single_world.pdf"))
    if args.input_uncertain is not None:
        input_df = pd.read_csv(args.input_uncertain)
        plot_distances_distribution(input_df, f"Distances Distribution of All Nodes Across {args.n_worlds} Sampled Random Worlds", os.path.join(args.output_dir, "all_nodes_sampled_worlds.pdf"), expected=True)