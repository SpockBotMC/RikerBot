%module CIOCore
%{
#include "io_core.hpp"
%}

%feature ("flatnested");

%include <stdint.i>
%include <std_string.i>
%include <pybuffer.i>
%include "plugin_base.hpp"

%typemap(out) (std::uint8_t[16]) {
  $result = PyBytes_FromStringAndSize(reinterpret_cast<char*>($1), 16);
}
%include "io_core.hpp"
