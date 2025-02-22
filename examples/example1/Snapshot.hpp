#ifndef SNAPSHOT_HPP
#define SNAPSHOT_HPP

#include "Common.hpp"

enum class SnapshotFormat {
    Json,
    Capnproto
};

std::vector<gerium_uint8_t> makeSnapshot(const entt::registry& registry,
                                         const std::map<entt::hashed_string, std::vector<uint8_t>>& states,
                                         SnapshotFormat format);

bool loadSnapshot(entt::registry& registry,
                  std::map<hashed_string_owner, std::vector<uint8_t>>& states,
                  SnapshotFormat format,
                  const std::vector<gerium_uint8_t>& data);

#endif // SNAPSHOT_HPP
