#ifndef RKR_SMPMAP_HPP
#define RKR_SMPMAP_HPP

#include <cstdint>
#include <unordered_map>
#include <optional>
#include <utility>
#include <array>
#include <shared_mutex>

#include <boost/functional/hash.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>

#include "minecraft_protocol.hpp"

namespace rkr {

typedef std::uint16_t block_id;
typedef std::pair<std::int64_t, std::int64_t> ChunkCoord;
typedef mcd::mc_position BlockCoord;

// This is specifically for 1.16.2+ SMPMap format
class SMPMap;
class ChunkColumn;

class ChunkSection {
public:
  ChunkSection() = default;
  ChunkSection(std::istream& data);

private:
  friend class ChunkColumn;

  // x, z, y ordered x + (z*16) + y*16*16
  std::array<block_id, 16*16*16> blocks;
  void update(std::istream& data);
  void update(std::uint8_t x, std::uint8_t y, std::uint8_t z, block_id block);
  void update_palette(std::istream& data, std::uint8_t bits_per_block);
  void update_direct(std::istream& data, std::uint8_t bits_per_block);
  block_id get(std::uint8_t x, std::uint8_t y, std::uint8_t z) const;
};

class ChunkColumn {
  friend class SMPMap;

  // sections[0], y=0-15 -> sections[15], y=240-255
  std::array<std::optional<ChunkSection>, 16> sections;

  void update(std::uint16_t bitmask, std::istream& data);
  void update(std::uint8_t sec_coord, const std::vector<std::int64_t>&
      records);
  void update(std::uint8_t x, std::uint8_t y, std::uint8_t z, block_id block);

  block_id get(std::int32_t x, std::int32_t y, std::int32_t z) const;
  std::vector<std::pair<block_id, std::int32_t>> get(
      std::vector<std::array<std::int32_t, 4>>& coords) const;
};

class SMPMap {
public:
  void update(const mcd::ClientboundMapChunk& packet);
  void update(const mcd::ClientboundMultiBlockChange& packet);
  void update(const mcd::ClientboundBlockChange& packet);
  void unload(const mcd::ClientboundUnloadChunk& packet);

  block_id get(const rkr::BlockCoord& coord) const;
  block_id get(std::int32_t x, std::int32_t y, std::int32_t z) const;

  std::vector<block_id> get(const std::vector<rkr::BlockCoord>& coords) const;
  std::vector<block_id> get(const std::vector<std::array<std::int32_t, 3>>&
      coords) const;

private:
  std::unordered_map<ChunkCoord, ChunkColumn, boost::hash<ChunkCoord>>
      chunks;
  mutable std::shared_mutex mutex;

};

} // namespace rkr

#endif // RKR_SMPMAP_HPP
