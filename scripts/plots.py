import os
from pathlib import Path
import argparse
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import seaborn as sns

def apply_large_fonts():
    """
    Helper function to apply large font sizes consistently across all plots.
    """

    # set the font sizes of title, axes labels and axes values
    plt.title(plt.gca().get_title(), fontsize=18)
    plt.xlabel(plt.gca().get_xlabel(), fontsize=18)
    plt.ylabel(plt.gca().get_ylabel(), fontsize=18)
    plt.tick_params(axis="both", which="both", labelsize=16)
    
    # set the font sizes for the legend
    legend = plt.gca().get_legend()
    if legend:
        plt.setp(legend.get_texts(), fontsize=16)
        plt.setp(legend.get_title(), fontsize=18)

def plot_times_threads(input_df, measure, query_size, palette, k, k_baseline, l, c, delta, output_path):

    # extract only the required data
    columns_to_keep = ["Method", "time", "n_threads"]
    mc_baseline_df = input_df[(input_df["Method"] == "Baseline") & (input_df["k"] == k_baseline) & (input_df["query_size"] == query_size)][columns_to_keep]
    mc_df = input_df[(input_df["Method"] == "Exact") & (input_df["k"] == k) & (input_df["query_size"] == query_size)][columns_to_keep]
    if measure == "lin":
        ew_df = input_df[(input_df["Method"] == "LinEW") & (input_df["k"] == k) & (input_df["l"] == l) & (input_df["c"] == c) & (input_df["query_size"] == query_size)][columns_to_keep]
    else:
        ew_df = input_df[(input_df["Method"] == "HarmonicEW") & (input_df["k"] == k) & (input_df["l"] == l) & (input_df["query_size"] == query_size)][columns_to_keep]
    pps_df = input_df[((input_df["Method"] == "LinPPS") | (input_df["Method"] == "HarmonicPPS")) & (input_df["k"] == k) & (input_df["l"] == l) & (input_df["delta"] == delta) & (input_df["query_size"] == query_size)][columns_to_keep]
    plot_df = pd.concat([mc_baseline_df, mc_df, ew_df, pps_df], ignore_index=True)

    # plot the dataframe
    plt.close()

    sns.set_theme("paper", font_scale=1.0)
    sns.set_style("whitegrid")

    sns.barplot(data=plot_df, x="n_threads", y="time", hue="Method", palette=palette)

    # plt.title("Running Time Over Threads")
    plt.xlabel("Number of Threads")
    plt.ylabel("Time (s)")
    plt.yscale("log")

    apply_large_fonts()
    
    # save the plot
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    plt.savefig(output_path, bbox_inches="tight")

def plot_times_k(input_df, measure, query_size, palette, n_threads, l, c, delta, output_path):

    # extract only the required data
    columns_to_keep = ["Method", "time", "k"]
    mc_df = input_df[(input_df["Method"] == "mc") & (input_df["n_threads"] == n_threads) & (input_df["query_size"] == query_size)][columns_to_keep]
    if measure == "lin":
        ew_df = input_df[(input_df["Method"] == "LinEW") & (input_df["n_threads"] == n_threads) & (input_df["l"] == l) & (input_df["c"] == c) & (input_df["query_size"] == query_size)][columns_to_keep]
    else:
        ew_df = input_df[(input_df["Method"] == "HarmonicEW") & (input_df["n_threads"] == n_threads) & (input_df["l"] == l) & (input_df["query_size"] == query_size)][columns_to_keep]
    pps_df = input_df[((input_df["Method"] == "LinPPS") | (input_df["Method"] == "HarmonicPPS")) & (input_df["n_threads"] == n_threads) & (input_df["l"] == l) & (input_df["delta"] == delta) & (input_df["query_size"] == query_size)][columns_to_keep]
    plot_df = pd.concat([mc_df, ew_df, pps_df], ignore_index=True)

    # plot the dataframe
    plt.close()

    sns.set_theme("paper", font_scale=1.0)
    sns.set_style("whitegrid")

    sns.barplot(data=plot_df, x="k", y="time", hue="Method", palette=palette)

    # plt.title("Running Time Over k")
    plt.xlabel("Number of Sampled Possible Worlds (k)")
    plt.ylabel("Time (s)")
    plt.yscale("log")

    apply_large_fonts()
    
    # save the plot
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    plt.savefig(output_path, bbox_inches="tight")

