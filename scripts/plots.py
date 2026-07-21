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
    parser.add_argument("-o", "--output_dir", required=True, type=str, help="Path to the directory where to store the plots.")

    return parser.parse_args()

def plot_times_threads(input_df, k, k_baseline, l, c, delta, output_path):

    # extract only the required data
    columns_to_keep = ["method", "time", "n_threads"]
    mc_baseline_df = input_df[(input_df["method"] == "mc_baseline") & (input_df["k"] == k_baseline)][columns_to_keep]
    mc_df = input_df[(input_df["method"] == "mc") & (input_df["k"] == k)][columns_to_keep]
    ew_df = input_df[(input_df["method"] == "ew") & (input_df["k"] == k) & (input_df["l"] == l) & (input_df["c"] == c)][columns_to_keep]
    pps_df = input_df[(input_df["method"] == "pps") & (input_df["k"] == k) & (input_df["l"] == l) & (input_df["delta"] == delta)][columns_to_keep]
    plot_df = pd.concat([mc_baseline_df, mc_df, ew_df, pps_df], ignore_index=True)

    # plot the dataframe
    plt.close()

    sns.set_theme("paper")

    sns.barplot(data=plot_df, x="n_threads", y="time", hue="method", palette=sns.color_palette("colorblind"))

    plt.title("Running Time Over Threads")
    plt.xlabel("Number of Threads")
    plt.ylabel("Time (s)")
    
    # save the plot
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    plt.savefig(output_path)

def plot_times_k(input_df, n_threads, l, c, delta, output_path):

    # extract only the required data
    columns_to_keep = ["method", "time", "k"]
    mc_df = input_df[(input_df["method"] == "mc") & (input_df["n_threads"] == n_threads)][columns_to_keep]
    ew_df = input_df[(input_df["method"] == "ew") & (input_df["n_threads"] == n_threads) & (input_df["l"] == l) & (input_df["c"] == c)][columns_to_keep]
    pps_df = input_df[(input_df["method"] == "pps") & (input_df["n_threads"] == n_threads) & (input_df["l"] == l) & (input_df["delta"] == delta)][columns_to_keep]
    plot_df = pd.concat([mc_df, ew_df, pps_df], ignore_index=True)

    # plot the dataframe
    plt.close()

    sns.set_theme("paper")

    sns.barplot(data=plot_df, x="k", y="time", hue="method", palette=sns.color_palette("colorblind"))

    plt.title("Running Time Over k")
    plt.xlabel("Number of Sampled Possible Worlds (k)")
    plt.ylabel("Time (s)")
    
    # save the plot
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    plt.savefig(output_path)

def plot_times_l(input_df, k, n_threads, c, delta, output_path):

    # extract only the required data
    columns_to_keep = ["method", "time", "l"]
    ew_df = input_df[(input_df["method"] == "ew") & (input_df["k"] == k) & (input_df["n_threads"] == n_threads) & (input_df["c"] == c)][columns_to_keep]
    pps_df = input_df[(input_df["method"] == "pps") & (input_df["k"] == k) & (input_df["n_threads"] == n_threads) & (input_df["delta"] == delta)][columns_to_keep]
    plot_df = pd.concat([ew_df, pps_df], ignore_index=True)

    # plot the dataframe
    plt.close()

    sns.set_theme("paper")

    sns.barplot(data=plot_df, x="l", y="time", hue="method", palette=sns.color_palette("colorblind"))

    plt.title("Running Time Over l")
    plt.xlabel("Number of Sampled Nodes (l)")
    plt.ylabel("Time (s)")
    
    # save the plot
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    plt.savefig(output_path)

