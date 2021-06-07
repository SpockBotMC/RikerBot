#ifndef NBT_HPP
#define NBT_HPP

#include "byteswap.hpp"
#include <bit>
#include <concepts>
#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace nbt {

inline static std::string indent_step {"  "};

enum TagType {
  TAG_END,
  TAG_BYTE,
  TAG_SHORT,
  TAG_INT,
  TAG_LONG,
  TAG_FLOAT,
  TAG_DOUBLE,
  TAG_BYTE_ARRAY,
  TAG_STRING,
  TAG_LIST,
  TAG_COMPOUND,
  TAG_INT_ARRAY,
  TAG_LONG_ARRAY
};

typedef std::nullptr_t TagEnd;

template <std::integral T> T decode(std::istream& buf) {
  T val;
  buf.read(reinterpret_cast<char*>(&val), sizeof(val));
  return nbeswap(val);
}

template <std::integral T> void encode(std::ostream& buf, const T val) {
  std::make_unsigned_t<T> out {nbeswap(val)};
  buf.write(reinterpret_cast<char*>(&out), sizeof(out));
}

typedef std::int8_t TagByte;
typedef std::int16_t TagShort;
typedef std::int32_t TagInt;
typedef std::int64_t TagLong;

template <std::floating_point T> T decode(std::istream& buf) {
  std::conditional_t<sizeof(T) <= sizeof(TagInt), TagInt, TagLong> in;
  buf.read(reinterpret_cast<char*>(&in), sizeof(in));
  in = nbeswap(in);
  return std::bit_cast<T, decltype(in)>(in);
}

template <std::floating_point T> void encode(std::ostream& buf, const T val) {
  std::conditional_t<sizeof(T) <= sizeof(TagInt), TagInt, TagLong> out {
      std::bit_cast<decltype(out), T>(val)};
  out = nbeswap(out);
  buf.write(reinterpret_cast<char*>(&out), sizeof(out));
}

typedef float TagFloat;
typedef double TagDouble;

template <std::integral T> std::vector<T> decode_array(std::istream& buf) {
  std::int32_t len;
  buf.read(reinterpret_cast<char*>(&len), sizeof(len));
  std::vector<T> vec(nbeswap(len));
  for(auto& el : vec) {
    buf.read(reinterpret_cast<char*>(&el), sizeof(el));
    el = nbeswap(el);
  }
  return vec;
}

template <std::integral T>
void encode_array(std::ostream& buf, const std::vector<T>& vec) {
  std::uint32_t len {nbeswap(static_cast<std::int32_t>(vec.size()))};
  buf.write(reinterpret_cast<char*>(&len), sizeof(len));
  for(auto el : vec) {
    el = nbeswap(el);
    buf.write(reinterpret_cast<char*>(&el), sizeof(el));
  }
}

void print_array(std::ostream& os, const auto& vec) {
  os << "{";
  if(const size_t size {vec.size()}; size) {
    os << vec[0];
    for(int i {1}; i < std::min(static_cast<int>(size), 3); i++)
      os << ", " << vec[i];
    if(size > 3)
      os << ", and " << size - 3 << " more";
  }
  os << "}";
}

typedef std::vector<TagByte> TagByteArray;
typedef std::vector<TagInt> TagIntArray;
typedef std::vector<TagLong> TagLongArray;

typedef std::string TagString;
inline TagString decode_string(std::istream& buf) {
  std::int16_t len {decode<TagShort>(buf)};
  std::string str(len, '\0');
  buf.read(str.data(), len);
  return str;
}

inline void encode_string(std::ostream& buf, const TagString& str) {
  encode<TagShort>(buf, str.size());
  buf.write(str.data(), str.size());
}

