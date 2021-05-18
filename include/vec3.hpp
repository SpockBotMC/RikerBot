#ifndef RKR_VEC3_HPP
#define RKR_VEC3_HPP

#include <array>
#include <cstdint>
#include <immintrin.h>
#include <stdalign.h>
#include <stdexcept>
#include <string>

namespace rkr {

// https://stackoverflow.com/a/49943540/1201456
inline __m128d hsum256_128(__m256d v) {
  __m128d vlow {_mm256_castpd256_pd128(v)};
  __m128d vhigh {_mm256_extractf128_pd(v, 1)};

  vlow = _mm_add_pd(vlow, vhigh);
  vhigh = _mm_unpackhi_pd(vlow, vlow);
  return _mm_add_sd(vlow, vhigh);
}

inline __m128d hsumsq256_128(__m256d v) {
  return hsum256_128(_mm256_mul_pd(v, v));
}

class Vec3 {
public:
  Vec3() = default;
  Vec3(double d) : ymm {_mm256_setr_pd(d, d, d, 0)} {}
  Vec3(const double d[3]) : ymm {_mm256_setr_pd(d[0], d[1], d[2], 0)} {}
  Vec3(double x, double y, double z) : ymm {_mm256_setr_pd(x, y, z, 0)} {}
  Vec3(const __m256d m) : ymm {m} {}

  Vec3& operator=(const __m256d m) {
    ymm = m;
    return *this;
  }

  double operator[](unsigned int index) const {
    return xyz().at(index);
  }

  operator __m256d() const {
    return ymm;
  }

  Vec3& set(double d, unsigned int idx) {
    __m256d m {_mm256_broadcast_sd(&d)};
    switch(idx) {
      case 0:
        ymm = _mm256_blend_pd(ymm, m, 1);
        break;
      case 1:
        ymm = _mm256_blend_pd(ymm, m, 2);
        break;
      case 2:
        ymm = _mm256_blend_pd(ymm, m, 4);
        break;
      default:
        throw std::out_of_range(std::to_string(idx) + " >= Vec3 size of 3");
    }
    return *this;
  }

  double x() const {
    return xyz()[0];
  }
  Vec3& x(double d) {
    return set(d, 0);
  }

  double y() const {
    return xyz()[1];
  }
  const Vec3 y(double d) {
    return set(d, 1);
  }

  double z() const {
    return xyz()[2];
  }
  Vec3& z(double d) {
    return set(d, 2);
  }

  std::array<double, 3> xyz() const {
    alignas(32) double s[4];
    _mm256_store_pd(s, ymm);
    return {s[0], s[1], s[2]};
  }
  Vec3& xyz(const std::array<double, 3>& da) {
    alignas(32) double s[4] {da[0], da[1], da[2], 0};
    ymm = _mm256_load_pd(s);
    return *this;
  }

  double sq_dist() const {
    return _mm_cvtsd_f64(hsumsq256_128(ymm));
  }

  double dist() const {
    return _mm_cvtsd_f64(_mm_sqrt_pd(hsumsq256_128(ymm)));
  }

  Vec3 unit() const {
    __m256d m {_mm256_broadcastsd_pd(_mm_sqrt_pd(hsumsq256_128(ymm)))};
    return _mm256_div_pd(ymm, m);
  }

