import os
from pathlib import Path
import argparse
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import seaborn as sns

def parse_args():

    # define the parser
    parser = argparse.ArgumentParser()

    # add the arguments
    parser.add_argument("-i", "--input_dir", required=True, type=str, help="Path to the directory with all produced results to load.")
    parser.add_argument("-o", "--output_dir", required=True, type=str, help="Path to the directory where to store the combined results.")
    parser.add_argument("-m", "--measure", required=True, type=str, help="Centrality measure to consider. Can be either \"lin\" or \"harmonic\".")
    parser.add_argument("-d", "--dataset", required=True, type=str, default=None, help="Name of the dataset.")

    return parser.parse_args()

def update_metadata_with_mae(input_df: pd.DataFrame, input_path: Path, baseline_name: str, baseline_k: int, centrality: str) -> pd.DataFrame:
    """
    """
    
    # initialize a new df with a copy of the input summary df
    complete_df = input_df.copy()

    # remove the rows related to the baseline, as we do not have to compute errors for it
    input_df = input_df[input_df["method"] != baseline_name]

    # iterate through the rows of the input dataframe with the metadata, corresponding to single experiments
    for idx, exp_metadata in input_df.iterrows():

        # retreieve the path to the centralities computed in the current experiment
        centralities_path = \
            input_path / \
            exp_metadata["method"] / \
            (f"query_size_{exp_metadata['query_size']}" if exp_metadata["query_size"] != -1 else "query_size_all") / \
            f"threads_{exp_metadata['n_threads']}" / \
            f"k_{exp_metadata['k']}"
        if exp_metadata["method"] != "mc":    
            centralities_path = centralities_path / f"l_{int(exp_metadata['l'])}"
            if exp_metadata["method"] == "ew":
                if centrality == "lin":
                    centralities_path = centralities_path / f"c_{int(exp_metadata['c'])}"
            elif exp_metadata["method"] == "pps":
                centralities_path = centralities_path / f"delta_{exp_metadata['delta']:.3f}"
        centralities_path = centralities_path / f"rep_{exp_metadata['exp_rep']}" / "centralities.tsv"

        # retrieve the path to the baseline centralities for the loaded experiment
        baseline_path = \
            input_path / \
            baseline_name / \
            (f"query_size_{exp_metadata['query_size']}" if exp_metadata["query_size"] != -1 else "query_size_all") / \
            f"threads_{exp_metadata['n_threads']}" / \
            f"k_{baseline_k}" / \
            f"rep_{exp_metadata['exp_rep']}" / "centralities.tsv"

        # load approximate and baseline centralities
        approx_centralities = pd.read_csv(centralities_path, sep="\t")
        baseline_centralities = pd.read_csv(baseline_path, sep="\t")

        # compute the MAE and add it to the complete df
        merged = approx_centralities.merge(baseline_centralities, on="node_id", suffixes=("_approx", "_baseline"))
        mae = np.abs(merged["centrality_approx"] - merged["centrality_baseline"]).mean()
        mask = complete_df[
            (complete_df["method"] == exp_metadata["method"]) &
            (complete_df["query_size"] == exp_metadata["query_size"]) &
            (complete_df["n_threads"] == exp_metadata["n_threads"]) &
            (complete_df["k"] == exp_metadata["k"]) &
            (complete_df["exp_rep"] == exp_metadata["exp_rep"])
        ]
        if exp_metadata["method"] != "mc":    
            mask = mask[mask["l"] == exp_metadata["l"]]
            if exp_metadata["method"] == "ew":
                if centrality == "lin":
                    mask = mask[mask["c"] == exp_metadata["c"]]
            elif exp_metadata["method"] == "pps":
                mask = mask[mask["delta"] == exp_metadata["delta"]]
        complete_df.loc[mask.index, "mae"] = mae

    return complete_df

def estimate_exact_stats(input_df: pd.DataFrame, input_path: Path, baseline_name: str, baseline_k: int, query_size: int, n_threads: int) -> pd.DataFrame:
    """
    """

    # extract only the data related to experiments with the baseline method
    baseline_df = input_df[input_df["method"] == baseline_name]

    # path to the directory with different experiment repetitions with the baseline method
    base_path = input_path / baseline_name / (f"query_size_{query_size}" if query_size != -1 else "query_size_all") / f"threads_{n_threads}" / f"k_{baseline_k}"

    # dictionary of statistics to compute
    lists_stats = {
        "min": [],
        "max": [],
        "median": [],
        "mean": [],
        "std": []
    }

    # iterate through the experiments repetitions to compute statistics
    for exp_rep in baseline_df["exp_rep"].unique():

        # load the centralities computed in the current experiment repetition
        exp_rep_path = base_path / f"rep_{exp_rep}" / "centralities.tsv"
        exp_rep_df = pd.read_csv(exp_rep_path, sep="\t")

        # compute the statistics and add them to the corresponding lists
        lists_stats["min"].append(exp_rep_df["centrality"].min())
        lists_stats["max"].append(exp_rep_df["centrality"].max())
        lists_stats["median"].append(exp_rep_df["centrality"].median())
        lists_stats["mean"].append(exp_rep_df["centrality"].mean())
        lists_stats["std"].append(exp_rep_df["centrality"].std())

    # compute mean and std of each computed statistic
    statistics = {}
    for stat, values in lists_stats.items():
        statistics[f"{stat}_mean"] = np.mean(values)
        statistics[f"{stat}_std"] = np.std(values)

    return pd.DataFrame([statistics])

if __name__ == "__main__":

    # parse command line arguments
    args = parse_args()

    # default parameters
    baseline_k = 500
    baseline_name = "mc_baseline"
    metadata_df_name = "summary.tsv"

    # load all metadata files across all experiments into a single dataframe
    input_path = Path(args.input_dir)
    original_df = pd.concat([pd.read_csv(f, sep="\t") for f in input_path.rglob(metadata_df_name)], ignore_index=True, join="outer")

    # create the output directory if it does not exist
    os.makedirs(args.output_dir, exist_ok=True)

    # for each experiment with loaded metadata, compute some statistics and save it
    summaries_df = update_metadata_with_mae(original_df, input_path, baseline_name, baseline_k, args.measure)
    summaries_df.insert(0, "centrality", args.measure)
    summaries_df.insert(0, "dataset", args.dataset)
    summaries_df.to_csv(Path(args.output_dir) / "summary.tsv", sep="\t", index=False)

    # compute min, max, mean and median centralities computed by the baseline exact approach
    query_size = -1
    n_threads = 16
    stats_df = estimate_exact_stats(original_df, input_path, baseline_name, baseline_k, query_size, n_threads)
    stats_df.to_csv(Path(args.output_dir) / "exact_stats.tsv", sep="\t", index=False)