// clang-format off
#define ALL_NUMERIC(macro)                                                    \
  macro(TAG_BYTE, TagByte)                                                    \
  macro(TAG_SHORT, TagShort)                                                  \
  macro(TAG_INT, TagInt)                                                      \
  macro(TAG_LONG, TagLong)                                                    \
  macro(TAG_FLOAT, TagFloat)                                                  \
  macro(TAG_DOUBLE, TagDouble)

#define ALL_ARRAYS(macro)                                                     \
  macro(TAG_BYTE_ARRAY, TagByteArray, TagByte)                                \
  macro(TAG_INT_ARRAY, TagIntArray, TagInt)                                   \
  macro(TAG_LONG_ARRAY, TagLongArray, TagLong)


#define ALL_OTHERS(macro)                                                     \
  macro(TAG_STRING, TagString, _string)                                       \
  macro(TAG_LIST, TagList, _list)                                             \
  macro(TAG_COMPOUND, TagCompound, _compound)
// clang-format on

struct TagList;
struct TagCompound;

typedef std::variant<TagEnd, TagByte, TagShort, TagInt, TagLong, TagFloat,
    TagDouble, TagByteArray, TagString, TagList, TagCompound, TagIntArray,
    TagLongArray>
    Tag;

struct TagCompound {
  // std::map of an incomplete type is technically UB, but as a practical
  // matter works everywhere. The same cannot be said of std::unordered_map
  std::map<std::string, Tag> base;
};

TagCompound decode_compound(std::istream& buf);
void encode_compound(std::ostream& buf, const TagCompound& map);
void print_compound(
    std::ostream& os, const std::string& indent, const TagCompound& map);

struct TagList {
  TagList() : base {} {};
  TagList(const auto& base) : base {base} {};

  std::variant<std::vector<TagEnd>, std::vector<TagByte>,
      std::vector<TagShort>, std::vector<TagInt>, std::vector<TagLong>,
      std::vector<TagFloat>, std::vector<TagDouble>, std::vector<TagByteArray>,
      std::vector<TagString>, std::vector<TagList>, std::vector<TagCompound>,
      std::vector<TagIntArray>, std::vector<TagLongArray>>
      base;

  size_t index() const {
    return base.index();
  }

  TagList& operator=(const auto& other) {
    base = other;
    return *this;
  }
};

template <typename T> const T& get(const TagList& list) {
  return std::get<T>(list.base);
}
template <typename T> T& get(TagList& list) {
  return std::get<T>(list.base);
}

inline TagList decode_list(std::istream& buf) {
  std::int8_t type {decode<TagByte>(buf)};
  std::int32_t len {decode<TagInt>(buf)};
  if(len <= 0)
    return {};

  switch(type) {
    case TAG_END:
      return {};

#define X(enum, type)                                                         \
  case(enum): {                                                               \
    std::vector<type> vec(len);                                               \
    for(auto& val : vec)                                                      \
      val = decode<type>(buf);                                                \
    return vec;                                                               \
  };
      ALL_NUMERIC(X)
#undef X

#define X(enum, type, base_type)                                              \
  case(enum): {                                                               \
    std::vector<TagByteArray> vec(len);                                       \
    for(auto& val : vec)                                                      \
      val = decode_array<TagByte>(buf);                                       \
    return vec;                                                               \
  };
      ALL_ARRAYS(X)
#undef X

#define X(enum, type, ext)                                                    \
  case enum: {                                                                \
    std::vector<type> vec(len);                                               \
    for(auto& val : vec)                                                      \
      val = decode##ext(buf);                                                 \
    return vec;                                                               \
  };
      ALL_OTHERS(X)
#undef X

    default:
      throw std::runtime_error {"invalid tag type"};
  }
}

