# Tenant-Aware Evaluation Plan (Publication-Oriented, Laptop-Friendly)

## What Was Added

The simulator now supports tenant-aware cache behavior through:

- `tenantPolicy=Baseline`
- `tenantPolicy=StaticPartitioning`
- `tenantPolicy=DynamicWeightedEviction`

with shared tenant extraction parameters:

- `tenantCount`
- `premiumTenantId`
- `tenantKeyModulo`
- `premiumProtectProbability`

The output `results.json` now includes tenant-level metrics:

- `tenant_to_cache_hits`
- `tenant_to_cache_lookups`
- `tenant_to_hit_rate`
- `tenant_to_avg_fct`

## Recommended Core Experiment (Low Compute)

Use one workload per run (same as existing framework), but map traffic into two competing tenants through key-based extraction.
This still creates contention in shared caches because both tenants coexist in the same trace and hit the same switch memory.

### Command

```bash
cd $NS3_HOME/scratch/switchv2p/results
./run.py -w websearch --tenantEval --maxProc 4
```

Then summarize:

```bash
./analyze_tenant_eval.py ./websearch_tenant_eval
```

For a reusable summary table + plot input:

```bash
./analyze_tenant_eval.py ./websearch_tenant_eval \
   --out-json ./websearch_tenant_eval_summary.json \
   --out-csv ./websearch_tenant_eval_summary.csv
```

Then generate figure set:

```bash
./plot_tenant_metrics.py ./websearch_tenant_eval_summary.json --out-dir ./plots
```

## Baseline vs Improvements

This pipeline automatically compares:

- Baseline (no tenant awareness)
- StaticPartitioning (fixed tenant partitions)
- DynamicWeightedEviction (premium protection during collisions)

across memory percentages `[5, 10, 25, 50]%`.

## Stronger Publishable Story (Still Laptop-Safe)

1. Run two workload families separately:
   - `websearch` (latency-sensitive)
   - `hadoop` (heavier/noisier)
2. Keep `--maxProc` in range `2..6`.
3. Use 3 random seeds per condition (recommended extension) for confidence intervals.
4. Report:
   - Premium tenant hit-rate gain vs baseline
   - Premium tenant FCT reduction vs baseline
   - Fairness cost on non-premium tenant
5. Plot memory-pressure sensitivity curves.

## Practical Limits to Avoid Overloading Your PC

- Start with `websearch`, `--maxProc 2`, and first 1-2 memory points.
- Avoid Alibaba for initial figure generation.
- Run long sweeps overnight only after validating one pilot run.

## Suggested Figure Set for Paper Draft

1. Premium tenant hit rate vs memory budget
2. Premium tenant FCT vs memory budget
3. Non-premium tenant FCT vs memory budget
4. Tradeoff: premium gain vs non-premium penalty
5. Drop-rate sanity plot (to show no pathological side effects)

The updated scripts now compute and export additional publication-grade parameters:

- `hit_rate_gap_pp_t0_minus_t1`
- `premium_hit_rate_rel_gain_pct_vs_t1`
- `premium_fct_reduction_pct_vs_t1`
- `fct_ratio_t1_over_t0`
- `avg_drop_rate`
- `avg_packet_latency_ns`
- `avg_packet_hops`

And they generate these graphs in the output plot directory:

- `tenant_hit_rate_curves.png`
- `premium_fct_reduction_pct.png`
- `premium_hit_gap_pp.png`
- `fct_ratio_t1_over_t0.png`
- `drop_rate_sanity.png`
