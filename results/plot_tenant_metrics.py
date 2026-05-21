#!/usr/bin/env python3

import argparse
import json
import os

import matplotlib.pyplot as plt


def load_summary(summary_path):
    with open(summary_path, "r") as f:
        rows = json.load(f)
    if not rows:
        raise ValueError("Empty summary JSON")
    return rows


def to_policy_map(rows, metric_key):
    by_policy = {}
    for row in rows:
        policy = row["policy"]
        by_policy.setdefault(policy, []).append((float(row["memory_pct"]), float(row[metric_key])))

    for policy in by_policy:
        by_policy[policy] = sorted(by_policy[policy], key=lambda x: x[0])

    return by_policy


def plot_metric(rows, metric_key, ylabel, title, out_path, transform=lambda x: x):
    plt.figure(figsize=(8.2, 5.2))
    by_policy = to_policy_map(rows, metric_key)

    for policy, pairs in sorted(by_policy.items()):
        xs = [p[0] * 100.0 for p in pairs]
        ys = [transform(p[1]) for p in pairs]
        plt.plot(xs, ys, marker="o", linewidth=2.0, label=policy)

    plt.xlabel("Memory Budget (% of address space)")
    plt.ylabel(ylabel)
    plt.title(title)
    plt.grid(True, linestyle="--", alpha=0.35)
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_path, dpi=300)
    print(f"Saved {out_path}")
    plt.close()


def plot_dual_tenant_hit_rates(rows, out_path):
    plt.figure(figsize=(9.0, 5.2))
    by_policy_t0 = to_policy_map(rows, "tenant0_hit_rate")
    by_policy_t1 = to_policy_map(rows, "tenant1_hit_rate")

    for policy in sorted(by_policy_t0.keys()):
        xs = [p[0] * 100.0 for p in by_policy_t0[policy]]
        y0 = [p[1] * 100.0 for p in by_policy_t0[policy]]
        y1 = [p[1] * 100.0 for p in by_policy_t1[policy]]

        plt.plot(xs, y0, marker="o", linewidth=2.0, label=f"{policy} - Premium (T0)")
        plt.plot(xs, y1, marker="s", linewidth=2.0, linestyle="--",
                 label=f"{policy} - Standard (T1)")

    plt.xlabel("Memory Budget (% of address space)")
    plt.ylabel("Cache Hit Rate (%)")
    plt.title("Tenant Hit Rates Across Policies")
    plt.grid(True, linestyle="--", alpha=0.35)
    plt.legend(ncol=2, fontsize=8)
    plt.tight_layout()
    plt.savefig(out_path, dpi=300)
    print(f"Saved {out_path}")
    plt.close()


def main():
    parser = argparse.ArgumentParser(description="Generate publication-style tenant evaluation plots")
    parser.add_argument("summary_json", help="Path produced by analyze_tenant_eval.py --out-json")
    parser.add_argument("--out-dir", default=".", help="Output directory for plots")
    args = parser.parse_args()

    rows = load_summary(args.summary_json)
    os.makedirs(args.out_dir, exist_ok=True)

    plot_dual_tenant_hit_rates(rows, os.path.join(args.out_dir, "tenant_hit_rate_curves.png"))
    plot_metric(
        rows,
        "premium_fct_reduction_pct_vs_t1",
        "Premium FCT Reduction vs Standard (%)",
        "Latency Gain for Premium Tenant",
        os.path.join(args.out_dir, "premium_fct_reduction_pct.png"),
    )
    plot_metric(
        rows,
        "hit_rate_gap_pp_t0_minus_t1",
        "Hit-Rate Gap (percentage points)",
        "Premium Hit-Rate Advantage",
        os.path.join(args.out_dir, "premium_hit_gap_pp.png"),
    )
    plot_metric(
        rows,
        "fct_ratio_t1_over_t0",
        "FCT Ratio (T1 / T0)",
        "Fairness Ratio: Standard vs Premium Latency",
        os.path.join(args.out_dir, "fct_ratio_t1_over_t0.png"),
    )
    plot_metric(
        rows,
        "avg_drop_rate",
        "Average Drop Rate",
        "Drop-Rate Sanity Check",
        os.path.join(args.out_dir, "drop_rate_sanity.png"),
    )


if __name__ == "__main__":
    main()
