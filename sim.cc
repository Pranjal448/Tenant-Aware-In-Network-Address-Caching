#include "include/migration-params.h"
#include "include/trace-sim.h"
#include <boost/algorithm/string.hpp>
int
main (int argc, char *argv[])
{
  string placementFile = "simple_placement.json";
  string traceFile = "simple_trace.csv";
  string outputFile = "output.json";
  string simModeArg = "Controller";
  string topologyArg = "Clos";
  size_t numOfPorts = 64;
  size_t numOfCore = 16;
  size_t podWidth = 4;
  string gwLeavesStr = "0";
  bool migrationTest = false, randomRouting = false, gatewayPerFlowLoadBalancing = false;
  uint32_t migrationContainerId = 0, migrationDstLeaf = 0, migrationDstHost = 0, migrationTs = 0;
  bool udpMode = false;
  string tenantPolicyArg = "Baseline";
  uint32_t tenantCount = 2;
  uint32_t premiumTenantId = 0;
  uint32_t tenantKeyModulo = 2;
  double premiumProtectProbability = 0.8;
  CommandLine cmd;
  cmd.AddValue ("placement", "The JSON placement file for the simulation", placementFile);
  cmd.AddValue ("trace", "The CSV trace file for the simulation", traceFile);
  cmd.AddValue ("output", "The output JSON file for the simulation", outputFile);
  cmd.AddValue ("simMode", "The simulation mode", simModeArg);
  cmd.AddValue ("topology", "The network topology", topologyArg);
  cmd.AddValue ("ports", "The number of ports per switch", numOfPorts);
  cmd.AddValue ("core", "The number of core switches", numOfCore);
  cmd.AddValue ("podWidth", "The width of each pod", podWidth);
  cmd.AddValue ("gwLeaves", "Gateway leaf indices", gwLeavesStr);
  cmd.AddValue ("migration", "Enable migration test", migrationTest);
  cmd.AddValue ("migrationTs", "The migration time", migrationTs);
  cmd.AddValue ("migrationContainerId", "The container ID to migrate", migrationContainerId);
  cmd.AddValue ("migrationDestinationLeaf", "The destination leaf id for migration",
                migrationDstLeaf);
  cmd.AddValue ("migrationDestinationHost", "The destination host id for migration",
                migrationDstHost);
  cmd.AddValue ("randomRouting", "Enable per-packet random routing", randomRouting);
  cmd.AddValue ("gatewayPerFlowLoadBalancing", "Enable per-flow gateway balancing",
                gatewayPerFlowLoadBalancing);
  cmd.AddValue ("udpMode", "Send UDP packets", udpMode);
  cmd.AddValue ("tenantPolicy",
                "Tenant-aware cache policy: Baseline | StaticPartitioning | DynamicWeightedEviction",
                tenantPolicyArg);
  cmd.AddValue ("tenantCount", "Number of active tenants", tenantCount);
  cmd.AddValue ("premiumTenantId", "Premium tenant identifier", premiumTenantId);
  cmd.AddValue ("tenantKeyModulo", "Tenant extraction modulo used on virtual destination key",
                tenantKeyModulo);
  cmd.AddValue ("premiumProtectProbability",
                "Dynamic weighted eviction protection probability for premium entries",
                premiumProtectProbability);
  cmd.Parse (argc, argv);

  int32_t tenantPolicyValue = static_cast<int32_t> (SimulationParameters::Baseline);
  if (tenantPolicyArg == "StaticPartitioning")
    {
      tenantPolicyValue = static_cast<int32_t> (SimulationParameters::StaticPartitioning);
    }
  else if (tenantPolicyArg == "DynamicWeightedEviction")
    {
      tenantPolicyValue = static_cast<int32_t> (SimulationParameters::DynamicWeightedEviction);
    }

  Config::SetDefault ("P4SwitchApp::TenantPolicy", IntegerValue (tenantPolicyValue));
  Config::SetDefault ("P4SwitchApp::TenantCount", UintegerValue (tenantCount));
  Config::SetDefault ("P4SwitchApp::PremiumTenantId", UintegerValue (premiumTenantId));
  Config::SetDefault ("P4SwitchApp::TenantKeyModulo", UintegerValue (tenantKeyModulo));
  Config::SetDefault ("P4SwitchApp::PremiumProtectProbability",
                      DoubleValue (premiumProtectProbability));

  vector<string> gwLeavesStrs;
  boost::split (gwLeavesStrs, gwLeavesStr, boost::is_any_of (","));
  vector<uint32_t> gwLeaves;
  std::transform (gwLeavesStrs.begin (), gwLeavesStrs.end (), back_inserter (gwLeaves),
                  [] (const string &astr) { return stoi (astr); });
  TraceSimulation (placementFile, traceFile,
                   SimulationParameters (simModeArg, topologyArg, numOfPorts, numOfCore, podWidth,
                                         gwLeaves, randomRouting, gatewayPerFlowLoadBalancing,
                                         udpMode, tenantPolicyArg, tenantCount,
                                         premiumTenantId, tenantKeyModulo,
                                         premiumProtectProbability),
                   outputFile,
                   MigrationParams (migrationTest, migrationContainerId, migrationDstLeaf,
                                    migrationDstHost, migrationTs))
      .Run ();
  return 0;
}