def plot_times_l(input_df, measure, query_size, palette, k, n_threads, c, delta, output_path):

    # extract only the required data
    columns_to_keep = ["Method", "time", "l"]
    if measure == "lin":
        ew_df = input_df[(input_df["Method"] == "LinEW") & (input_df["k"] == k) & (input_df["n_threads"] == n_threads) & (input_df["c"] == c) & (input_df["query_size"] == query_size)][columns_to_keep]
    else:
        ew_df = input_df[(input_df["Method"] == "HarmonicEW") & (input_df["k"] == k) & (input_df["n_threads"] == n_threads) & (input_df["query_size"] == query_size)][columns_to_keep]
    pps_df = input_df[((input_df["Method"] == "LinPPS") | (input_df["Method"] == "HarmonicPPS")) & (input_df["k"] == k) & (input_df["n_threads"] == n_threads) & (input_df["delta"] == delta) & (input_df["query_size"] == query_size)][columns_to_keep]
    plot_df = pd.concat([ew_df, pps_df], ignore_index=True)
    plot_df["l"] = plot_df["l"].astype(int)

    # plot the dataframe
    plt.close()

    sns.set_theme("paper", font_scale=1.0)
    sns.set_style("whitegrid")

    sns.barplot(data=plot_df, x="l", y="time", hue="Method", palette=palette)

    # plt.title("Running Time Over l")
    plt.xlabel("Number of Sampled Nodes (l)")
    plt.ylabel("Time (s)")
    plt.yscale("log")

    apply_large_fonts()

    # keep only powers of 10 on the y axis, inserting at least 3 values
    ax = plt.gca()
    ymin, ymax = plot_df["time"].min(), plot_df["time"].max()
    lo = 10 ** np.floor(np.log10(ymin))
    hi = 10 ** np.ceil(np.log10(ymax))
    if hi / lo < 100:
        lo /= 10
        hi *= 10 if hi / lo < 100 else 1
    ax.set_ylim(lo, hi)
    ax.yaxis.set_major_locator(ticker.LogLocator(base=10, numticks=20))
    ax.yaxis.set_major_formatter(ticker.LogFormatterMathtext(base=10))
    ax.yaxis.set_minor_formatter(ticker.NullFormatter())
    
    # save the plot
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    plt.savefig(output_path, bbox_inches="tight")

def plot_errors_k(input_df, measure, query_size, palette, n_threads, l, c, delta, output_path):

    # extract only the required data
    columns_to_keep = ["Method", "k", "mae"]
    mc_df = input_df[(input_df["Method"] == "mc") & (input_df["n_threads"] == n_threads) & (input_df["query_size"] == query_size)][columns_to_keep]
    if measure == "lin":
        ew_df = input_df[(input_df["Method"] == "LinEW") & (input_df["n_threads"] == n_threads) & (input_df["l"] == l) & (input_df["c"] == c) & (input_df["query_size"] == query_size)][columns_to_keep]
    else:
        ew_df = input_df[(input_df["Method"] == "HarmonicEW") & (input_df["n_threads"] == n_threads) & (input_df["l"] == l) & (input_df["query_size"] == query_size)][columns_to_keep]
    pps_df = input_df[((input_df["Method"] == "LinPPS") | (input_df["Method"] == "HarmonicPPS")) & (input_df["n_threads"] == n_threads) & (input_df["l"] == l) & (input_df["delta"] == delta) & (input_df["query_size"] == query_size)][columns_to_keep]
    plot_df = pd.concat([mc_df, ew_df, pps_df], ignore_index=True)

    # plot the dataframe
    plt.close()

    sns.set_theme("paper", font_scale=1.0)
    sns.set_style("whitegrid")

    sns.boxplot(data=plot_df, x="k", y="mae", hue="Method", palette=palette)

    # plt.title("Error Over k")
    plt.xlabel("k")
    plt.ylabel("Error (MAE)")

    apply_large_fonts()
    plt.gca().yaxis.set_major_formatter(ticker.FormatStrFormatter("%.3f"))
    
    # save the plot
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    plt.savefig(output_path, bbox_inches="tight")

