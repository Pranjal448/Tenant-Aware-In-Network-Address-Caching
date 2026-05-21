# Tenant-aware additions over switchv2p

This project is an extension to paper "In-Network Address Caching for Virtual Networks" published in ACM SIGCOMM 2024. The original switchv2p codebase already provides the base simulator, switch, gateway, client, and trace handling. This README documents what was added or changed here.

## What changed

### Tenant-aware cache policies

`sim.cc` and `sim-parameters.*` now support three tenant cache policies:

- `Baseline`
- `StaticPartitioning`
- `DynamicWeightedEviction`

### New tenant parameters

The improved simulator adds these runtime parameters:

- `tenantPolicy`
- `tenantCount`
- `premiumTenantId`
- `tenantKeyModulo`
- `premiumProtectProbability`

These are mapped into `P4SwitchApp` defaults before the simulation starts.

### Tenant-aware evaluation runner

`results/run.py` adds a tenant-evaluation mode that runs the same workload under all three policies for each memory budget.

It uses:

- placement files from `switchv2p_improved/datasets/`
- trace files from the original `switchv2p/datasets/`

The generated outputs include tenant-level metrics and plots in `results/`.

## Verified tenant-evaluation command

Run from the ns-3 root:

```bash
export NS3_HOME=/home/pranjal/Downloads/SwitchV2P-main/ns3
python3 scratch/switchv2p_improved/results/run.py -w websearch --tenantEval --maxProc 3
```

This launches the three improved policies in order for each budget:

1. `Baseline`
2. `StaticPartitioning`
3. `DynamicWeightedEviction`

## Direct simulation example

If you want to run one policy directly:

```bash
./ns3 run --no-build "scratch/switchv2p_improved/sim \
  --placement=scratch/switchv2p/datasets/placement_n128_v80.json \
  --trace=scratch/switchv2p/datasets/websearch.csv \
  --output=scratch/switchv2p_improved/results/example.json \
  --simMode=SwitchV2P --topology=Fattree --ports=8 --core=16 --podWidth=4 \
  --gwLeaves=0,10,20,31 --tenantPolicy=DynamicWeightedEviction \
  --tenantCount=2 --premiumTenantId=0 --tenantKeyModulo=2 \
  --premiumProtectProbability=0.8"
```

## Notes

- The base SwitchV2P build and layout remain the same as the original project.
- If you need the full original background, check out original repo https://github.com/acsl-technion/SwitchV2P
