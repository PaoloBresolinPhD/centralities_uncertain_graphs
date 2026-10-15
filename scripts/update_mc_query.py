import os
from pathlib import Path
import argparse
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

def parse_args():

    # define the parser
    parser = argparse.ArgumentParser()

    # add the arguments
    parser.add_argument("-i", "--input_dir", required=True, type=str, help="Path to the directory with all produced results to load.")

    return parser.parse_args()

if __name__ == "__main__":

    # parse command line arguments
    args = parse_args()

    # query size
    q = 1000

    # number of input nodes
    n = 875713

    # path to the input directory
    input_path = Path(args.input_dir)

    # paths to all summary.tsv in the folder
    dfs_paths = input_path.rglob("summary.tsv")

    # iterate through the paths
    for df_path in dfs_paths:

        # load the df
        df = pd.read_csv(df_path, sep="\t")

        # update the time
        df["time"] = df["time"] / q * n

        # save the updated df
        df.to_csv(df_path, sep="\t", index=False)
