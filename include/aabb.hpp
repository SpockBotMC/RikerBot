#ifndef RKR_AABB_HPP
#define RKR_AABB_HPP

#include "vec3.hpp"

namespace rkr {

constexpr double PlayerHeight {1.8};
constexpr double PlayerHalfWidth {0.3};

class AABB {
public:
  AABB() = default;
  AABB(Vec3 max) : max {max} {};
  AABB(Vec3 min, Vec3 max) : max {max}, min {min} {};

  AABB& contract(const Vec3 v) {
    min += v;
    max -= v;
    return *this;
  }

  AABB& expand(const Vec3 v) {
    min -= v;
    max += v;
    return *this;
  }

  AABB& offset(const Vec3 v) {
    min += v;
    max += v;
    return *this;
  }

  bool intersects(const AABB& other) const {
    auto overlaps {(min < other.max) & (max > other.min)};
    return overlaps == XYZ_AXIS;
  }

private:
  Vec3 max;
  Vec3 min;
};

inline AABB player_bbox(Vec3 pos) {
  const double w {PlayerHalfWidth};
  return AABB {{-w, 0.0, -w}, {w, PlayerHeight, w}}.offset(pos);
}

} // namespace rkr

#endif // RKR_AABB_HPP
