#include <mutex>

#include "datautils.hpp"
#include "smpmap.hpp"

namespace rkr {

ChunkSection::ChunkSection(std::istream& data) {
  update(data);
}

void ChunkSection::update(std::istream& data) {
  mcd::dec_be16(data); // Block Count, used for lighting, unused by us
  std::uint8_t bits_per_block {mcd::dec_byte(data)};
  if(bits_per_block < 8)
    update_palette(data, bits_per_block < 4 ? 4 : bits_per_block);
  else
    update_direct(data, bits_per_block);
}

void ChunkSection::update_direct(
    std::istream& data, std::uint8_t bits_per_block) {
  auto len {static_cast<std::int32_t>(mcd::dec_varint(data))};
  int bits_remaining {0};
  std::uint64_t cur_long {0};
  std::uint64_t mask {(1UL << bits_per_block) - 1};

  for(auto& block : blocks) {
    if(bits_remaining < bits_per_block) {
      cur_long = mcd::dec_be64(data);
      bits_remaining = 64;
      len--;
    }
    block = static_cast<block_id>(cur_long & mask);
    cur_long >>= bits_per_block;
    bits_remaining -= bits_per_block;
  }

  // There's some situations where there's rando garbage at the end of the
  // array
  if(len)
    data.ignore(len * sizeof(cur_long));
}

void ChunkSection::update_palette(
    std::istream& data, std::uint8_t bits_per_block) {
  std::vector<block_id> palette(mcd::dec_varint(data));
  for(auto& indexed_block : palette)
    indexed_block = static_cast<block_id>(mcd::dec_varint(data));

  auto len {static_cast<std::int32_t>(mcd::dec_varint(data))};
  int bits_remaining {0};
  std::uint64_t cur_long {0};
  std::uint64_t mask {(1UL << bits_per_block) - 1};

  for(auto& block : blocks) {
    if(bits_remaining < bits_per_block) {
      cur_long = mcd::dec_be64(data);
      bits_remaining = 64;
      len--;
    }
    block = palette[cur_long & mask];
    cur_long >>= bits_per_block;
    bits_remaining -= bits_per_block;
  }

  if(len)
    data.ignore(len * sizeof(cur_long));
}

void ChunkSection::update(
    std::uint8_t x, std::uint8_t y, std::uint8_t z, block_id block) {
  blocks[x + (z + y * 16) * 16] = block;
}

block_id ChunkSection::get(
    std::uint8_t x, std::uint8_t y, std::uint8_t z) const {
  return blocks[x + (z + y * 16) * 16];
}

void ChunkColumn::update(std::uint16_t bitmask, std::istream& data) {
  for(unsigned int i = 0; i < std::size(sections); i++)
    if(bitmask & (1 << i)) {
      if(sections[i])
        sections[i]->update(data);
      else
        sections[i].emplace(data);
    }
}

void ChunkColumn::update(
    std::uint8_t sec_coord, const std::vector<std::int64_t>& records) {
  for(auto rec : records) {
    auto y {static_cast<std::uint8_t>(rec & 0xF)};
    auto z {static_cast<std::uint8_t>((rec >>= 4) & 0xF)};
    auto x {static_cast<std::uint8_t>((rec >>= 4) & 0xF)};
    auto block {static_cast<block_id>(rec >> 4)};
    sections[sec_coord]->update(x, y, z, block);
  }
}

void ChunkColumn::update(
    std::uint8_t x, std::uint8_t y, std::uint8_t z, block_id block) {
  auto& section {sections[y >> 4]};
  if(!section)
    section.emplace();
  section->update(x, y & 0xF, z, block);
}

block_id ChunkColumn::get(
    std::int32_t x, std::int32_t y, std::int32_t z) const {
  const auto& section {sections[y >> 4]};
  if(section)
    return section->get(x, y & 0xF, z);
  else
    return 0;
}

IndexedBlockVec ChunkColumn::get(const IndexedCoordVec& positions) const {
  IndexedBlockVec ret(positions.size());
  for(int i = 0, end = positions.size(); i < end; i++) {
    const auto& pos {positions[i]};
    const auto& section {sections[pos[1] >> 4]};
    // ToDo: Group all requests inside a given section together so we don't
    // repeat this check
    if(section)
      ret[i] = {section->get(pos[0], pos[1] & 0xF, pos[2]), pos[3]};
    else
      ret[i] = {0, pos[3]};
  }
  return ret;
}

void SMPMap::update(const mcd::ClientboundMapChunk& packet) {
  std::unique_lock lock {mutex};
  boost::interprocess::ibufferstream ibuf(
      packet.chunkData.data(), packet.chunkData.size());
  chunks[{packet.x, packet.z}].update(packet.bitMap, ibuf);
}

void SMPMap::update(const mcd::ClientboundMultiBlockChange& packet) {
  std::unique_lock lock {mutex};
  chunks[{packet.chunkCoordinates.x, packet.chunkCoordinates.z}].update(
      packet.chunkCoordinates.y, packet.records);
}

void SMPMap::update(const mcd::ClientboundBlockChange& packet) {
  std::unique_lock lock {mutex};
  chunks[{packet.location.x >> 4, packet.location.z >> 4}].update(
      packet.location.x & 0xF, packet.location.y, packet.location.z & 0xF,
      packet.type);
}

void SMPMap::unload(const mcd::ClientboundUnloadChunk& packet) {
  std::unique_lock lock {mutex};
  chunks.erase({packet.chunkX, packet.chunkZ});
}

block_id SMPMap::get(std::int32_t x, std::int32_t y, std::int32_t z) const {
  std::shared_lock lock {mutex};
  return get({x, y, z});
}

block_id SMPMap::get(const BlockCoord& coord) const {
  std::shared_lock lock {mutex};
  if(auto itr {chunks.find({coord.x >> 4, coord.z >> 4})}; itr != chunks.end())
    return itr->second.get(coord.x & 15, coord.y, coord.z & 15);
  return 0;
}

BlockVec SMPMap::get(const std::vector<BlockCoord>& coords) const {
  std::shared_lock lock {mutex};
  BlockVec ret(coords.size());
  std::unordered_map<ChunkCoord, IndexedCoordVec, boost::hash<ChunkCoord>> map;

  for(std::int32_t i = 0, end = coords.size(); i < end; i++) {
    auto& block_coord = coords[i];
    map[{block_coord.x >> 4, block_coord.z >> 4}].push_back(
        {block_coord.x & 0xF, block_coord.y, block_coord.z & 0xF, i});
  }

  for(const auto& [chunk_coord, pos_vec] : map) {
    if(auto itr {chunks.find(chunk_coord)}; itr != chunks.end())
      for(const auto& [block_id, idx] : itr->second.get(pos_vec))
        ret[idx] = block_id;
    else
      for(const auto& coord : pos_vec)
        ret[coord[3]] = 0;
  }
  return ret;
}

BlockVec SMPMap::get(const CoordVec& coords) const {
  std::shared_lock lock {mutex};
  BlockVec ret(coords.size());
  std::unordered_map<ChunkCoord, IndexedCoordVec, boost::hash<ChunkCoord>> map;

  for(std::int32_t i = 0, end = coords.size(); i < end; i++) {
    const auto& block_coord {coords[i]};
    map[{block_coord[0] >> 4, block_coord[2] >> 4}].push_back(
        {block_coord[0] & 0xF, block_coord[1], block_coord[2] & 0xF, i});
  }

  for(const auto& [chunk_coord, pos_vec] : map) {
    auto iter = chunks.find(chunk_coord);
    if(iter != chunks.end())
      for(const auto& [block_id, idx] : iter->second.get(pos_vec))
        ret[idx] = block_id;
    else
      for(const auto& coord : pos_vec)
        ret[coord[3]] = 0;
  }
  return ret;
}

} // namespace rkr
