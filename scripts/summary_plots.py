import os
from pathlib import Path
import argparse
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
from matplotlib.patches import Patch
import pandas as pd
import numpy as np
import seaborn as sns
from matplotlib.lines import Line2D
from matplotlib.colors import to_rgba

# ----------------------------------------------------------------------
# Layout of the VLDB template (acmart.cls, sigconf, 9pt)
# ----------------------------------------------------------------------
# \columnwidth = 241.15pt, \textwidth = 506.30pt (1in = 72.27pt)
COLUMN_WIDTH = 241.15 / 72.27   # ~3.34in -> \includegraphics[width=\columnwidth]
TEXT_WIDTH = 506.30 / 72.27     # ~7.01in -> figure* with \includegraphics[width=\textwidth]

# font sizes (pt): captions in the template are 8pt, body text 9pt
LABEL_SIZE = 8
TICK_SIZE = 7
LEGEND_SIZE = 7

# marker size (pt); stars look smaller than other markers, so they are enlarged
MARKER_SIZE = 5
STAR_SCALE = 1.3

# set to False if LaTeX (with the libertine and newtx packages) is not installed
USE_TEX = True


def set_paper_style(use_tex=USE_TEX):
    """
    Seaborn whitegrid style with fonts and sizes matching the VLDB template.
    Must be called instead of sns.set_theme / sns.set_style, since those reset the fonts.
    """

    rc = {
        # fonts: Linux Libertine as in acmart.cls
        "font.family": "serif",
        "font.serif": ["Linux Libertine O", "Libertinus Serif", "Linux Libertine", "DejaVu Serif"],
        "font.size": LABEL_SIZE,
        "axes.titlesize": LABEL_SIZE,
        "axes.labelsize": LABEL_SIZE,
        "xtick.labelsize": TICK_SIZE,
        "ytick.labelsize": TICK_SIZE,
        "legend.fontsize": LEGEND_SIZE,
        "legend.title_fontsize": LABEL_SIZE,
        "figure.titlesize": LABEL_SIZE,
        "figure.labelsize": LABEL_SIZE,

        # thinner lines, suited to small figures
        "axes.linewidth": 0.6,
        "grid.linewidth": 0.4,
        "lines.linewidth": 0.8,
        "xtick.major.width": 0.6,
        "ytick.major.width": 0.6,
        "patch.linewidth": 0.5,

        # embed TrueType fonts instead of Type 3 (rejected by many submission checkers)
        "pdf.fonttype": 42,
        "ps.fonttype": 42,
    }

    if use_tex:
        # render all text with LaTeX, using the same packages as acmart.cls (pdflatex branch)
        rc["text.usetex"] = True
        rc["text.latex.preamble"] = (
            r"\usepackage[T1]{fontenc}"
            r"\usepackage[tt=false, type1=true]{libertine}"
            r"\usepackage[libertine]{newtxmath}"
        )
    else:
        # approximation without LaTeX: use the serif font above for math too (e.g. 10^x ticks)
        rc["text.usetex"] = False
        rc["mathtext.fontset"] = "custom"
        rc["mathtext.rm"] = "serif"
        rc["mathtext.it"] = "serif:italic"
        rc["mathtext.bf"] = "serif:bold"

    sns.set_theme(context="paper", style="whitegrid", rc=rc)


def marker_size(marker):
    return MARKER_SIZE * STAR_SCALE if marker == "*" else MARKER_SIZE


