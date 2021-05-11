#ifndef BYTESWAP_HPP
#define BYTESWAP_HPP

#include <bit>
#include <concepts>
#include <cstdint>
#include <type_traits>

#if defined(_MSC_VER)

inline auto bswap(std::uint64_t v) noexcept {
  return _byteswap_uint64(v);
}
inline auto bswap(std::uint32_t v) noexcept {
  return _byteswap_ulong(v);
}
inline auto bswap(std::uint16_t v) noexcept {
  return _byteswap_ushort(v);
}

#else

inline auto bswap(std::uint64_t v) noexcept {
  return __builtin_bswap64(v);
}
inline auto bswap(std::uint32_t v) noexcept {
  return __builtin_bswap32(v);
}
inline auto bswap(std::uint16_t v) noexcept {
  return __builtin_bswap16(v);
}

#endif // _MSC_VER

// I don't know why this is required but compiles sometimes fail without it
inline auto bswap(std::uint8_t v) noexcept {
  return v;
}

inline auto byteswap(std::integral auto val) noexcept {
  if constexpr(sizeof(val) == 1)
    return static_cast<std::make_unsigned_t<decltype(val)>>(val);
  return bswap(static_cast<std::make_unsigned_t<decltype(val)>>(val));
}

inline auto nbeswap(std::integral auto val) noexcept {
  if constexpr(std::endian::native == std::endian::big)
    return val;
  return byteswap(val);
}

inline auto nleswap(std::integral auto val) noexcept {
  if constexpr(std::endian::native == std::endian::little)
    return val;
  return byteswap(val);
}

#endif // BYTESWAP_HPP
