#include "smpmap.hpp"
#include "datautils.hpp"

namespace rkr {

ChunkSection::ChunkSection(std::istream& data) {
  update(data);
}

void ChunkSection::update(std::istream& data) {
  block_count = mcd::dec_be16(data);
  std::uint8_t bits_per_block = mcd::dec_byte(data);
  
}

void ChunkColumn::update(std::uint16_t bitmask, std::istream& data) {
  for(int i = 0; i < std::size(sections); i++)
    if(bitmask & (1<<i))
      if(sections[i])
        sections[i].value().update(data);
      else
        sections[i].emplace(data);
}


void SMPMap::update(const mcd::ClientboundMapChunk& packet) {
  boost::interprocess::ibufferstream ibuf(packet.chunkData.data(),
      packet.chunkData.size());
    chunks[{packet.x, packet.z}].update(packet.bitMap, ibuf);
}

} // namespace rkr
