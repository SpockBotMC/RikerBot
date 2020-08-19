#ifndef RKR_STATUS_CORE_HPP
#define RKR_STATUS_CORE_HPP

namespace rkr {

class StatusCore {
public:
  struct {
    double x;
    double y;
    double z;
  } position;
  struct {
    float yaw;
    float pitch;
  } look;
  bool on_ground;

private:
}


} // namespace rkr


#endif // RKR_STATUS_CORE_HPP
