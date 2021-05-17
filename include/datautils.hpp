// I re-read your first,
// your second, your third,
//
// look for your small xx,
// feeling absurd.
//
// The codes we send
// arrive with a broken chord.
//   - Carol Ann Duffy, Text

#ifndef MCD_DATAUTILS_HPP
#define MCD_DATAUTILS_HPP

#include "nbt.hpp"
#include "particletypes.hpp"
#include <array>
#include <cstdint>
#include <iostream>
#include <variant>

namespace mcd {

// Big Endian 128-bit uint
struct mc_uuid {
  std::uint64_t msb;
  std::uint64_t lsb;
};

struct mc_position {
  std::int32_t x;
  std::int32_t y;
  std::int32_t z;
};

void enc_byte(std::ostream& dest, const std::uint8_t src);
std::uint8_t dec_byte(std::istream& src);

void enc_be16(std::ostream& dest, std::uint16_t src);
std::uint16_t dec_be16(std::istream& src);
void enc_le16(std::ostream& dest, std::uint16_t src);
std::uint16_t dec_le16(std::istream& src);

void enc_be32(std::ostream& dest, std::uint32_t src);
std::uint32_t dec_be32(std::istream& src);
void enc_le32(std::ostream& dest, std::uint32_t src);
std::uint32_t dec_le32(std::istream& src);

void enc_be64(std::ostream& dest, std::uint64_t src);
std::uint64_t dec_be64(std::istream& src);
void enc_le64(std::ostream& dest, std::uint64_t src);
std::uint64_t dec_le64(std::istream& src);

void enc_bef32(std::ostream& dest, float src);
float dec_bef32(std::istream& src);
void enc_lef32(std::ostream& dest, float src);
float dec_lef32(std::istream& src);

void enc_bef64(std::ostream& dest, double src);
double dec_bef64(std::istream& src);
void enc_lef64(std::ostream& dest, double src);
double dec_lef64(std::istream& src);

enum varnum_fail {
  VARNUM_INVALID = -1, // Impossible varnum, give up
  VARNUM_OVERRUN = -2  // Your buffer is too small, read more
};

int verify_varint(const char* buf, std::size_t max_len);
int verify_varlong(const char* buf, std::size_t max_len);

std::size_t size_varint(std::uint32_t varint);
std::size_t size_varlong(std::uint64_t varlong);

void enc_varint(std::ostream& dest, std::uint64_t src);
std::int64_t dec_varint(std::istream& src);

void enc_string(std::ostream& dest, const std::string& src);
std::string dec_string(std::istream& src);

void enc_uuid(std::ostream& dest, const mc_uuid& src);
mc_uuid dec_uuid(std::istream& src);

void enc_position(std::ostream& dest, const mc_position& src);
mc_position dec_position(std::istream& src);

void enc_buffer(std::ostream& dest, const std::vector<char>& src);
std::vector<char> dec_buffer(std::istream& src, size_t len);

class MCSlot {
public:
  std::uint8_t present;
  std::int32_t item_id;
  std::int8_t item_count;
  std::optional<nbt::TagCompound> nbt_data;

  void encode(std::ostream& dest) const;
  void decode(std::istream& src);
};

class MCParticle {
public:
  std::int32_t type;
  std::int32_t block_state;
  float red;
  float green;
  float blue;
  float scale;
  MCSlot item;

  void encode(std::ostream& dest) const;
  void decode(std::istream& src, mcd::particle_type p_type);
};

class MCSmelting {
public:
  std::string group;
  std::vector<MCSlot> ingredient;
  MCSlot result;
  float experience;
  std::int32_t cook_time;

  void encode(std::ostream& dest) const;
  void decode(std::istream& src);
};

class MCTag {
public:
  std::string tag_name;
  std::vector<int32_t> entries;

  void encode(std::ostream& dest) const;
  void decode(std::istream& src);
};

class MCEntityEquipment {
public:
  struct item {
    std::int8_t slot;
    MCSlot item;
  };
  std::vector<item> equipments;

  void encode(std::ostream& dest) const;
  void decode(std::istream& src);
};

enum metatag_type {
  METATAG_BYTE,
  METATAG_VARINT,
  METATAG_FLOAT,
  METATAG_STRING,
  METATAG_CHAT,
  METATAG_OPTCHAT,
  METATAG_SLOT,
  METATAG_BOOLEAN,
  METATAG_ROTATION,
  METATAG_POSITION,
  METATAG_OPTPOSITION,
  METATAG_DIRECTION,
  METATAG_OPTUUID,
  METATAG_BLOCKID,
  METATAG_NBT,
  METATAG_PARTICLE,
  METATAG_VILLAGERDATA,
  METATAG_OPTVARINT,
  METATAG_POSE
};

class MCEntityMetadata {
public:
  struct metatag {
    std::uint8_t index;
    std::int32_t type;
    // WTF
    std::variant<std::int8_t, std::int32_t, float, std::string,
        std::optional<std::string>, MCSlot, std::array<float, 3>, mc_position,
        std::optional<mc_position>, std::optional<mc_uuid>, nbt::TagCompound,
        MCParticle, std::array<std::int32_t, 3>, std::optional<std::int32_t>>
        value;
  };

  std::vector<metatag> data;

  void encode(std::ostream& dest) const;
  void decode(std::istream& src);
};

} // namespace mcd

#endif
