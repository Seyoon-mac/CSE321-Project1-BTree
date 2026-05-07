import pandas as pd
import matplotlib
matplotlib.use("Agg")  # save figures without opening plot windows
import matplotlib.pyplot as plt

df = pd.read_csv("results.csv")
df["M"] = df["M"].astype(int)

tree_order = ["BTree", "BStarTree", "BPlusTree"]

def plot_metric(y_col, y_label, title, output_name):
    plt.figure(figsize=(6, 4))

    for tree in tree_order:
        sub = df[df["tree"] == tree].sort_values("M")
        plt.plot(sub["M"], sub[y_col], marker="o", label=tree)

    plt.xlabel("M")
    plt.ylabel(y_label)
    plt.title(title)
    plt.xticks([3, 5, 10])
    plt.legend()
    plt.grid(True, linestyle="--", alpha=0.5)
    plt.tight_layout()
    plt.savefig(output_name, dpi=300)
    plt.close()

plot_metric(
    "splits",
    "Number of Splits",
    "Split Count by M",
    "fig_splits_by_m.png"
)

plot_metric(
    "range_time_ms",
    "Range Query Time (ms)",
    "Range Query Time by M",
    "fig_range_time_by_m.png"
)

plot_metric(
    "utilization",
    "Node Utilization",
    "Node Utilization by M",
    "fig_utilization_by_m.png"
)

print("Saved figures:")
print("- fig_splits_by_m.png")
print("- fig_range_time_by_m.png")
print("- fig_utilization_by_m.png")