def plot_errors_l(input_df, measure, query_size, palette, n_threads, k, c, delta, output_path):

    # extract only the required data
    columns_to_keep = ["Method", "l", "mae"]
    if measure == "lin":
        ew_df = input_df[(input_df["Method"] == "LinEW") & (input_df["n_threads"] == n_threads) & (input_df["k"] == k) & (input_df["c"] == c) & (input_df["query_size"] == query_size)][columns_to_keep]
    else:
        ew_df = input_df[(input_df["Method"] == "HarmonicEW") & (input_df["n_threads"] == n_threads) & (input_df["k"] == k) & (input_df["query_size"] == query_size)][columns_to_keep]
    pps_df = input_df[((input_df["Method"] == "LinPPS") | (input_df["Method"] == "HarmonicPPS")) & (input_df["n_threads"] == n_threads) & (input_df["k"] == k) & (input_df["delta"] == delta) & (input_df["query_size"] == query_size)][columns_to_keep]
    plot_df = pd.concat([ew_df, pps_df], ignore_index=True)
    plot_df["l"] = plot_df["l"].astype(int)

    # plot the dataframe
    plt.close()

    sns.set_theme("paper", font_scale=1.0)
    sns.set_style("whitegrid")

    sns.boxplot(data=plot_df, x="l", y="mae", hue="Method", palette=palette)

    # plt.title("Error Over l")
    plt.xlabel("l")
    plt.ylabel("Error (MAE)")

    apply_large_fonts()
    plt.gca().yaxis.set_major_formatter(ticker.FormatStrFormatter("%.3f"))
    
    # save the plot
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    plt.savefig(output_path, bbox_inches="tight")

def parse_args():

    # define the parser
    parser = argparse.ArgumentParser()

    # add the arguments
    parser.add_argument("-i", "--input_dir", required=True, type=str, help="Path to the directory with all produced results to load.")
    parser.add_argument("-o", "--output_dir", required=True, type=str, help="Path to the directory where to store the plots.")
    parser.add_argument("-m", "--measure", required=True, type=str, help="Centrality measure to consider. Can be either 'lin' or 'harmonic'.")

    return parser.parse_args()

if __name__ == "__main__":

    # parse command line arguments
    args = parse_args()

    # load the summary results across all experiments into a single dataframe
    input_path = Path(args.input_dir)
    summaries_df = pd.concat([pd.read_csv(f, sep="\t") for f in input_path.rglob("summary.tsv")], ignore_index=True, join="outer")

    # rename the names of the methods for better plotting visualization
    summaries_df["method"] = summaries_df["method"].map(
        {
            "mc_baseline": "Baseline",
            "mc": "Exact",
            "ew": "LinEW" if args.measure == "lin" else "HarmonicEW",
            "pps": "LinPPS" if args.measure == "lin" else "HarmonicPPS"
        }
    )
    summaries_df = summaries_df.rename(columns={"method": "Method"})

    # create a color the same color palette for all methods
    colors = sns.color_palette("colorblind")
    palette = {
        "Baseline": colors[3],
        "Exact": colors[0],
        "LinEW": colors[1],
        "HarmonicEW": colors[1],
        "LinPPS": colors[2],
        "HarmonicPPS": colors[2]
    }

    
    # plot how the running time changes as the number of threads changes, for fixed other values
    query_size = 1000
    k = 100
    k_baseline = 500
    l = 100
    c = 5
    delta = 0.01
    plot_times_threads(summaries_df, measure=args.measure, query_size=query_size, palette=palette, k=k, k_baseline=k_baseline, l=l, c=c, delta=delta, output_path=os.path.join(args.output_dir, f"times_over_threads__q_{query_size}_k_{k}_k_baseline_{k_baseline}_l_{l}_c_{c}_delta_{delta}.pdf"))

    # plot how the running time changes as k changes, for fixed other values
    n_threads = 16
    plot_times_k(summaries_df, measure=args.measure, query_size=query_size, palette=palette, n_threads=n_threads, l=l, c=c, delta=delta, output_path=os.path.join(args.output_dir, f"times_over_k__q_{query_size}_threads_{n_threads}_l_{l}_c_{c}_delta_{delta}.pdf"))

    # plot how the running time changes as l changes, for fixed other values
    plot_times_l(summaries_df, measure=args.measure, query_size=query_size, palette=palette, k=k, n_threads=n_threads, c=c, delta=delta, output_path=os.path.join(args.output_dir, f"times_over_l__q_{query_size}_k_{k}_n_threads_{n_threads}_c_{c}_delta_{delta}.pdf"))

    # plot how the error changes as k changes, for fixed other values
    plot_errors_k(summaries_df, measure=args.measure, query_size=query_size, palette=palette, n_threads=n_threads, l=l, c=c, delta=delta, output_path=os.path.join(args.output_dir, f"errors_over_k__q_{query_size}_threads_{n_threads}_l_{l}_c_{c}_delta_{delta}.pdf"))

    # plot how the error changes as l changes, for fixed other values
    plot_errors_l(summaries_df, measure=args.measure, query_size=query_size, palette=palette, n_threads=n_threads, k=k, c=c, delta=delta, output_path=os.path.join(args.output_dir, f"errors_over_l__q_{query_size}_threads_{n_threads}_k_{k}_c_{c}_delta_{delta}.pdf"))