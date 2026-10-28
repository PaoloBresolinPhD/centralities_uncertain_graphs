import os
from pathlib import Path
import argparse
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import pandas as pd
import seaborn as sns
from matplotlib.lines import Line2D
from matplotlib.colors import to_rgba

def apply_large_fonts():
    """
    Helper function to apply large font sizes consistently across all plots.
    """

    # set the font sizes of title, axes labels and axes values
    plt.title(plt.gca().get_title(), fontsize=16)
    plt.xlabel(plt.gca().get_xlabel(), fontsize=16)
    plt.ylabel(plt.gca().get_ylabel(), fontsize=16)
    plt.tick_params(axis="both", which="both", labelsize=14)
    
    # set the font sizes for the legend
    legend = plt.gca().get_legend()
    if legend:
        plt.setp(legend.get_texts(), fontsize=14)
        plt.setp(legend.get_title(), fontsize=16)

def time_error_plot(plot_df, centrality, palette, markers, methods_order, datasets_order, output_path):
    """
    """

    # close previous plots
    plt.close()

    # set theme and style
    sns.set_theme("paper", font_scale=1.0)
    sns.set_style("whitegrid")

    # create the figure
    fig, ax = plt.subplots()

    # iterate through the rows of the input df
    for _, row in plot_df.iterrows():

        # extract mean and std of times and errors
        t, t_std = row["Time"], row["Time_std"]
        e, e_std = row["MAE"], row["MAE_std"]


        # clip the lower bars: time must stay > 0 on a log axis, MAE can't be < 0
        x_lower = min(t_std, t * 0.999)
        y_lower = min(e_std, e)

        # points: semi-transparent fill, opaque edge so the dataset shape stays readable
        ax.plot(
            t, e,
            marker=markers[row["Dataset"]], linestyle="none",
            markerfacecolor=to_rgba(palette[row["Method"]], 0.5),
            markeredgecolor=palette[row["Method"]],
            markersize=12 if markers[row["Dataset"]] != "*" else 12 * 1.3,
            markeredgewidth=0,
            zorder=2,
        )

        # error bars on top of the points
        ax.errorbar(
            t, e,
            xerr=[[x_lower], [t_std]],
            yerr=[[y_lower], [e_std]],
            fmt="none",
            ecolor=palette[row["Method"]],
            elinewidth=1, capsize=3,
            zorder=3,
        )

    # set scales and labels of the axes
    ax.set_xscale("log")
    ax.set_xlabel("Time (s)")
    ax.set_ylabel("Error (MAE)")
 
    # show only powers of 10 in the x axis and make it start from the desired value based on the centrality
    x_min = 1e-2 if centrality == "harmonic" else 1e-1
    ax.set_xlim(left=x_min)
    ax.xaxis.set_major_locator(ticker.LogLocator(base=10, numticks=20))
    ax.xaxis.set_major_formatter(ticker.LogFormatterMathtext(base=10))
    ax.xaxis.set_minor_formatter(ticker.NullFormatter())
    
    # format the values in the y axis to 3 decimals and make it start from 0
    ax.yaxis.set_major_formatter(ticker.FormatStrFormatter("%.3f"))
    ax.set_ylim(bottom=0)
    ax.yaxis.set_major_locator(ticker.MultipleLocator(0.001))
 
    # enlarge fonts
    apply_large_fonts()
 
    # create a legend with methods
    method_handles = [Line2D([], [], marker="o", linestyle="none", color=palette[m], label=m, markersize=12, markeredgewidth=0) for m in methods_order]
    method_legend = ax.legend(
        handles=method_handles, title="Method",
        loc="lower center", bbox_to_anchor=(0.5, 1.02), ncol=len(methods_order), frameon=False,
        title_fontsize=16,
        fontsize=14
    )
    ax.add_artist(method_legend)

    # create a legend with datasets
    dataset_handles = [Line2D([], [], marker=markers[d], linestyle="none", color="lightgray", label=d, markersize=12 if markers[d] != "*" else 12 * 1.3, markeredgewidth=0.5) for d in datasets_order]
    dataset_legend = ax.legend(
        handles=dataset_handles, title="Dataset",
        loc="upper left", bbox_to_anchor=(1.02, 1.0), borderaxespad=0,
        title_fontsize=16,
        fontsize=14
    )
 
    # save the plot
    plt.savefig(output_path, bbox_inches="tight", bbox_extra_artists=[method_legend, dataset_legend])

def parse_args():

    # define the parser
    parser = argparse.ArgumentParser()

    # add the arguments
    parser.add_argument("-i", "--input_dir", required=True, type=str, help="Path to the directory with all produced results for all datasets.")
    parser.add_argument("-o", "--output_dir", required=True, type=str, help="Path to the directory where to store the plots.")

    return parser.parse_args()