inline void encode_list(std::ostream& buf, const TagList& list) {
  if(list.base.valueless_by_exception())
    throw std::runtime_error {"invalid TagList"};
  encode<TagByte>(buf, list.index());
  switch(list.index()) {
    case TAG_END:
      encode<TagInt>(buf, 0);
      break;

#define X(enum, type)                                                         \
  case enum: {                                                                \
    auto& vec {get<std::vector<type>>(list)};                                 \
    encode<TagInt>(buf, vec.size());                                          \
    for(const auto val : vec)                                                 \
      encode<type>(buf, val);                                                 \
  } break;
      ALL_NUMERIC(X)
#undef X

#define X(enum, type, base_type)                                              \
  case enum: {                                                                \
    auto& vec {get<std::vector<type>>(list)};                                 \
    encode<TagInt>(buf, vec.size());                                          \
    for(const auto& val : vec)                                                \
      encode_array<base_type>(buf, val);                                      \
  } break;
      ALL_ARRAYS(X)
#undef X

#define X(enum, type, ext)                                                    \
  case enum: {                                                                \
    auto& vec {get<std::vector<type>>(list)};                                 \
    for(const auto& val : vec)                                                \
      encode##ext(buf, val);                                                  \
  } break;
      ALL_OTHERS(X)
#undef X
  }
}

inline void print_list(
    std::ostream& os, const std::string& indent, const TagList& list) {
  if(list.base.valueless_by_exception())
    throw std::runtime_error {"invalid TagList"};

  os << "<TagList of ";
  std::string next_indent {indent + indent_step};

  switch(list.index()) {
    case TAG_END:
      os << "TagEnd> {";
      break;

#define X(enum, type)                                                         \
  case enum: {                                                                \
    os << #type ">";                                                          \
    auto& vec {get<std::vector<type>>(list)};                                 \
    print_array(os, vec);                                                     \
  } break;
      ALL_NUMERIC(X)
#undef X

#define X(enum, type, base_type)                                              \
  case enum: {                                                                \
    os << #type "> {";                                                        \
    auto& vec {get<std::vector<type>>(list)};                                 \
    if(size_t size {vec.size()}; size) {                                      \
      os << "\n" << next_indent;                                              \
      print_array(os, vec[0]);                                                \
      for(size_t i {1}; i < size; i++) {                                      \
        os << ",\n" << next_indent;                                           \
        print_array(os, vec[i]);                                              \
      }                                                                       \
      os << "\n" << indent;                                                   \
    }                                                                         \
  } break;
      ALL_ARRAYS(X)
#undef X

    case TAG_STRING: {
      os << "TagString> {";
      auto& vec {get<std::vector<TagString>>(list)};
      if(size_t size {vec.size()}; size) {
        os << "\n" << next_indent << "\"" << vec[0] << "\"";
        for(size_t i {1}; i < size; i++)
          os << ",\n" << next_indent << "\"" << vec[i] << "\"";
        os << "\n" << indent;
      }
    } break;

    case TAG_LIST: {
      os << "TagList> {";
      auto& vec {get<std::vector<TagList>>(list)};
      if(size_t size {vec.size()}; size) {
        os << "\n" << next_indent;
        print_list(os, next_indent, vec[0]);
        for(size_t i {1}; i < size; i++) {
          os << ",\n" << next_indent;
          print_list(os, next_indent, vec[i]);
        }
        os << "\n" << indent;
      }
    } break;

    case TAG_COMPOUND: {
      os << "TagCompound> {";
      auto& vec {get<std::vector<TagCompound>>(list)};
      if(size_t size {vec.size()}; size) {
        os << "\n" << next_indent;
        print_compound(os, next_indent, vec[0]);
        for(size_t i {1}; i < size; i++) {
          os << ",\n" << next_indent;
          print_compound(os, next_indent, vec[i]);
        }
        os << "\n" << indent;
      }
    } break;
  }
  os << "}";
}

