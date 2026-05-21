#!/usr/bin/env python3

import argparse
import csv
import json
import os
from collections import defaultdict


def _safe_float(value, default=0.0):
    try:
        return float(value)
    except (TypeError, ValueError):
        return default


def _safe_ratio(num, den):
    return 0.0 if den == 0 else num / den


def load_runs(results_dir):
    runs = []
    for name in sorted(os.listdir(results_dir), key=lambda x: int(x) if x.isdigit() else 10**9):
        run_dir = os.path.join(results_dir, name)
        if not os.path.isdir(run_dir):
            continue

        cfg_path = os.path.join(run_dir, "config.json")
        res_path = os.path.join(run_dir, "results.json")
        if not os.path.exists(cfg_path) or not os.path.exists(res_path):
            continue

        with open(cfg_path, "r") as f:
            cfg = json.load(f)
        with open(res_path, "r") as f:
            res = json.load(f)

        runs.append((cfg, res, run_dir))

    return runs


def summarize(runs):
    grouped = defaultdict(lambda: {
        "tenant0_hit_rate": [],
        "tenant1_hit_rate": [],
        "tenant0_hits": [],
        "tenant1_hits": [],
        "tenant0_lookups": [],
        "tenant1_lookups": [],
        "tenant0_fct_ns": [],
        "tenant1_fct_ns": [],
        "overall_fct_ns": [],
        "packet_latency_ns": [],
        "packet_hops": [],
        "drops": [],
        "drop_rate": [],
    })

    for cfg, res, _ in runs:
        key = (cfg.get("p", 0), cfg.get("tenantPolicy", "Baseline"))
        hit_rate = res.get("tenant_to_hit_rate", {})
        fct = res.get("tenant_to_avg_fct", {})
        hits = res.get("tenant_to_cache_hits", {})
        lookups = res.get("tenant_to_cache_lookups", {})

        tx = _safe_float(res.get("total_tx_packets", 0.0))
        drops = _safe_float(res.get("total_dropped_packets", 0.0))

        grouped[key]["tenant0_hit_rate"].append(_safe_float(hit_rate.get("0", 0.0)))
        grouped[key]["tenant1_hit_rate"].append(_safe_float(hit_rate.get("1", 0.0)))
        grouped[key]["tenant0_hits"].append(_safe_float(hits.get("0", 0.0)))
        grouped[key]["tenant1_hits"].append(_safe_float(hits.get("1", 0.0)))
        grouped[key]["tenant0_lookups"].append(_safe_float(lookups.get("0", 0.0)))
        grouped[key]["tenant1_lookups"].append(_safe_float(lookups.get("1", 0.0)))
        grouped[key]["tenant0_fct_ns"].append(_safe_float(fct.get("0", 0.0)))
        grouped[key]["tenant1_fct_ns"].append(_safe_float(fct.get("1", 0.0)))
        grouped[key]["overall_fct_ns"].append(_safe_float(res.get("avg_fct", 0.0)))
        grouped[key]["packet_latency_ns"].append(_safe_float(res.get("avg_packet_latency", 0.0)))
        grouped[key]["packet_hops"].append(_safe_float(res.get("avg_packet_hops", 0.0)))
        grouped[key]["drops"].append(drops)
        grouped[key]["drop_rate"].append(_safe_ratio(drops, tx))

    rows = []
    for (p, policy), vals in grouped.items():
        n = max(1, len(vals["tenant0_hit_rate"]))

        tenant0_hit_rate = sum(vals["tenant0_hit_rate"]) / n
        tenant1_hit_rate = sum(vals["tenant1_hit_rate"]) / n
        tenant0_fct_ns = sum(vals["tenant0_fct_ns"]) / n
        tenant1_fct_ns = sum(vals["tenant1_fct_ns"]) / n

        hit_gap_pp = (tenant0_hit_rate - tenant1_hit_rate) * 100.0
        premium_hit_rel_gain_pct = _safe_ratio(tenant0_hit_rate - tenant1_hit_rate,
                                               tenant1_hit_rate) * 100.0
        premium_fct_reduction_pct = _safe_ratio(tenant1_fct_ns - tenant0_fct_ns,
                                                tenant1_fct_ns) * 100.0

        rows.append({
            "memory_pct": p,
            "policy": policy,
            "tenant0_hit_rate": tenant0_hit_rate,
            "tenant1_hit_rate": tenant1_hit_rate,
            "tenant0_cache_hits": sum(vals["tenant0_hits"]) / n,
            "tenant1_cache_hits": sum(vals["tenant1_hits"]) / n,
            "tenant0_cache_lookups": sum(vals["tenant0_lookups"]) / n,
            "tenant1_cache_lookups": sum(vals["tenant1_lookups"]) / n,
            "tenant0_avg_fct_ns": tenant0_fct_ns,
            "tenant1_avg_fct_ns": tenant1_fct_ns,
            "overall_avg_fct_ns": sum(vals["overall_fct_ns"]) / n,
            "avg_packet_latency_ns": sum(vals["packet_latency_ns"]) / n,
            "avg_packet_hops": sum(vals["packet_hops"]) / n,
            "avg_drops": sum(vals["drops"]) / n,
            "avg_drop_rate": sum(vals["drop_rate"]) / n,
            "hit_rate_gap_pp_t0_minus_t1": hit_gap_pp,
            "premium_hit_rate_rel_gain_pct_vs_t1": premium_hit_rel_gain_pct,
            "premium_fct_reduction_pct_vs_t1": premium_fct_reduction_pct,
            "fct_ratio_t1_over_t0": _safe_ratio(tenant1_fct_ns, tenant0_fct_ns),
            "runs": len(vals["tenant0_hit_rate"]),
        })

    return sorted(rows, key=lambda r: (r["memory_pct"], r["policy"]))


