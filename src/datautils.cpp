// I re-read your first,
// your second, your third,
//
// look for your small xx,
// feeling absurd.
//
// The codes we send
// arrive with a broken chord.
//   - Carol Ann Duffy, Text


#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
#include "datautils.hpp"
#include "byteswap.hpp"

namespace mcd {

void enc_byte(std::ostream &dest, const std::uint8_t src) {
  dest.put(src);
}
std::uint8_t dec_byte(std::istream &src) {
  return src.get();
}

void enc_be16(std::ostream &dest, std::uint16_t src) {
  src = nbeswap(src);
  dest.write(reinterpret_cast<char *>(&src), sizeof(src));
}
std::uint16_t dec_be16(std::istream &src) {
  std::uint16_t dest;
  src.read(reinterpret_cast<char *>(&dest), sizeof(dest));
  return nbeswap(dest);
}
void enc_le16(std::ostream &dest, std::uint16_t src) {
  src = nleswap(src);
  dest.write(reinterpret_cast<char *>(&src), sizeof(src));
}
std::uint16_t dec_le16(std::istream &src) {
  std::uint16_t dest;
  src.read(reinterpret_cast<char *>(&dest), sizeof(dest));
  return nleswap(dest);
}

void enc_be32(std::ostream &dest, std::uint32_t src) {
  src = nbeswap(src);
  dest.write(reinterpret_cast<char *>(&src), sizeof(src));
}
std::uint32_t dec_be32(std::istream &src) {
  std::uint32_t dest;
  src.read(reinterpret_cast<char *>(&dest), sizeof(dest));
  return nbeswap(dest);
}
void enc_le32(std::ostream &dest, std::uint32_t src) {
  src = nleswap(src);
  dest.write(reinterpret_cast<char *>(&src), sizeof(src));
}
std::uint32_t dec_le32(std::istream &src) {
  std::uint32_t dest;
  src.read(reinterpret_cast<char *>(&dest), sizeof(dest));
  return nleswap(dest);
}

void enc_be64(std::ostream &dest, std::uint64_t src) {
  src = nbeswap(src);
  dest.write(reinterpret_cast<char *>(&src), sizeof(src));
}
std::uint64_t dec_be64(std::istream &src) {
  std::uint64_t dest;
  src.read(reinterpret_cast<char *>(&dest), sizeof(dest));
  return nbeswap(dest);
}
void enc_le64(std::ostream &dest, std::uint64_t src) {
  src = nleswap(src);
  dest.write(reinterpret_cast<char *>(&src), sizeof(src));
}
std::uint64_t dec_le64(std::istream &src) {
  std::uint64_t dest;
  src.read(reinterpret_cast<char *>(&dest), sizeof(dest));
  return nleswap(dest);
}

void enc_bef32(std::ostream &dest, float src) {
  std::uint32_t tmp;
  std::memcpy(&tmp, &src, sizeof(tmp));
  tmp = nbeswap(tmp);
  dest.write(reinterpret_cast<char *>(&tmp), sizeof(tmp));
}
float dec_bef32(std::istream &src) {
  std::uint32_t tmp;
  src.read(reinterpret_cast<char *>(&tmp), sizeof(tmp));
  tmp = nbeswap(tmp);
  float dest;
  std::memcpy(&dest, &tmp, sizeof(dest));
  return dest;
}
void enc_lef32(std::ostream &dest, float src) {
  std::uint32_t tmp;
  std::memcpy(&tmp, &src, sizeof(tmp));
  tmp = nleswap(tmp);
  dest.write(reinterpret_cast<char *>(&tmp), sizeof(tmp));
}
float dec_lef32(std::istream &src) {
  std::uint32_t tmp;
  src.read(reinterpret_cast<char *>(&tmp), sizeof(tmp));
  tmp = nleswap(tmp);
  float dest;
  std::memcpy(&dest, &tmp, sizeof(dest));
  return dest;
}

void enc_bef64(std::ostream &dest, double src) {
  std::uint64_t tmp;
  std::memcpy(&tmp, &src, sizeof(tmp));
  tmp = nbeswap(tmp);
  dest.write(reinterpret_cast<char *>(&tmp), sizeof(tmp));
}
double dec_bef64(std::istream &src) {
  std::uint64_t tmp;
  src.read(reinterpret_cast<char *>(&tmp), sizeof(tmp));
  tmp = nbeswap(tmp);
  double dest;
  std::memcpy(&dest, &tmp, sizeof(dest));
  return dest;
}
void enc_lef64(std::ostream &dest, double src) {
  std::uint64_t tmp;
  std::memcpy(&tmp, &src, sizeof(tmp));
  tmp = nleswap(tmp);
  dest.write(reinterpret_cast<char *>(&tmp), sizeof(tmp));
}
double dec_lef64(std::istream &src) {
  std::uint64_t tmp;
  src.read(reinterpret_cast<char *>(&tmp), sizeof(tmp));
  tmp = nleswap(tmp);
  double dest;
  std::memcpy(&dest, &tmp, sizeof(dest));
  return dest;
}

int verify_varnum(const char *buf, std::size_t max_len, int type_max) {
  if(!max_len)
    return VARNUM_OVERRUN;
  int len =  1;
  for(; *reinterpret_cast<const unsigned char*>(buf) & 0x80; buf++, len++) {
    if(len == type_max)
      return VARNUM_INVALID;
    if(static_cast<std::size_t>(len) == max_len)
      return VARNUM_OVERRUN;
  }
  return len;
}

int verify_varint(const char *buf, std::size_t max_len) {
  return verify_varnum(buf, max_len, 5);
}
int verify_varlong(const char *buf, std::size_t max_len) {
  return verify_varnum(buf, max_len, 10);
}

std::size_t size_varint(std::uint32_t varint) {
  if(varint < (1 << 7))
    return 1;
  if(varint < (1 << 14))
    return 2;
  if(varint < (1 << 21))
    return 3;
  if(varint < (1 << 28))
    return 4;
  return 5;
}
std::size_t size_varlong(std::uint64_t varlong) {
  if(varlong < (1 << 7))
    return 1;
  if(varlong < (1 << 14))
    return 2;
  if(varlong < (1 << 21))
    return 3;
  if(varlong < (1 << 28))
    return 4;
  if(varlong < (1ULL << 35))
    return 5;
  if(varlong < (1ULL << 42))
    return 6;
  if(varlong < (1ULL << 49))
    return 7;
  if(varlong < (1ULL << 56))
    return 8;
  if(varlong < (1ULL << 63))
    return 9;
  return 10;
}

void enc_varint(std::ostream &dest, std::uint64_t src) {
  for(; src >= 0x80; src >>= 7)
    dest.put(0x80 | (src & 0x7F));
  dest.put(src & 0x7F);
}
std::int64_t dec_varint(std::istream &src) {
  std::uint64_t dest = 0;
  int i = 0;
  std::uint64_t j = src.get();
  for(; j & 0x80; i+=7, j = src.get())
    dest |= (j & 0x7F) << i;
  dest |= j << i;
  return static_cast<std::int64_t>(dest);
}

void enc_string(std::ostream &dest, std::string src) {
  enc_varint(dest, src.size());
  dest.write(src.data(), src.size());
}
std::string dec_string(std::istream &src) {
  std::string str;
  str.resize(dec_varint(src));
  src.read(str.data(), str.size());
  return str;
}

void enc_uuid(std::ostream &dest, mc_uuid src) {
  enc_be64(dest, src.msb);
  enc_be64(dest, src.lsb);
}
mc_uuid dec_uuid(std::istream &src) {
  mc_uuid dest;
  dest.msb = dec_be64(src);
  dest.lsb = dec_be64(src);
  return dest;
}

// From MSB to LSB x: 26-bits, y: 12-bits, z: 26-bits
// each is an independent signed 2-complement integer
void enc_position(std::ostream &dest, mc_position src) {
  uint64_t tmp = (
    (static_cast<std::uint64_t>(src.x) & 0x3FFFFFFUL) << 38 |
    (static_cast<std::uint64_t>(src.y) & 0xFFFUL) << 26 |
    (static_cast<std::uint64_t>(src.z) & 0x3FFFFFFUL)
  );
  enc_be64(dest, tmp);
}
mc_position dec_position(std::istream &src) {
  mc_position dest;
  std::uint64_t tmp = dec_be64(src);
  if((dest.x = tmp >> 38) & (1UL << 25))
    dest.x -= 1UL << 26;
  if((dest.y = (tmp >> 26) & 0xFFFUL) & (1UL << 11))
    dest.y -= 1UL << 12;
  if((dest.z = tmp & 0x3FFFFFFUL) & (1UL << 25))
    dest.z -= 1UL << 26;
  return dest;
}

void enc_buffer(std::ostream &dest, const std::vector<char> &src) {
  dest.write(src.data(), src.size());
}
std::vector<char> dec_buffer(std::istream &src, size_t len) {
  std::vector<char> vec(len);
  src.read(vec.data(), len);
  return vec;
}

void MCSlot::encode(std::ostream &dest) const {
  enc_byte(dest, present);
  if(present) {
    enc_varint(dest, item_id);
    enc_byte(dest, item_count);
    if(nbt_data)
      nbt_data.value().encode_full(dest);
    else
      enc_byte(dest, nbt::TAG_END);
  }
}

void MCSlot::decode(std::istream &src) {
  present = dec_byte(src);
  if(present) {
    item_id = dec_varint(src);
    item_count = dec_byte(src);
    if(dec_byte(src) == nbt::TAG_COMPOUND)
      nbt_data.emplace(src, nbt::read_string(src));
  }
}

void MCParticle::encode(std::ostream &dest) const {
  switch(type) {
    case PARTICLE_BLOCK:
    case PARTICLE_FALLING_DUST:
      enc_varint(dest, block_state);
      break;
    case PARTICLE_DUST:
      enc_be32(dest, red);
      enc_be32(dest, green);
      enc_be32(dest, blue);
      enc_be32(dest, scale);
      break;
    case PARTICLE_ITEM:
      item.encode(dest);
      break;
  }
}

void MCParticle::decode(std::istream &src, particle_type p_type) {
  type = p_type;
  switch (type) {
    case PARTICLE_BLOCK:
    case PARTICLE_FALLING_DUST:
      block_state = dec_varint(src);
      break;
    case PARTICLE_DUST:
      red = dec_be32(src);
      green = dec_be32(src);
      blue = dec_be32(src);
      scale = dec_be32(src);
      break;
    case PARTICLE_ITEM:
      item.decode(src);
      break;
  }
}

void MCSmelting::encode(std::ostream &dest) const {
  enc_string(dest, group);
  enc_varint(dest, ingredient.size());
  for(auto &el : ingredient)
    el.encode(dest);
  result.encode(dest);
  enc_bef32(dest, experience);
  enc_varint(dest, cook_time);
}

void MCSmelting::decode(std::istream &src) {
  group = dec_string(src);
  ingredient.resize(dec_varint(src));
  for(auto &el : ingredient)
    el.decode(src);
  result.decode(src);
  experience = dec_bef32(src);
  cook_time = dec_varint(src);
}

void MCTag::encode(std::ostream &dest) const {
  enc_string(dest, tag_name);
  enc_varint(dest, entries.size());
  for(auto &el : entries)
    enc_varint(dest, el);
}

void MCTag::decode(std::istream &src) {
  tag_name = dec_string(src);
  entries.resize(dec_varint(src));
  for(auto &el : entries)
    el = dec_varint(src);
}

void MCEntityEquipment::encode(std::ostream &dest) const {
  for(auto el = equipments.begin(); el != --equipments.end(); ++el) {
    enc_byte(dest, 0x80 | el->slot);
    el->item.encode(dest);
  }
  enc_byte(dest, equipments.back().slot);
  equipments.back().item.encode(dest);
}

void MCEntityEquipment::decode(std::istream &src) {
  equipments.clear();
  std::uint8_t slot;
  do {
    slot = dec_byte(src);
    equipments.emplace_back();
    equipments.back().slot = slot & 0x7F;
    equipments.back().item.decode(src);
  } while(slot & 0x80);
}

void MCEntityMetadata::encode(std::ostream &dest) const {
  for(auto &el : data) {
    enc_byte(dest, el.index);
    enc_varint(dest, el.type);
    switch(el.type) {
      case METATAG_BYTE:
      case METATAG_BOOLEAN:
        enc_byte(dest, std::get<std::int8_t>(el.value));
        break;
      case METATAG_VARINT:
      case METATAG_DIRECTION:
      case METATAG_BLOCKID:
      case METATAG_POSE:
        enc_varint(dest, std::get<std::int32_t>(el.value));
        break;
      case METATAG_FLOAT:
        enc_bef32(dest, std::get<float>(el.value));
        break;
      case METATAG_STRING:
      case METATAG_CHAT:
        enc_string(dest, std::get<std::string>(el.value));
        break;
      case METATAG_OPTCHAT: {
        auto str = std::get<std::optional<std::string>>(el.value);
        enc_byte(dest, str.has_value());
        if(str.has_value())
          enc_string(dest, str.value());
      }
        break;
      case METATAG_SLOT:
        std::get<MCSlot>(el.value).encode(dest);
        break;
      case METATAG_ROTATION:
        for(auto& el : std::get<std::array<float, 3>>(el.value))
          enc_bef32(dest, el);
      case METATAG_POSITION:
        enc_position(dest, std::get<mc_position>(el.value));
        break;
      case METATAG_OPTPOSITION: {
        auto pos = std::get<std::optional<mc_position>>(el.value);
        enc_byte(dest, pos.has_value());
        if(pos.has_value())
          enc_position(dest, pos.value());
      }
        break;
      case METATAG_OPTUUID: {
        auto uuid = std::get<std::optional<mc_uuid>>(el.value);
        enc_byte(dest, uuid.has_value());
        if(uuid.has_value())
          enc_uuid(dest, uuid.value());
      }
        break;
      case METATAG_NBT:
        std::get<nbt::TagCompound>(el.value).encode_full(dest);
        break;
      case METATAG_PARTICLE:
        enc_varint(dest, std::get<MCParticle>(el.value).type);
        std::get<MCParticle>(el.value).encode(dest);
        break;
      case METATAG_VILLAGERDATA:
        for(auto& el : std::get<std::array<std::int32_t, 3>>(el.value))
          enc_varint(dest, el);
        break;
      case METATAG_OPTVARINT: {
        auto varint = std::get<std::optional<std::int32_t>>(el.value);
        enc_byte(dest, varint.has_value());
        if(varint.has_value())
          enc_varint(dest, varint.value());
      }
        break;
    }
  }
  enc_byte(dest, 0xFF);
}

void MCEntityMetadata::decode(std::istream &src) {
  data.clear();
  std::uint8_t index = dec_byte(src);
  while(index != 0xFF) {
    auto& tag = data.emplace_back();
    tag.index = index;
    tag.type = dec_varint(src);
    switch(tag.type) {
      case METATAG_BYTE:
      case METATAG_BOOLEAN:
        tag.value = dec_byte(src);
        break;
      case METATAG_VARINT:
      case METATAG_DIRECTION:
      case METATAG_BLOCKID:
      case METATAG_POSE:
        // Not sure why this is considered ambiguous, maybe conflict with int8?
        tag.value.emplace<std::int32_t>(dec_varint(src));
        break;
      case METATAG_FLOAT:
        tag.value = dec_bef32(src);
        break;
      case METATAG_STRING:
      case METATAG_CHAT:
        tag.value = dec_string(src);
        break;
      case METATAG_OPTCHAT: {
        auto& str = tag.value.emplace<std::optional<std::string>>();
        if(dec_byte(src))
          str = dec_string(src);
      }
        break;
      case METATAG_SLOT: {
        auto& slot = tag.value.emplace<MCSlot>();
        slot.decode(src);
      }
        break;
      case METATAG_ROTATION: {
        auto& rot = tag.value.emplace<std::array<float, 3>>();
        for(auto &el : rot)
          el = dec_bef32(src);
      }
        break;
      case METATAG_POSITION:
        tag.value = dec_position(src);
        break;
      case METATAG_OPTPOSITION: {
        auto& pos = tag.value.emplace<std::optional<mc_position>>();
        if(dec_byte(src))
          pos = dec_position(src);
      }
        break;
      case METATAG_OPTUUID: {
        auto& uuid = tag.value.emplace<std::optional<mc_uuid>>();
        if(dec_byte(src))
          uuid = dec_uuid(src);
      }
        break;
      case METATAG_NBT: {
        auto& nbt_tag = tag.value.emplace<nbt::TagCompound>();
        nbt_tag.decode_full(src);
      }
        break;
      case METATAG_PARTICLE: {
        auto& particle = tag.value.emplace<MCParticle>();
        particle.decode(src, static_cast<particle_type>(dec_varint(src)));
      }
        break;
      case METATAG_VILLAGERDATA: {
        auto& data = tag.value.emplace<std::array<std::int32_t, 3>>();
        for(auto &el : data)
          el = dec_varint(src);
      }
        break;
      case METATAG_OPTVARINT: {
        auto& varint = tag.value.emplace<std::optional<std::int32_t>>();
        if(dec_byte(src))
          varint = dec_varint(src);
      }
        break;
    }
    index = dec_byte(src);
  }
}

}  // namespace rkr
