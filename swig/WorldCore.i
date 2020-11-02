%module CWorldCore
%{
#include "world_core.hpp"
%}

%include <stdint.i>
%include <std_string.i>
%include <std_vector.i>
%include <std_array.i>

namespace rkr {
typedef uint16_t block_id;
}

%template(BlockIdVector) std::vector<rkr::block_id>;
%template(PositionVector) std::vector<mcd::mc_position>;
%template(PosArray) std::array<std::int32_t, 3>;
%template(PosArrayVector) std::vector<std::array<std::int32_t, 3>>;

%include "plugin_base.hpp"
%include "world_core.hpp"