def times_k_plot(plot_df, centrality, palette, n_threads, l, c, delta, methods_order, datasets_order, output_path, n_cols=3):
    """
    Creates a single figure with one bar subplot for each dataset across different values of k.
    Sized for a figure* spanning both columns.
    """
 
    # filter the dataframe by keeping only required data
    df = plot_df[(plot_df["Centrality"] == centrality) & (plot_df["n_threads"] == n_threads)]
    ew_mask = (df["Method"] == "ew") & (df["l"] == l)
    if centrality == "lin":
        ew_mask &= df["c"] == c
    pps_mask = (df["Method"] == "pps") & (df["l"] == l) & (df["delta"] == delta)
    df = df[(df["Method"] == "mc") | ew_mask | pps_mask][["Dataset", "Method", "k", "Time"]].copy()
 
    # map the names of the methods
    prefix = "Lin" if centrality == "lin" else "Harmonic"
    df["Method"] = df["Method"].map({"mc": "Exact", "ew": f"{prefix}EW", "pps": f"{prefix}PPS"})
    df["k"] = df["k"].astype(int)
    k_order = sorted(df["k"].unique())

    # close any previously opened subplot
    plt.close()

    # set theme, style and fonts
    set_paper_style()

    # compute the number of rows for the subplots
    n_rows = int(np.ceil(len(datasets_order) / n_cols))

    # initialize the figure at its final size in the paper (full text width)
    fig, axes = plt.subplots(n_rows, n_cols, figsize=(TEXT_WIDTH, 1.9 * n_rows + 0.6), squeeze=False, layout="constrained")

    # add some space among rows
    fig.get_layout_engine().set(hspace=0.06)

    # flatten the array with the axes of the subplots
    axes = axes.flatten()

    # iterate over the subplots and datasets
    for i, (ax, dataset) in enumerate(zip(axes, datasets_order)):
        
        # extract the current dataset
        dataset_df = df[df["Dataset"] == dataset]

        # create the barplot
        sns.barplot(
            data=dataset_df,
            x="k",
            y="Time",
            hue="Method",
            order=k_order,
            hue_order=methods_order,
            palette=palette,
            ax=ax,
            saturation=1,
            err_kws={"linewidth": 0.8},
        )

        # remove the legend: we want a single legend for all subplots on top of them
        if ax.get_legend() is not None:
            ax.get_legend().remove()

        # set title and axes labels (sizes come from set_paper_style)
        ax.set_title(dataset)
        ax.set_xlabel(f"({chr(ord('a') + i)})")
        ax.set_ylabel("")
 
        # set the y axis to log scale with only powers of 10, showing at least two powers of 10
        ax.set_yscale("log")
        if not dataset_df.empty:
            lo = 10 ** np.floor(np.log10(dataset_df["Time"].min()))
            hi = 10 ** np.ceil(np.log10(dataset_df["Time"].max() * 1.5))  # room for the error bars
            if hi / lo < 100:
                lo /= 10
            ax.set_ylim(lo, hi)
        ax.yaxis.set_major_locator(ticker.LogLocator(base=10, numticks=20))
        ax.yaxis.set_major_formatter(ticker.LogFormatterMathtext(base=10))
        ax.yaxis.set_minor_formatter(ticker.NullFormatter())
 
    # hide unused subplots if the grid is larger than the number of datasets
    for ax in axes[len(datasets_order):]:
        ax.set_visible(False)
 
    # set shared axes labels
    fig.supxlabel("Number of Sampled Possible Worlds (k)")
    fig.supylabel("Time (s)")
 
    # create a single, shared legend above all subplots ("outside" keeps it inside the figure size)
    handles = [Patch(facecolor=palette[m], label=m) for m in methods_order]
    fig.legend(handles=handles, title="Method", loc="outside upper center", ncol=len(methods_order), frameon=False)
 
    # save the plot: no bbox_inches="tight", so the PDF keeps exactly the size set above
    plt.savefig(output_path)


def time_error_plot(plot_df, centrality, palette, markers, methods_order, datasets_order, output_path):
    """
    Scatter of mean error vs mean running time for each (dataset, method) pair.
    Sized for a single column.
    """

    # close previous plots
    plt.close()

    # set theme, style and fonts
    set_paper_style()

    # create the figure at its final size in the paper (one column)
    fig, ax = plt.subplots(figsize=(COLUMN_WIDTH, 3.0), layout="constrained")

    # iterate through the rows of the input df
    for _, row in plot_df.iterrows():

        # extract mean and std of times and errors
        t, t_std = row["Time"], row["Time_std"]
        e, e_std = row["MAE"], row["MAE_std"]

        # clip the lower bars: time must stay > 0 on a log axis, MAE can't be < 0
        x_lower = min(t_std, t * 0.999)
        y_lower = min(e_std, e)

        # points: semi-transparent fill
        ax.plot(
            t, e,
            marker=markers[row["Dataset"]], linestyle="none",
            markerfacecolor=to_rgba(palette[row["Method"]], 0.5),
            markeredgecolor=palette[row["Method"]],
            markersize=marker_size(markers[row["Dataset"]]),
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
            elinewidth=0.6, capsize=1.5, capthick=0.6,
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
 
    # legend with methods above the plot
    method_handles = [Line2D([], [], marker="o", linestyle="none", color=palette[m], label=m, markersize=MARKER_SIZE, markeredgewidth=0) for m in methods_order]
    fig.legend(handles=method_handles, title="Method", loc="outside upper center", ncol=len(methods_order), frameon=False)

    # legend with datasets below the plot: a column is too narrow to place it on the right
    dataset_handles = [Line2D([], [], marker=markers[d], linestyle="none", color="gray", label=d, markersize=marker_size(markers[d]), markeredgewidth=0) for d in datasets_order]
    fig.legend(handles=dataset_handles, title="Dataset", loc="outside lower center", ncol=3, frameon=False, columnspacing=1.0, handletextpad=0.3)
 
    # save the plot: no bbox_inches="tight", so the PDF keeps exactly the size set above
    plt.savefig(output_path)


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
    ][columns_to_keep].copy()
    harmonic_df = harmonic_df[
        (harmonic_df["n_threads"] == n_threads) &
        (harmonic_df["k"] == k) & (
            (harmonic_df["Method"] == "mc") |
            ((harmonic_df["Method"] == "ew") & (harmonic_df["l"] == l)) |
            ((harmonic_df["Method"] == "pps") & (harmonic_df["l"] == l) & (harmonic_df["delta"] == delta))
        )
    ][columns_to_keep].copy()

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
    times_k_plot(summaries_df, "lin", palette, n_threads, l, c, delta, methods_order_lin, datasets_order, Path(args.output_dir) / "lin_times_k.pdf")
    times_k_plot(summaries_df, "harmonic", palette, n_threads, l, c, delta, methods_order_harmonic, datasets_order, Path(args.output_dir) / "harmonic_times_k.pdf")