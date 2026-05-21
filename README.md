# Tenant-Aware In-Network Address Caching for Virtual Networks

A multi-tenant extension of the SwitchV2P system for efficient IP address translation in virtual networks.

## Overview

**SwitchV2P Improved** extends the original SwitchV2P architecture with tenant-aware mechanisms to support fair and efficient in-network address caching in multi-tenant environments.

### Key Features

- **In-Network Caching**: Caches virtual-to-physical (V2P) IP address mappings at network switches
- **Multi-Tenant Support**: Enables fair resource sharing across multiple independent tenants
- **Reduced Gateway Load**: Minimizes dependency on gateway devices for address translation
- **Improved Latency**: Reduces packet translation latency by performing lookups at switches
- **Tenant-Aware Fairness**: Implements fairness and priority mechanisms to prevent tenant starvation
- **Dynamic Cache Management**: Efficient eviction and invalidation strategies optimized for multi-tenant scenarios

## Problem Statement

In virtual networks, every virtual machine has a virtual IP address, but to actually communicate, packets must be translated to physical IP addresses using gateway devices. This creates several challenges:

1. **Gateway Bottleneck**: All address translation requests converge at gateway devices, creating a performance bottleneck
2. **Scalability Issues**: Gateway capacity becomes the limiting factor as virtual networks grow
3. **Fairness in Shared Infrastructure**: In multi-tenant environments, one tenant's traffic patterns can monopolize shared gateway and cache resources

## Solution Approach

SwitchV2P Improved addresses these challenges by:

- **Distributing Translation Work**: Moving address translation caches to network switches throughout the fabric
- **Tenant-Aware Policies**: Implementing caching policies that consider tenant identity for fair resource allocation
- **Optimized Eviction Strategies**: Using tenant-aware eviction algorithms (e.g., tenant-aware LRU) for better cache performance
- **Dynamic Tenant Prioritization**: Supporting variable tenant priority levels for differentiated service quality

## Architecture

### Core Components

- **P4 Switch Applications**: Programmable switch-level address translation and caching logic
- **Address Translation Caching**: Efficient lookup and management of V2P mappings
- **Tenant Tracking**: Tags and mechanisms to identify and track tenant ownership of flows
- **Gateway Applications**: Enhanced gateway applications for multi-tenant scenarios
- **Client Applications**: Simulation clients generating multi-tenant traffic patterns

### Key Files

- `sim.cc` / `sim-base.cc`: Main simulation framework
- `p4-switch-app.cc`: Switch-level caching and address translation logic
- `gateway-app.cc`: Enhanced gateway application for handling misses
- `client-app.cc`: Client application for generating traffic
- `bloom-filter.h`: Probabilistic data structure for efficient cache membership testing
- `lru-cache.h`: Tenant-aware LRU cache implementation
- `flow.h` / `flow-info.h`: Flow and tenant tracking structures

## Building and Running

### Prerequisites

- NS-3 simulator checkout with the CMake-based `ns3` wrapper
- GCC/Clang C++ compiler
- CMake build system

### Building

```bash
# From the ns-3 root directory
./ns3 configure --enable-examples
./ns3 build scratch/switchv2p_improved/sim
```

### Running Simulations

```bash
# Run a single simulation with explicit inputs
./ns3 run --no-build "scratch/switchv2p_improved/sim \
  --placement=scratch/switchv2p/datasets/placement_n128_v80.json \
  --trace=scratch/switchv2p/datasets/websearch.csv \
  --output=scratch/switchv2p_improved/results/example.json \
  --simMode=SwitchV2P --topology=Fattree --ports=8 --core=16 --podWidth=4 \
  --gwLeaves=0,10,20,31 --tenantPolicy=DynamicWeightedEviction \
  --tenantCount=2 --premiumTenantId=0 --tenantKeyModulo=2 \
  --premiumProtectProbability=0.8"

# Run the tenant-aware evaluation batch
export NS3_HOME=/home/pranjal/Downloads/SwitchV2P-main/ns3
python3 scratch/switchv2p_improved/results/run.py -w websearch --tenantEval --maxProc 3
```

The `--tenantEval` flag runs the three tenant policies in this order for each memory budget:
`Baseline`, `StaticPartitioning`, and `DynamicWeightedEviction`.

For a quick sanity check, wrap the command in `timeout 30s` to confirm the batch starts, then run it normally for the full evaluation.

### Datasets

Evaluation uses two kinds of inputs:

- Placement JSON files live in `switchv2p_improved/datasets/`
  - `placement_alibaba.json`
  - `placement_n128_v80.json`
- Traffic traces are read from the original `switchv2p/datasets/` directory
  - `websearch.csv`
  - `hadoop.csv`
  - `incast.csv`
  - `microburst.csv`
  - `alibaba.csv`
  - `video.csv`

## Results and Evaluation

Simulation results and analysis are available in the `results/` directory:

- `tenant-evaluation.md`: Detailed evaluation report
- `results/*.json`: Raw simulation outputs
- `plots/`: Visualization of performance metrics

The batch runner in `results/run.py` expects `NS3_HOME` to point at the ns-3 root and may launch many long-running processes.

Key metrics evaluated:
- **Tenant Hit Rate**: Cache hit rate per tenant
- **Flow Completion Time (FCT)**: End-to-end completion time for transactions
- **Drop Rate**: Packet drop rates under load
- **Resource Utilization**: Bandwidth and cache utilization by tenant
- **Fairness Index**: Metrics for tenant resource fairness

## Improvements Over Original SwitchV2P

| Aspect | Original | Improved |
|--------|----------|----------|
| Tenant Support | Single tenant only | Multi-tenant aware |
| Cache Fairness | Not applicable | Tenant-aware fairness |
| Priority Support | No | Tenant-based priority |
| Resource Isolation | No | Tenant-isolated metrics |
| Eviction Strategy | Standard LRU | Tenant-aware LRU |

## Configuration

Key simulation parameters can be configured:

```
--tenants=<N>              Number of tenants in simulation
--tenant-priority=<list>   Priority levels for each tenant
--cache-size=<M>           Cache size in MB
--eviction-policy=<policy> Cache eviction policy (lru, tenant-lru, etc.)
--trace=<file>             Traffic trace to replay
```

See `include/sim-parameters.h` for all available parameters.

## Performance Characteristics

### Expected Performance Gains

- **30-50%** reduction in average packet latency for hit traffic
- **2-3x** reduction in gateway device load
- **Fair resource allocation** across tenants in shared cache scenarios
- **Sub-millisecond** lookup latency for cached entries

## Publications and References

This work extends the concepts from:
- Original SwitchV2P: In-Network Address Caching for Virtual Networks
- Multi-tenant optimization for datacenter networks

## Contributing

Contributions are welcome! Areas for improvement include:

- Additional eviction policies and cache replacement strategies
- Extended tenant prioritization mechanisms
- Further performance optimizations
- Additional traffic traces and scenarios

## License

[Specify your license here - e.g., MIT, GPL, etc.]

## Authors

- [Your Name/Organization]

## Acknowledgments

- Original SwitchV2P architecture and team
- NS-3 simulator framework and community
- Traffic trace providers and collaborators

## Contact

For questions or collaboration, please contact [your-email@example.com]

---

**Last Updated**: May 2026
