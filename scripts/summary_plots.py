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
from matplotlib import font_manager

# render all text with LaTeX
USE_LATEX = True

def bold(text):
    """
    Returns the text so that it is rendered in bold: with LaTeX, fontweight="bold" is ignored,
    so the text must be wrapped in \\textbf{}.
    """
    return rf"\textbf{{{text}}}" if USE_LATEX else text

def times_k_plot(plot_df, centrality, palette, n_threads, l, c, delta, methods_order, datasets_order, output_path, n_cols=3):
    """
    Creates a single figure with one bar subplot for each dataset across different values of k.
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

    # set theme and style
    sns.set_theme("paper", font_scale=1.0)
    sns.set_style("whitegrid")
    plt.rcParams.update({"text.usetex": USE_LATEX, "font.family": "serif" if USE_LATEX else "sans-serif"})

    # compute the number of rows for the subplots
    n_rows = int(np.ceil(len(datasets_order) / n_cols))

    # initialize the figure with the desired number of rows and columns
    fig, axes = plt.subplots(n_rows, n_cols, figsize=(5 * n_cols, 4 * n_rows), squeeze=False, layout="constrained")

    # add some space among rows
    fig.get_layout_engine().set(hspace=0.06)

    # flattes the array with the axes of the subplots
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
            saturation=1
        )

        # remove the legend: we want a single legend for all subplots on top of them
        if ax.get_legend() is not None:
            ax.get_legend().remove()

        # set title, axes labels and ticks
        ax.set_title(bold(dataset), fontsize=18, fontweight="bold")
        ax.set_xlabel(f"({chr(ord('a') + i)})", fontsize=18)
        ax.set_ylabel("")
        ax.tick_params(axis="both", which="both", labelsize=16)
 
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
    x_label = fig.supxlabel("Number of Sampled Possible Worlds (k)", fontsize=18)
    y_label = fig.supylabel("Time (s)", fontsize=18)
 
    # create a single, shared legend above all subplots
    handles = [Patch(facecolor=palette[m], label=m) for m in methods_order]
    legend = fig.legend(
        handles=handles,
        title="Method",
        loc="lower center",
        bbox_to_anchor=(0.5, 1.0),
        ncol=len(methods_order),
        frameon=False,
        title_fontsize=18,
        fontsize=16
    )
 
    # save the plot
    plt.savefig(output_path, bbox_inches="tight", bbox_extra_artists=[legend, x_label, y_label])

def times_l_plot(plot_df, centrality, palette, n_threads, k, c, delta, methods_order, datasets_order, output_path, n_cols=3):
    """
    Creates a single figure with one bar subplot for each dataset across different values of l.
    """
 
    # filter the dataframe by keeping only required data (the exact method does not depend on l)
    df = plot_df[(plot_df["Centrality"] == centrality) & (plot_df["n_threads"] == n_threads) & (plot_df["k"] == k)]
    ew_mask = df["Method"] == "ew"
    if centrality == "lin":
        ew_mask &= df["c"] == c
    pps_mask = (df["Method"] == "pps") & (df["delta"] == delta)
    df = df[ew_mask | pps_mask][["Dataset", "Method", "l", "Time"]].copy()
 
    # map the names of the methods
    prefix = "Lin" if centrality == "lin" else "Harmonic"
    df["Method"] = df["Method"].map({"ew": f"{prefix}EW", "pps": f"{prefix}PPS"})
    df["l"] = df["l"].astype(int)
    l_order = sorted(df["l"].unique())

    # keep only the methods that appear in the plot
    methods_order = [m for m in methods_order if m != "Exact"]

    # close any previously opened subplot
    plt.close()

    # set theme and style
    sns.set_theme("paper", font_scale=1.0)
    sns.set_style("whitegrid")
    plt.rcParams.update({"text.usetex": USE_LATEX, "font.family": "serif" if USE_LATEX else "sans-serif"})

    # compute the number of rows for the subplots
    n_rows = int(np.ceil(len(datasets_order) / n_cols))

    # initialize the figure with the desired number of rows and columns
    fig, axes = plt.subplots(n_rows, n_cols, figsize=(5 * n_cols, 4 * n_rows), squeeze=False, layout="constrained")

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
            x="l",
            y="Time",
            hue="Method",
            order=l_order,
            hue_order=methods_order,
            palette=palette,
            ax=ax,
            saturation=1
        )

        # remove the legend: we want a single legend for all subplots on top of them
        if ax.get_legend() is not None:
            ax.get_legend().remove()

        # set title, axes labels and ticks
        ax.set_title(bold(dataset), fontsize=18, fontweight="bold")
        ax.set_xlabel(f"({chr(ord('a') + i)})", fontsize=18)
        ax.set_ylabel("")
        ax.tick_params(axis="both", which="both", labelsize=16)
 
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
    x_label = fig.supxlabel("Number of Sampled Nodes (l)", fontsize=18)
    y_label = fig.supylabel("Time (s)", fontsize=18)
 
    # create a single, shared legend above all subplots
    handles = [Patch(facecolor=palette[m], label=m) for m in methods_order]
    legend = fig.legend(
        handles=handles,
        title="Method",
        loc="lower center",
        bbox_to_anchor=(0.5, 1.0),
        ncol=len(methods_order),
        frameon=False,
        title_fontsize=18,
        fontsize=16
    )
 
    # save the plot
    plt.savefig(output_path, bbox_inches="tight", bbox_extra_artists=[legend, x_label, y_label])

def errors_k_plot(plot_df, centrality, palette, n_threads, l, c, delta, methods_order, datasets_order, output_path, n_cols=3):
    """
    Creates a single figure with one box subplot for each dataset showing the errors across different values of k.
    """
 
    # filter the dataframe by keeping only required data
    df = plot_df[(plot_df["Centrality"] == centrality) & (plot_df["n_threads"] == n_threads)]
    ew_mask = (df["Method"] == "ew") & (df["l"] == l)
    if centrality == "lin":
        ew_mask &= df["c"] == c
    pps_mask = (df["Method"] == "pps") & (df["l"] == l) & (df["delta"] == delta)
    df = df[(df["Method"] == "mc") | ew_mask | pps_mask][["Dataset", "Method", "k", "MAE"]].copy()
 
    # map the names of the methods
    prefix = "Lin" if centrality == "lin" else "Harmonic"
    df["Method"] = df["Method"].map({"mc": "Exact", "ew": f"{prefix}EW", "pps": f"{prefix}PPS"})
    df["k"] = df["k"].astype(int)
    k_order = sorted(df["k"].unique())

    # close any previously opened subplot
    plt.close()

    # set theme and style
    sns.set_theme("paper", font_scale=1.0)
    sns.set_style("whitegrid")
    plt.rcParams.update({"text.usetex": USE_LATEX, "font.family": "serif" if USE_LATEX else "sans-serif"})

    # compute the number of rows for the subplots
    n_rows = int(np.ceil(len(datasets_order) / n_cols))

    # initialize the figure with the desired number of rows and columns
    fig, axes = plt.subplots(n_rows, n_cols, figsize=(5 * n_cols, 4 * n_rows), squeeze=False, layout="constrained")

    # add some space among rows
    fig.get_layout_engine().set(hspace=0.06)

    # flatten the array with the axes of the subplots
    axes = axes.flatten()

    # iterate over the subplots and datasets
    for i, (ax, dataset) in enumerate(zip(axes, datasets_order)):
        
        # extract the current dataset
        dataset_df = df[df["Dataset"] == dataset]

        # create the boxplot
        sns.boxplot(
            data=dataset_df,
            x="k",
            y="MAE",
            hue="Method",
            order=k_order,
            hue_order=methods_order,
            palette=palette,
            ax=ax,
            saturation=1
        )

        # remove the legend: we want a single legend for all subplots on top of them
        if ax.get_legend() is not None:
            ax.get_legend().remove()

        # set title, axes labels and ticks
        ax.set_title(bold(dataset), fontsize=18, fontweight="bold")
        ax.set_xlabel(f"({chr(ord('a') + i)})", fontsize=18)
        ax.set_ylabel("")
        ax.tick_params(axis="both", which="both", labelsize=16)
 
        # make the y axis start from 0 and format its values to 3 decimals
        ax.set_ylim(bottom=0)
        ax.yaxis.set_major_formatter(ticker.FormatStrFormatter("%.3f"))
 
    # hide unused subplots if the grid is larger than the number of datasets
    for ax in axes[len(datasets_order):]:
        ax.set_visible(False)
 
    # set shared axes labels
    x_label = fig.supxlabel("Number of Sampled Possible Worlds (k)", fontsize=18)
    y_label = fig.supylabel("Error (MAE)", fontsize=18)
 
    # create a single, shared legend above all subplots
    handles = [Patch(facecolor=palette[m], label=m) for m in methods_order]
    legend = fig.legend(
        handles=handles,
        title="Method",
        loc="lower center",
        bbox_to_anchor=(0.5, 1.0),
        ncol=len(methods_order),
        frameon=False,
        title_fontsize=18,
        fontsize=16
    )
 
    # save the plot
    plt.savefig(output_path, bbox_inches="tight", bbox_extra_artists=[legend, x_label, y_label])

def errors_l_plot(plot_df, centrality, palette, n_threads, k, c, delta, methods_order, datasets_order, output_path, n_cols=3):
    """
    Creates a single figure with one box subplot for each dataset showing the errors across different values of l.
    """
 
    # filter the dataframe by keeping only required data (the exact method does not depend on l)
    df = plot_df[(plot_df["Centrality"] == centrality) & (plot_df["n_threads"] == n_threads) & (plot_df["k"] == k)]
    ew_mask = df["Method"] == "ew"
    if centrality == "lin":
        ew_mask &= df["c"] == c
    pps_mask = (df["Method"] == "pps") & (df["delta"] == delta)
    df = df[ew_mask | pps_mask][["Dataset", "Method", "l", "MAE"]].copy()
 
    # map the names of the methods
    prefix = "Lin" if centrality == "lin" else "Harmonic"
    df["Method"] = df["Method"].map({"ew": f"{prefix}EW", "pps": f"{prefix}PPS"})
    df["l"] = df["l"].astype(int)
    l_order = sorted(df["l"].unique())

    # keep only the methods that appear in the plot
    methods_order = [m for m in methods_order if m != "Exact"]

    # close any previously opened subplot
    plt.close()

    # set theme and style
    sns.set_theme("paper", font_scale=1.0)
    sns.set_style("whitegrid")
    plt.rcParams.update({"text.usetex": USE_LATEX, "font.family": "serif" if USE_LATEX else "sans-serif"})

    # compute the number of rows for the subplots
    n_rows = int(np.ceil(len(datasets_order) / n_cols))

    # initialize the figure with the desired number of rows and columns
    fig, axes = plt.subplots(n_rows, n_cols, figsize=(5 * n_cols, 4 * n_rows), squeeze=False, layout="constrained")

    # add some space among rows
    fig.get_layout_engine().set(hspace=0.06)

    # flatten the array with the axes of the subplots
    axes = axes.flatten()

    # iterate over the subplots and datasets
    for i, (ax, dataset) in enumerate(zip(axes, datasets_order)):
        
        # extract the current dataset
        dataset_df = df[df["Dataset"] == dataset]

        # create the boxplot
        sns.boxplot(
            data=dataset_df,
            x="l",
            y="MAE",
            hue="Method",
            order=l_order,
            hue_order=methods_order,
            palette=palette,
            ax=ax,
            saturation=1
        )

        # remove the legend: we want a single legend for all subplots on top of them
        if ax.get_legend() is not None:
            ax.get_legend().remove()

        # set title, axes labels and ticks
        ax.set_title(bold(dataset), fontsize=18, fontweight="bold")
        ax.set_xlabel(f"({chr(ord('a') + i)})", fontsize=18)
        ax.set_ylabel("")
        ax.tick_params(axis="both", which="both", labelsize=16)
 
        # make the y axis start from 0 and format its values to 3 decimals
        ax.set_ylim(bottom=0)
        ax.yaxis.set_major_formatter(ticker.FormatStrFormatter("%.3f"))
 
    # hide unused subplots if the grid is larger than the number of datasets
    for ax in axes[len(datasets_order):]:
        ax.set_visible(False)
 
    # set shared axes labels
    x_label = fig.supxlabel("Number of Sampled Nodes (l)", fontsize=18)
    y_label = fig.supylabel("Error (MAE)", fontsize=18)
 
    # create a single, shared legend above all subplots
    handles = [Patch(facecolor=palette[m], label=m) for m in methods_order]
    legend = fig.legend(
        handles=handles,
        title="Method",
        loc="lower center",
        bbox_to_anchor=(0.5, 1.0),
        ncol=len(methods_order),
        frameon=False,
        title_fontsize=18,
        fontsize=16
    )
 
    # save the plot
    plt.savefig(output_path, bbox_inches="tight", bbox_extra_artists=[legend, x_label, y_label])

def time_error_plot(plot_df, centrality, palette, markers, methods_order, datasets_order, output_path):
    """
    """

    # close previous plots
    plt.close()

    # set theme and style
    sns.set_theme("paper", font_scale=1.0)
    sns.set_style("whitegrid")
    plt.rcParams.update({"text.usetex": USE_LATEX, "font.family": "serif" if USE_LATEX else "sans-serif"})

    # create the figure: wide, since it spans both columns of the paper (figure*), so that once
    # scaled down to \textwidth the fonts are about half of the sizes set below (16pt -> ~8pt)
    fig, ax = plt.subplots(figsize=(14, 5), layout="constrained")

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
        loc="upper center", bbox_to_anchor=(0.5, -0.17), ncol=len(datasets_order),
        title_fontsize=16,
        fontsize=14
    )
 
    # save the plot
    plt.savefig(output_path, bbox_inches="tight", bbox_extra_artists=[method_legend, dataset_legend])

def time_error_plots(lin_plot_df, harmonic_plot_df, palette, markers, methods_order_lin, methods_order_harmonic, datasets_order, output_path):
    """
    Creates a single figure with a subplot per centrality showing error and time across datasets.
    """

    # close previous plots
    plt.close()

    # set theme and style
    sns.set_theme("paper", font_scale=1.0)
    sns.set_style("whitegrid")
    plt.rcParams.update({"text.usetex": USE_LATEX, "font.family": "serif" if USE_LATEX else "sans-serif"})

    # create the figure
    fig, axes = plt.subplots(1, 2, figsize=(14, 5), layout="constrained")

    # data, title and minimum value of the x axis for each subplot
    subplots = [
        (lin_plot_df, "Lin's Index", 1e-1),
        (harmonic_plot_df, "Harmonic Centrality", 1e-2),
    ]

    # iterate over the subplots
    for i, (ax, (plot_df, title, x_min)) in enumerate(zip(axes, subplots)):

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

        # set title and label of the subplot
        ax.set_title(bold(title), fontsize=16, fontweight="bold")
        ax.set_xlabel(f"({chr(ord('a') + i)})", fontsize=16)
        ax.set_ylabel("")
        ax.tick_params(axis="both", which="both", labelsize=14)

        # show only powers of 10 in the x axis and make it start from the desired value based on the centrality
        ax.set_xscale("log")
        ax.set_xlim(left=x_min)
        ax.xaxis.set_major_locator(ticker.LogLocator(base=10, numticks=20))
        ax.xaxis.set_major_formatter(ticker.LogFormatterMathtext(base=10))
        ax.xaxis.set_minor_formatter(ticker.NullFormatter())

        # format the values in the y axis to 3 decimals and make it start from 0
        ax.yaxis.set_major_formatter(ticker.FormatStrFormatter("%.3f"))
        ax.set_ylim(bottom=0)
        ax.yaxis.set_major_locator(ticker.MultipleLocator(0.001))

    # set shared axes labels
    x_label = fig.supxlabel("Time (s)", fontsize=16)
    y_label = fig.supylabel("Error (MAE)", fontsize=16)

    # create a shared legend with methods above the subplots
    method_labels = [m if m_lin == m_harmonic else f"{m_lin} / {m_harmonic}" for m, m_lin, m_harmonic in zip(methods_order_lin, methods_order_lin, methods_order_harmonic)]
    method_handles = [Line2D([], [], marker="o", linestyle="none", color=palette[m], label=label, markersize=12, markeredgewidth=0) for m, label in zip(methods_order_lin, method_labels)]
    method_legend = fig.legend(
        handles=method_handles, title="Method",
        loc="outside upper center", ncol=len(method_handles), frameon=False,
        title_fontsize=16,
        fontsize=14
    )

    # create a shared legend with datasets below the subplots
    dataset_handles = [Line2D([], [], marker=markers[d], linestyle="none", color="lightgray", label=d, markersize=12 if markers[d] != "*" else 12 * 1.3, markeredgewidth=0.5) for d in datasets_order]
    dataset_legend = fig.legend(
        handles=dataset_handles, title="Dataset",
        loc="upper center", bbox_to_anchor=(0.5, 0.0), ncol=len(datasets_order),
        title_fontsize=16,
        fontsize=14
    )

    # save the plot
    plt.savefig(output_path, bbox_inches="tight", bbox_extra_artists=[method_legend, dataset_legend, x_label, y_label])

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
    time_error_plots(lin_plot_df, harmonic_plot_df, palette, markers, methods_order_lin, methods_order_harmonic, datasets_order, Path(args.output_dir) / "summary_plot.pdf")
    times_k_plot(summaries_df, "lin", palette, n_threads, l, c, delta, methods_order_lin, datasets_order, Path(args.output_dir) / "lin_times_k.pdf")
    times_k_plot(summaries_df, "harmonic", palette, n_threads, l, c, delta, methods_order_harmonic, datasets_order, Path(args.output_dir) / "harmonic_times_k.pdf")
    times_l_plot(summaries_df, "lin", palette, n_threads, k, c, delta, methods_order_lin, datasets_order, Path(args.output_dir) / "lin_times_l.pdf")
    times_l_plot(summaries_df, "harmonic", palette, n_threads, k, c, delta, methods_order_harmonic, datasets_order, Path(args.output_dir) / "harmonic_times_l.pdf")
    errors_k_plot(summaries_df, "lin", palette, n_threads, l, c, delta, methods_order_lin, datasets_order, Path(args.output_dir) / "lin_errors_k.pdf")
    errors_k_plot(summaries_df, "harmonic", palette, n_threads, l, c, delta, methods_order_harmonic, datasets_order, Path(args.output_dir) / "harmonic_errors_k.pdf")
    errors_l_plot(summaries_df, "lin", palette, n_threads, k, c, delta, methods_order_lin, datasets_order, Path(args.output_dir) / "lin_errors_l.pdf")
    errors_l_plot(summaries_df, "harmonic", palette, n_threads, k, c, delta, methods_order_harmonic, datasets_order, Path(args.output_dir) / "harmonic_errors_l.pdf")