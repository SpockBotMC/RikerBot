#include "smpmap.hpp"
#include "datautils.hpp"

namespace rkr {

ChunkSection::ChunkSection(std::istream& data) {
  update(data);
}

void ChunkSection::update(std::istream& data) {
  block_count = mcd::dec_be16(data);
  std::uint8_t bits_per_block = mcd::dec_byte(data);
  if(bits_per_block < 8) {
    if(bits_per_block < 4)
      bits_per_block = 4;
    update_palette(data, bits_per_block);
  } else {
    update_direct(data, bits_per_block);
  }
}

void ChunkSection::update_direct(std::istream& data,
    std::uint8_t bits_per_block) {
  std::int32_t len = mcd::dec_varint(data);
  int bits_remaining = 0;
  std::uint64_t cur_long;
  std::uint64_t mask = (1UL << bits_per_block) - 1;
  for(auto& el : blocks) {
    if(bits_remaining < bits_per_block) {
      cur_long = mcd::dec_be64(data);
      bits_remaining = 64;
      len--;
    }
    el = static_cast<block_id>(cur_long&mask);
    cur_long >>= bits_per_block;
    bits_remaining -= bits_per_block;
  }
  // There's some situations where there's rando garbage at the end of the
  // array
  if(len)
    data.ignore(len * sizeof(cur_long));
}

void ChunkSection::update_palette(std::istream& data,
    std::uint8_t bits_per_block) {
  std::vector<block_id> palette(mcd::dec_varint(data));
  for(auto& el : palette)
    el = static_cast<block_id>(mcd::dec_varint(data));
  std::int32_t len = mcd::dec_varint(data);
  int bits_remaining = 0;
  std::uint64_t cur_long;
  std::uint64_t mask = (1UL << bits_per_block) - 1;
  for(auto& el : blocks) {
    if(bits_remaining < bits_per_block) {
      cur_long = mcd::dec_be64(data);
      bits_remaining = 64;
      len--;
    }
    el = palette[cur_long&mask];
    cur_long >>= bits_per_block;
    bits_remaining -= bits_per_block;
  }
  if(len)
    data.ignore(len * sizeof(cur_long));
}

void ChunkColumn::update(std::uint16_t bitmask, std::istream& data) {
  for(unsigned int i = 0; i < std::size(sections); i++)
    if(bitmask & (1<<i)) {
      if(sections[i])
        sections[i].value().update(data);
      else
        sections[i].emplace(data);
    }
}


void SMPMap::update(const mcd::ClientboundMapChunk& packet) {
  boost::interprocess::ibufferstream ibuf(packet.chunkData.data(),
      packet.chunkData.size());
    chunks[{packet.x, packet.z}].update(packet.bitMap, ibuf);
}

void SMPMap::unload(const mcd::ClientboundUnloadChunk& packet) {
  chunks.erase({packet.chunkX, packet.chunkZ});
}

} // namespace rkr