def print_table(rows):
    header = (
        "memory_pct,policy,tenant0_hit_rate,tenant1_hit_rate,"
        "tenant0_avg_fct_ns,tenant1_avg_fct_ns,hit_rate_gap_pp_t0_minus_t1,"
        "premium_fct_reduction_pct_vs_t1,premium_hit_rate_rel_gain_pct_vs_t1,"
        "avg_packet_latency_ns,avg_packet_hops,avg_drop_rate,runs"
    )
    print(header)
    for r in rows:
        print(
            f"{r['memory_pct']},{r['policy']},"
            f"{r['tenant0_hit_rate']:.6f},{r['tenant1_hit_rate']:.6f},"
            f"{r['tenant0_avg_fct_ns']:.2f},{r['tenant1_avg_fct_ns']:.2f},"
            f"{r['hit_rate_gap_pp_t0_minus_t1']:.4f},"
            f"{r['premium_fct_reduction_pct_vs_t1']:.4f},"
            f"{r['premium_hit_rate_rel_gain_pct_vs_t1']:.4f},"
            f"{r['avg_packet_latency_ns']:.2f},{r['avg_packet_hops']:.4f},"
            f"{r['avg_drop_rate']:.8f},{r['runs']}"
        )


def write_csv(rows, csv_path):
    if not rows:
        return
    fields = list(rows[0].keys())
    with open(csv_path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("results_dir", help="Directory created by run.py --tenantEval")
    parser.add_argument("--out-json", help="Optional summary JSON output path")
    parser.add_argument("--out-csv", help="Optional summary CSV output path")
    args = parser.parse_args()

    runs = load_runs(args.results_dir)
    if not runs:
        print("No completed runs found")
        return 1

    rows = summarize(runs)
    print_table(rows)

    if args.out_json:
        with open(args.out_json, "w") as f:
            json.dump(rows, f, indent=2)

    if args.out_csv:
        write_csv(rows, args.out_csv)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
