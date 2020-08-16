#ifndef RKR_SMPMAP_HPP
#define RKR_SMPMAP_HPP

#include <cstdint>
#include <unordered_map>
#include <optional>
#include <utility>
#include <array>

#include <boost/functional/hash.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>

#include <minecraft_protocol.hpp>

namespace rkr {

typedef std::uint16_t block_id;
typedef std::pair<std::int64_t, std::int64_t> chunk_coord;

// This is specifically for 1.16.2+ SMPMap format
class SMPMap;
class ChunkColumn;

class ChunkSection {
public:
  ChunkSection() = default;
  ChunkSection(std::istream& data);

private:
  friend class ChunkColumn;

  std::uint16_t block_count;

  // x, z, y ordered x + (z*16) + y*16*16
  std::array<block_id, 16*16*16> blocks;
  void update(std::istream& data);
  void update_palette(std::istream& data, std::uint8_t bits_per_block);
  void update_direct(std::istream& data, std::uint8_t bits_per_block);
};

class ChunkColumn {
  friend class SMPMap;

  // sections[0], y=0-15 -> sections[15], y=240-255
  std::array<std::optional<ChunkSection>, 16> sections;

  void update(std::uint16_t bitmask, std::istream& data);
};

class SMPMap {
public:
  void update(const mcd::ClientboundMapChunk& packet);
  void update(const mcd::ClientboundMultiBlockChange& packet);
  void unload(const mcd::ClientboundUnloadChunk& packet);

private:
  std::unordered_map<chunk_coord, ChunkColumn, boost::hash<chunk_coord>>
      chunks;

};

} // namespace rkr

#endif // RKR_SMPMAP_HPP
