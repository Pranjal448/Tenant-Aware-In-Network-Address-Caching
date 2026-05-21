#ifndef P4_CACHE_H
#define P4_CACHE_H

#include <algorithm>
#include <map>
#include <deque>
#include <vector>
#include "ns3/network-module.h"
#include "ns3/core-module.h"

using ns3::CRC32Calculate;
using ns3::CreateObject;
using ns3::UniformRandomVariable;
using std::pair;
using std::vector;

template <typename K, typename V>
class P4Cache
{
public:
  enum TenantMode
  {
    BASELINE = 0,
    STATIC_PARTITIONING = 1,
    DYNAMIC_WEIGHTED_EVICTION = 2
  };

  size_t m_cacheSize;
  vector<pair<K, V>> m_array;
  vector<uint8_t> m_bits;
  K m_buffer[2];
  ns3::Ptr<UniformRandomVariable> m_random;

  int m_tenantMode;
  uint32_t m_tenantCount;
  uint32_t m_premiumTenantId;
  uint32_t m_tenantKeyModulo;
  double m_premiumProtectProbability;

  P4Cache ()
      : m_buffer{0, 0},
        m_random (CreateObject<UniformRandomVariable> ()),
        m_tenantMode (BASELINE),
        m_tenantCount (2),
        m_premiumTenantId (0),
        m_tenantKeyModulo (2),
        m_premiumProtectProbability (0.8)
  {
  }

  void
  ConfigureTenantAware (int mode, uint32_t tenantCount, uint32_t premiumTenantId,
                        uint32_t tenantKeyModulo, double premiumProtectProbability)
  {
    m_tenantMode = mode;
    m_tenantCount = std::max (tenantCount, 1u);
    m_premiumTenantId = premiumTenantId % m_tenantCount;
    m_tenantKeyModulo = std::max (tenantKeyModulo, 1u);
    m_premiumProtectProbability = std::max (0.0, std::min (1.0, premiumProtectProbability));
  }

  uint32_t
  GetTenantId (K key) const
  {
    return (static_cast<uint32_t> (key) % m_tenantKeyModulo) % m_tenantCount;
  }

  bool
  IsPremium (K key) const
  {
    return GetTenantId (key) == m_premiumTenantId;
  }

  void
  Setup (int capacity, bool randomHash)
  {
    m_cacheSize = capacity;
    m_array.assign (m_cacheSize, std::make_pair (0, 0));
    m_bits.assign (m_cacheSize, 0);
    for (size_t i = 0; i < m_cacheSize; ++i)
      {
        m_array[i].first = 0;
        m_array[i].second = 0;
        m_bits[i] = 0;
      }

    if (randomHash)
      {
        m_buffer[1] = m_random->GetInteger (0, (K) -1);
      }
  }

  bool
  Get (K key, V &value)
  {
    uint32_t idx = GetIndex (key);

    if (m_array[idx].first == key && m_array[idx].second != 0)
      {
        value = m_array[idx].second;
        m_bits[idx] = 1;
        return true;
      }
    m_bits[idx] = 0;
    return false;
  }

  uint8_t
  GetBit (K key)
  {
    uint32_t idx = GetIndex (key);
    return m_bits[idx];
  }

  bool
  Find (K key)
  {
    uint32_t idx = GetIndex (key);
    return m_array[idx].first == key && m_array[idx].second != 0;
  }

  bool
  Get (K key, V &value, uint8_t &bit)
  {
    uint32_t idx = GetIndex (key);

    if (m_array[idx].first == key && m_array[idx].second != 0)
      {
        value = m_array[idx].second;
        bit = m_bits[idx];
        m_bits[idx] = 1;
        return true;
      }

    return false;
  }

  bool
  PutIfNotEvict (K key, V value)
  {
    uint32_t idx = GetIndex (key);
    if (m_array[idx].first != 0 && m_array[idx].first != key)
      {
        return false;
      }
    m_array[idx].first = key;
    m_array[idx].second = value;
    return true;
  }

  void
  Put (K key, V value)
  {
    pair<K, V> evicted;
    Put (key, value, evicted);
  }

  bool
  Put (K key, V value, pair<K, V> &evicted)
  {
    uint32_t idx = GetIndex (key);
    bool eviction = false;
    if (m_array[idx].first != 0 && m_array[idx].first != key)
      {
        if (m_tenantMode == DYNAMIC_WEIGHTED_EVICTION)
          {
            bool existingPremium = IsPremium (m_array[idx].first);
            bool newPremium = IsPremium (key);
            if (existingPremium && !newPremium &&
                m_random->GetValue (0.0, 1.0) <= m_premiumProtectProbability)
              {
                return false;
              }
          }

        evicted = m_array[idx];
        eviction = true;
      }
    m_array[idx].first = key;
    m_array[idx].second = value;
    m_bits[idx] = 0;
    return eviction;
  }

  void
  Remove (K key)
  {
    uint32_t idx = GetIndex (key);
    if (m_array[idx].first == key)
      {
        m_array[idx].first = 0;
        m_array[idx].second = 0;
        m_bits[idx] = 0;
      }
  }

private:
  uint32_t
  Hash (K key)
  {
    m_buffer[0] = key;
    return CRC32Calculate ((uint8_t *) m_buffer, 2 * sizeof (K));
  }

  pair<uint32_t, uint32_t>
  GetTenantRange (uint32_t tenantId)
  {
    uint32_t base = static_cast<uint32_t> (m_cacheSize) / m_tenantCount;
    uint32_t rem = static_cast<uint32_t> (m_cacheSize) % m_tenantCount;
    uint32_t start = tenantId * base + std::min (tenantId, rem);
    uint32_t width = base + (tenantId < rem ? 1 : 0);
    if (width == 0)
      {
        width = 1;
        start = tenantId % static_cast<uint32_t> (m_cacheSize);
      }
    return std::make_pair (start, width);
  }

  uint32_t
  GetIndex (K key)
  {
    if (m_tenantMode == STATIC_PARTITIONING)
      {
        uint32_t tenantId = GetTenantId (key);
        pair<uint32_t, uint32_t> range = GetTenantRange (tenantId);
        return range.first + (Hash (key) % range.second);
      }

    return Hash (key) % static_cast<uint32_t> (m_cacheSize);
  }
};

#endif /* P4_CACHE_H */