inline TagCompound decode_compound(std::istream& buf) {
  TagCompound tag;
  TagByte type {decode<TagByte>(buf)};
  for(; type != TAG_END; type = decode<TagByte>(buf)) {
    std::string key {decode_string(buf)};
    switch(type) {
#define X(enum, type)                                                         \
  case enum:                                                                  \
    tag.base[key] = decode<type>(buf);                                        \
    break;
      ALL_NUMERIC(X)
#undef X

#define X(enum, type, base_type)                                              \
  case enum:                                                                  \
    tag.base[key] = decode_array<base_type>(buf);                             \
    break;
      ALL_ARRAYS(X)
#undef X

#define X(enum, type, ext)                                                    \
  case enum:                                                                  \
    tag.base[key] = decode##ext(buf);                                         \
    break;
      ALL_OTHERS(X)
#undef X
      default:
        throw std::runtime_error {"invalid tag type"};
    }
  }
  return tag;
}

inline void encode_compound(std::ostream& buf, const TagCompound& map) {
  for(const auto& [key, tag] : map.base) {
    encode<TagByte>(buf, tag.index());
    encode_string(buf, key);

    switch(tag.index()) {
#define X(enum, type)                                                         \
  case enum:                                                                  \
    encode<type>(buf, std::get<type>(tag));                                   \
    break;
      ALL_NUMERIC(X)
#undef X

#define X(enum, type, base_type)                                              \
  case enum:                                                                  \
    encode_array<base_type>(buf, std::get<type>(tag));                        \
    break;
      ALL_ARRAYS(X)
#undef X

#define X(enum, type, ext)                                                    \
  case enum:                                                                  \
    encode##ext(buf, std::get<type>(tag));                                    \
    break;
      ALL_OTHERS(X)
#undef X
      default:
        throw std::runtime_error {"invalid tag type"};
    }
  }
  encode<TagByte>(buf, 0);
}

inline void print_compound(
    std::ostream& os, const std::string& indent, const TagCompound& map) {
  os << "<TagCompound> {";
  std::string next_indent {indent + indent_step};
  bool first {true};

  for(const auto& [key, tag] : map.base) {
    if(first)
      first = false;
    else
      os << ",";
    os << "\n" << next_indent << key << ": ";

    switch(tag.index()) {
#define X(enum, type)                                                         \
  case enum:                                                                  \
    os << "<" #type "> " << std::get<type>(tag);                              \
    break;
      ALL_NUMERIC(X)
#undef X

#define X(enum, type, base_type)                                              \
  case enum:                                                                  \
    print_array(os, std::get<type>(tag));                                     \
    break;
      ALL_ARRAYS(X)
#undef X

      case TAG_STRING:
        os << "\"" << std::get<TagString>(tag) << "\"";
        break;

      case TAG_LIST:
        print_list(os, next_indent, std::get<TagList>(tag));
        break;

      case TAG_COMPOUND:
        print_compound(os, next_indent, std::get<TagCompound>(tag));
        break;

      default:
        throw std::runtime_error {"invalid tag"};
    }
  }
  if(!map.base.empty())
    os << "\n" << indent;
  os << "}";
}

struct NBT {
  std::optional<std::string> name;
  TagCompound tag;

  void decode(std::istream& buf) {
    TagByte type {nbt::decode<TagByte>(buf)};
    if(type == TAG_COMPOUND) {
      name = decode_string(buf);
      tag = decode_compound(buf);
    } else if(type != TAG_END)
      throw std::runtime_error {"invalid tag type"};
  }

  void encode(std::ostream& buf) const {
    if(!name && tag.base.empty())
      nbt::encode<TagByte>(buf, TAG_END);
    else {
      nbt::encode<TagByte>(buf, TAG_COMPOUND);
      encode_string(buf, name ? *name : "");
    }
  }

  operator bool() const {
    return !name && tag.base.empty();
  }

private:
  friend std::ostream& operator<<(std::ostream& os, const NBT& val) {
    os << "\"" << (val.name ? *val.name : "") << "\"\n";
    print_compound(os, "", val.tag);
    return os;
  }
};

} // namespace nbt

#endif // NBT_HPP