  Vec3& floor() {
    ymm = _mm256_floor_pd(ymm);
    return *this;
  }

private:
  __m256d ymm {_mm256_set1_pd(0.0)};
};

inline Vec3 operator+(const Vec3 a, const Vec3 b) {
  return _mm256_add_pd(a, b);
}
inline Vec3 operator+(const Vec3 a, double b) {
  return a + Vec3(b);
}
inline Vec3 operator+(double a, const Vec3 b) {
  return Vec3(a) + b;
}

inline Vec3& operator+=(Vec3& a, const Vec3 b) {
  a = a + b;
  return a;
}

inline Vec3 operator++(Vec3& a, int) {
  Vec3 b {a};
  a = a + 1.0;
  return b;
}
inline Vec3& operator++(Vec3& a) {
  a = a + 1.0;
  return a;
}

inline Vec3 operator-(const Vec3 a, const Vec3 b) {
  return _mm256_sub_pd(a, b);
}
inline Vec3 operator-(const Vec3 a, double b) {
  return a - Vec3(b);
}
inline Vec3 operator-(double a, const Vec3 b) {
  return Vec3(a) - b;
}

inline Vec3 operator-(const Vec3 a) {
  __m256i m {
      _mm256_setr_epi32(0, 0x80000000u, 0, 0x80000000u, 0, 0x80000000u, 0, 0)};
  return _mm256_xor_pd(a, _mm256_castsi256_pd(m));
}

inline Vec3& operator-=(Vec3& a, const Vec3 b) {
  a = a - b;
  return a;
}

inline Vec3 operator--(Vec3& a, int) {
  Vec3 b {a};
  a = a - 1.0;
  return b;
}
inline Vec3& operator--(Vec3& a) {
  a = a - 1.0;
  return a;
}

inline Vec3 operator*(const Vec3 a, const Vec3 b) {
  return _mm256_mul_pd(a, b);
}
inline Vec3 operator*(const Vec3 a, double b) {
  return a * Vec3(b);
}
inline Vec3 operator*(double a, const Vec3 b) {
  return Vec3(a) * b;
}

inline Vec3& operator*=(Vec3& a, const Vec3 b) {
  a = a * b;
  return a;
}

inline Vec3 operator/(const Vec3 a, const Vec3 b) {
  return _mm256_mul_pd(a, b);
}
inline Vec3 operator/(const Vec3 a, double b) {
  return a / Vec3(b);
}
inline Vec3 operator/(double a, const Vec3 b) {
  return Vec3(a) / b;
}

inline Vec3& operator/=(Vec3& a, const Vec3 b) {
  a = a / b;
  return a;
}

enum AXIS { X_AXIS = 0x1, Y_AXIS = 0x2, Z_AXIS = 0x4 };

std::uint8_t operator==(const Vec3 a, const Vec3 b) {
  return _mm256_movemask_pd(_mm256_cmp_pd(a, b, _CMP_EQ_OQ));
}
std::uint8_t operator!=(const Vec3 a, const Vec3 b) {
  return _mm256_movemask_pd(_mm256_cmp_pd(a, b, _CMP_NEQ_UQ));
}

std::uint8_t operator<=(const Vec3 a, const Vec3 b) {
  return _mm256_movemask_pd(_mm256_cmp_pd(a, b, _CMP_LE_OQ));
}
std::uint8_t operator<(const Vec3 a, const Vec3 b) {
  return _mm256_movemask_pd(_mm256_cmp_pd(a, b, _CMP_LT_OQ));
}

std::uint8_t operator>=(const Vec3 a, const Vec3 b) {
  return _mm256_movemask_pd(_mm256_cmp_pd(a, b, _CMP_NLT_UQ));
}
std::uint8_t operator>(const Vec3 a, const Vec3 b) {
  return _mm256_movemask_pd(_mm256_cmp_pd(a, b, _CMP_NLE_UQ));
}

double dot_product(const Vec3 a, const Vec3 b) {
  return _mm_cvtsd_f64(hsum256_128(a * b));
}

Vec3 cross_product(const Vec3 a, const Vec3 b) {
  const std::uint8_t mm_yzx {_MM_SHUFFLE(3, 0, 2, 1)};
  const std::uint8_t mm_zxy {_MM_SHUFFLE(3, 1, 0, 2)};

  __m256d c {_mm256_mul_pd(
      _mm256_permute4x64_pd(a, mm_yzx), _mm256_permute4x64_pd(b, mm_zxy))};
  __m256d d {_mm256_mul_pd(
      _mm256_permute4x64_pd(a, mm_zxy), _mm256_permute4x64_pd(b, mm_yzx))};
  return _mm256_sub_pd(c, d);
}

} // namespace rkr
#endif // RKR_VEC3_HPP
