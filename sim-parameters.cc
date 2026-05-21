#include "include/sim-parameters.h"
#include <algorithm>

const map<string, enum SimulationParameters::Mode> SimulationParameters::simulationModeMap =
    boost::assign::map_list_of ("Controller", Controller) ("SwitchV2P", SwitchV2P) (
        "GwCache", GwCache) ("LocalLearning", LocalLearning) ("NoCache", NoCache) (
        "Direct", Direct) ("Bluebird", Bluebird) ("OnDemand", OnDemand);

const map<string, enum SimulationParameters::Topology> SimulationParameters::simulationTopologyMap =
    boost::assign::map_list_of ("Clos", CLOS) ("Fattree", FATTREE);

const map<string, enum SimulationParameters::TenantCachePolicy>
    SimulationParameters::simulationTenantPolicyMap =
        boost::assign::map_list_of ("Baseline", Baseline) ("StaticPartitioning",
                                                             StaticPartitioning) (
            "DynamicWeightedEviction", DynamicWeightedEviction);

SimulationParameters::SimulationParameters (string simMode, string networkTopology,
                                            size_t numOfPorts, size_t numOfCore, size_t podWidth,
                                            vector<uint32_t> gatewayLeaves, bool randomRouting,
                                            bool gatewayPerFlowLoadBalancing, bool udpMode,
                                            string tenantPolicy, uint32_t tenantCount,
                                            uint32_t premiumTenantId, uint32_t tenantKeyModulo,
                                            double premiumProtectProbability)
    : SimMode (SimulationParameters::simulationModeMap.at (simMode)),
      NetworkTopology (SimulationParameters::simulationTopologyMap.at (networkTopology)),
      NumOfPorts (numOfPorts),
      NumOfCore (numOfCore),
      PodWidth (podWidth),
      GatewayLeaves (gatewayLeaves),
      RandomRouting (randomRouting),
      GatewayPerFlowLoadBalancing (gatewayPerFlowLoadBalancing),
      UdpMode (udpMode),
      TenantPolicy (SimulationParameters::simulationTenantPolicyMap.at (tenantPolicy)),
      TenantCount (std::max (tenantCount, 1u)),
      PremiumTenantId (premiumTenantId % TenantCount),
      TenantKeyModulo (std::max (tenantKeyModulo, 1u)),
      PremiumProtectProbability (std::max (0.0, std::min (1.0, premiumProtectProbability)))
{
}
