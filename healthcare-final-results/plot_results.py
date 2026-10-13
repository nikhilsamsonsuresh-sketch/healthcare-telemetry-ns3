import csv
import os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

folder = os.path.dirname(os.path.abspath(__file__))

with open(os.path.join(folder, "results.csv"), newline="") as f:
    rows = list(csv.DictReader(f))

loads = [1, 4, 8]
protocols = ["UDP", "TCP"]

def value(row, column):
    return float(row[column])

def plot_metric(column, title, ylabel, filename, healthcare=True):
    plt.figure(figsize=(8, 5))

    for protocol in protocols:
        selected = [
            next(r for r in rows
                 if int(r["Background_Load_Mbps"]) == load
                 and r["Protocol"] == protocol)
            for load in loads
        ]
        plt.plot(
            loads,
            [value(r, column) for r in selected],
            marker="o",
            linewidth=2,
            label=protocol
        )

    plt.title(title)
    plt.xlabel("Background Traffic Load (Mbps)")
    plt.ylabel(ylabel)
    plt.xticks(loads)
    plt.grid(True, linestyle="--", alpha=0.5)
    plt.legend(title="Healthcare Protocol")
    plt.tight_layout()
    plt.savefig(os.path.join(folder, filename), dpi=200)
    plt.close()
    print("Created:", filename)

plot_metric(
    "Healthcare_Delay_ms",
    "Healthcare Packet Delay: TCP vs UDP",
    "Average Delay (ms)",
    "healthcare_delay.png"
)

plot_metric(
    "Healthcare_Jitter_ms",
    "Healthcare Packet Jitter: TCP vs UDP",
    "Average Jitter (ms)",
    "healthcare_jitter.png"
)

plot_metric(
    "Healthcare_Throughput_kbps",
    "Healthcare Throughput: TCP vs UDP",
    "Throughput (kbps)",
    "healthcare_throughput.png"
)

plot_metric(
    "Healthcare_Lost",
    "Healthcare Packet Loss: TCP vs UDP",
    "Lost Packets",
    "healthcare_packet_loss.png"
)

plot_metric(
    "Background_Lost",
    "Background Packet Loss Under Congestion",
    "Lost Packets",
    "background_packet_loss.png"
)

print("\nAll graphs saved in:", folder)