def plot_errors_k(input_df, n_threads, l, c, delta, output_path):

    # extract only the required data
    columns_to_keep = ["method", "k", "avg_error"]
    mc_df = input_df[(input_df["method"] == "mc") & (input_df["n_threads"] == n_threads)][columns_to_keep]
    ew_df = input_df[(input_df["method"] == "ew") & (input_df["n_threads"] == n_threads) & (input_df["l"] == l) & (input_df["c"] == c)][columns_to_keep]
    pps_df = input_df[(input_df["method"] == "pps") & (input_df["n_threads"] == n_threads) & (input_df["l"] == l) & (input_df["delta"] == delta)][columns_to_keep]
    plot_df = pd.concat([mc_df, ew_df, pps_df], ignore_index=True)

    # plot the dataframe
    plt.close()

    sns.set_theme("paper")

    sns.boxplot(data=plot_df, x="method", y="avg_error", hue="k", palette=sns.color_palette("colorblind"))

    plt.title("Error Over k")
    plt.xlabel("Method")
    plt.ylabel("Error (MAE)")
    
    # save the plot
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    plt.savefig(output_path)

def plot_errors_l(input_df, n_threads, k, c, delta, output_path):

    # extract only the required data
    columns_to_keep = ["method", "l", "avg_error"]
    ew_df = input_df[(input_df["method"] == "ew") & (input_df["n_threads"] == n_threads) & (input_df["k"] == k) & (input_df["c"] == c)][columns_to_keep]
    pps_df = input_df[(input_df["method"] == "pps") & (input_df["n_threads"] == n_threads) & (input_df["k"] == k) & (input_df["delta"] == delta)][columns_to_keep]
    plot_df = pd.concat([ew_df, pps_df], ignore_index=True)

    # plot the dataframe
    plt.close()

    sns.set_theme("paper")

    sns.boxplot(data=plot_df, x="method", y="avg_error", hue="l", palette=sns.color_palette("colorblind"))

    plt.title("Error Over l")
    plt.xlabel("Method")
    plt.ylabel("Error (MAE)")
    
    # save the plot
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    plt.savefig(output_path)

if __name__ == "__main__":

    # parse command line arguments
    args = parse_args()

    # load the summary results across all experiments into a single dataframe
    input_path = Path(args.input_dir)
    summaries_df = pd.concat([pd.read_csv(f, sep="\t") for f in input_path.rglob("summary.tsv")], ignore_index=True, join="outer")
    
    # plot how the running time changes as the number of threads changes, for fixed other values
    k = 25
    k_baseline = 500
    l = 25
    c = 5
    delta = 0.01
    plot_times_threads(summaries_df, k=k, k_baseline=k_baseline, l=l, c=c, delta=delta, output_path=os.path.join(args.output_dir, f"times_over_threads__k_{k}_k_baseline_{k_baseline}_l_{l}_c_{c}_delta{delta}.pdf"))

    # plot how the running time changes as k changes, for fixed other values
    n_threads = 16
    plot_times_k(summaries_df, n_threads=n_threads, l=l, c=c, delta=delta, output_path=os.path.join(args.output_dir, f"times_over_k__threads_{n_threads}_l_{l}_c_{c}_delta{delta}.pdf"))

    # plot how the running time changes as l changes, for fixed other values
    plot_times_l(summaries_df, k=k, n_threads=n_threads, c=c, delta=delta, output_path=os.path.join(args.output_dir, f"times_over_l__k_{k}_n_threads_{n_threads}_c_{c}_delta{delta}.pdf"))

    # plot how the error changes as k changes, for fixed other values
    plot_errors_k(summaries_df, n_threads=n_threads, l=l, c=c, delta=delta, output_path=os.path.join(args.output_dir, f"errors_over_k__threads_{n_threads}_l_{l}_c_{c}_delta{delta}.pdf"))

    # plot how the error changes as l changes, for fixed other values
    plot_errors_l(summaries_df, n_threads=n_threads, k=k, c=c, delta=delta, output_path=os.path.join(args.output_dir, f"errors_over_l__threads_{n_threads}_k_{k}_c_{c}_delta{delta}.pdf"))