if __name__ == "__main__":

    # constants
    n_threads = 16
    k = 100
    l = 100
    c = 5
    delta = 0.01

    # parse command line arguments
    args = parse_args()

    # load the summary results across all experiments into a single dataframe
    summaries_df = pd.concat([pd.read_csv(f, sep="\t") for f in Path(args.input_dir).rglob("summary.tsv")], ignore_index=True, join="outer")

    # map some column names
    summaries_df = summaries_df.rename(
        columns =
            {
                "method": "Method",
                "time": "Time",
                "mae": "MAE",
                "dataset": "Dataset",
                "centrality": "Centrality"
            }
    )

    # map the names of the datasets
    summaries_df["Dataset"] = summaries_df["Dataset"].map(
        {
            "miner": "ChG-Miner",
            "slashdot": "SlashDot",
            "notredame": "Web-NotreDame",
            "google": "Web-Google",
            "berkstan": "Web-BerkStan",
            "talk": "Wiki-Talk"
        }
    )

    # split the df into a df for lin and one for harmonic
    lin_df = summaries_df[summaries_df["Centrality"] == "lin"]
    harmonic_df = summaries_df[summaries_df["Centrality"] == "harmonic"]

    # filter the dataframes to keep only what will be used
    columns_to_keep = ["Method", "Dataset", "exp_rep", "Time", "MAE"]
    lin_df = lin_df[
        (lin_df["n_threads"] == n_threads) &
        (lin_df["k"] == k) & (
            (lin_df["Method"] == "mc") |
            ((lin_df["Method"] == "ew") & (lin_df["l"] == l) & (lin_df["c"] == c)) |
            ((lin_df["Method"] == "pps") & (lin_df["l"] == l) & (lin_df["delta"] == delta))
        )
    ][columns_to_keep]
    harmonic_df = harmonic_df[
        (harmonic_df["n_threads"] == n_threads) &
        (harmonic_df["k"] == k) & (
            (harmonic_df["Method"] == "mc") |
            ((harmonic_df["Method"] == "ew") & (harmonic_df["l"] == l)) |
            ((harmonic_df["Method"] == "pps") & (harmonic_df["l"] == l) & (harmonic_df["delta"] == delta))
        )
    ][columns_to_keep]

    # map the names of the methods
    lin_df["Method"] = lin_df["Method"].map(
        {
            "mc_baseline": "Baseline",
            "mc": "Exact",
            "ew": "LinEW",
            "pps": "LinPPS"
        }
    )
    harmonic_df["Method"] = harmonic_df["Method"].map(
        {
            "mc_baseline": "Baseline",
            "mc": "Exact",
            "ew": "HarmonicEW",
            "pps": "HarmonicPPS"
        }
    )

    # compute mean and std of times and errors across experiment repetitions
    lin_plot_df = lin_df.groupby(["Dataset", "Method"]).agg(
        Time = ("Time", "mean"),
        Time_std = ("Time", "std"),
        MAE = ("MAE", "mean"),
        MAE_std = ("MAE", "std")
    ).reset_index()
    
    harmonic_plot_df = harmonic_df.groupby(["Dataset", "Method"]).agg(
            Time = ("Time", "mean"),
            Time_std = ("Time", "std"),
            MAE = ("MAE", "mean"),
            MAE_std = ("MAE", "std")
    ).reset_index()

    # assign colors to methods
    colors = sns.color_palette("colorblind")
    palette = {
        "Baseline": colors[3],
        "Exact": colors[0],
        "LinEW": colors[1],
        "HarmonicEW": colors[1],
        "LinPPS": colors[2],
        "HarmonicPPS": colors[2]
    }

    # assign markers to datasets
    markers = {
        "ChG-Miner": "^",
        "SlashDot": "s",
        "Web-NotreDame": "d",
        "Web-Google": "P",
        "Web-BerkStan": "*",
        "Wiki-Talk": "H"
    }

    # create ordered lists of methods and datasets for the plot legend
    methods_order_lin = ["Exact", "LinEW", "LinPPS"]
    methods_order_harmonic = ["Exact", "HarmonicEW", "HarmonicPPS"]
    datasets_order = ["ChG-Miner", "SlashDot", "Web-NotreDame", "Web-Google", "Web-BerkStan", "Wiki-Talk"]

    # create and save the plots
    os.makedirs(args.output_dir, exist_ok=True)
    time_error_plot(lin_plot_df, "lin", palette, markers, methods_order_lin, datasets_order, Path(args.output_dir) / "lin_summary_plot.pdf")
    time_error_plot(harmonic_plot_df, "harmonic", palette, markers, methods_order_harmonic, datasets_order, Path(args.output_dir) / "harmonic_summary_plot.pdf")
