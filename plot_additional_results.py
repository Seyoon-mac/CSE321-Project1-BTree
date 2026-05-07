import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# -----------------------------
# 1. Load additional result files
# -----------------------------

range_files = [
    "additional_range_results_m3.csv",
    "additional_range_results_m5.csv",
    "additional_range_results_m10.csv",
]

search_files = [
    "additional_search_results_m3.csv",
    "additional_search_results_m5.csv",
    "additional_search_results_m10.csv",
]

range_df = pd.concat([pd.read_csv(file) for file in range_files], ignore_index=True)
search_df = pd.concat([pd.read_csv(file) for file in search_files], ignore_index=True)

range_df["M"] = range_df["M"].astype(int)
search_df["M"] = search_df["M"].astype(int)

tree_order = ["BTree", "BStarTree", "BPlusTree"]
range_order = ["Small", "Medium", "Large"]
query_order = ["Successful", "Unsuccessful"]

# Save merged CSV files
range_df.to_csv("additional_range_results.csv", index=False)
search_df.to_csv("additional_search_results.csv", index=False)

print("Saved merged CSV files:")
print("- additional_range_results.csv")
print("- additional_search_results.csv")


# -----------------------------
# 2. Figure: Range query time by range size
#    One figure for each M
# -----------------------------

for m in [3, 5, 10]:
    sub_m = range_df[range_df["M"] == m]

    plt.figure(figsize=(6, 4))

    for tree in tree_order:
        sub = sub_m[sub_m["tree"] == tree].copy()
        sub["range_type"] = pd.Categorical(
            sub["range_type"],
            categories=range_order,
            ordered=True
        )
        sub = sub.sort_values("range_type")

        plt.plot(
            sub["range_type"],
            sub["range_time_ms"],
            marker="o",
            label=tree
        )

    plt.xlabel("Range Size")
    plt.ylabel("Range Query Time (ms)")
    plt.title(f"Range Query Time by Range Size (M={m})")
    plt.legend()
    plt.grid(True, linestyle="--", alpha=0.5)
    plt.tight_layout()

    output_name = f"fig_additional_range_m{m}.png"
    plt.savefig(output_name, dpi=300)
    plt.close()

    print(f"Saved: {output_name}")


# -----------------------------
# 3. Figure: Range query time by M
#    Separate line for each range size
#    One figure for each tree
# -----------------------------

for tree in tree_order:
    sub_tree = range_df[range_df["tree"] == tree]

    plt.figure(figsize=(6, 4))

    for range_type in range_order:
        sub = sub_tree[sub_tree["range_type"] == range_type].sort_values("M")

        plt.plot(
            sub["M"],
            sub["range_time_ms"],
            marker="o",
            label=range_type
        )

    plt.xlabel("M")
    plt.ylabel("Range Query Time (ms)")
    plt.title(f"Range Query Time by M ({tree})")
    plt.xticks([3, 5, 10])
    plt.legend()
    plt.grid(True, linestyle="--", alpha=0.5)
    plt.tight_layout()

    output_name = f"fig_additional_range_by_m_{tree}.png"
    plt.savefig(output_name, dpi=300)
    plt.close()

    print(f"Saved: {output_name}")


# -----------------------------
# 4. Figure: Successful vs Unsuccessful Search
#    One figure for each M
# -----------------------------

for m in [3, 5, 10]:
    sub_m = search_df[search_df["M"] == m]

    x_positions = range(len(tree_order))
    width = 0.35

    successful = []
    unsuccessful = []

    for tree in tree_order:
        tree_data = sub_m[sub_m["tree"] == tree]

        success_time = tree_data[
            tree_data["query_type"] == "Successful"
        ]["search_mean_us"].values[0]

        unsuccess_time = tree_data[
            tree_data["query_type"] == "Unsuccessful"
        ]["search_mean_us"].values[0]

        successful.append(success_time)
        unsuccessful.append(unsuccess_time)

    plt.figure(figsize=(6, 4))

    plt.bar(
        [x - width / 2 for x in x_positions],
        successful,
        width,
        label="Successful"
    )

    plt.bar(
        [x + width / 2 for x in x_positions],
        unsuccessful,
        width,
        label="Unsuccessful"
    )

    plt.xlabel("Tree Structure")
    plt.ylabel("Mean Search Time (us)")
    plt.title(f"Successful vs Unsuccessful Search (M={m})")
    plt.xticks(x_positions, tree_order)
    plt.legend()
    plt.grid(True, axis="y", linestyle="--", alpha=0.5)
    plt.tight_layout()

    output_name = f"fig_additional_search_m{m}.png"
    plt.savefig(output_name, dpi=300)
    plt.close()

    print(f"Saved: {output_name}")


# -----------------------------
# 5. Figure: Search time by M
#    One figure for each query type
# -----------------------------

for query_type in query_order:
    sub_query = search_df[search_df["query_type"] == query_type]

    plt.figure(figsize=(6, 4))

    for tree in tree_order:
        sub = sub_query[sub_query["tree"] == tree].sort_values("M")

        plt.plot(
            sub["M"],
            sub["search_mean_us"],
            marker="o",
            label=tree
        )

    plt.xlabel("M")
    plt.ylabel("Mean Search Time (us)")
    plt.title(f"{query_type} Search Time by M")
    plt.xticks([3, 5, 10])
    plt.legend()
    plt.grid(True, linestyle="--", alpha=0.5)
    plt.tight_layout()

    output_name = f"fig_additional_search_by_m_{query_type}.png"
    plt.savefig(output_name, dpi=300)
    plt.close()

    print(f"Saved: {output_name}")


# -----------------------------
# 6. Create compact tables for report
# -----------------------------

# M=10 range table is useful for report
range_m10 = range_df[range_df["M"] == 10].copy()
range_m10 = range_m10[
    ["tree", "range_type", "range_count", "male_count", "avg_gpa", "avg_height", "range_time_ms"]
]
range_m10.to_csv("table_additional_range_m10.csv", index=False)

# M=10 search table is useful for report
search_m10 = search_df[search_df["M"] == 10].copy()
search_m10 = search_m10[
    ["tree", "query_type", "search_mean_us", "found_count"]
]
search_m10.to_csv("table_additional_search_m10.csv", index=False)

print("Saved report tables:")
print("- table_additional_range_m10.csv")
print("- table_additional_search_m10.csv")