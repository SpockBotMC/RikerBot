%module CIOCore
%{
#include "io_core.hpp"
%}

%include <stdint.i>
%include <std_string.i>
%include <pybuffer.i>
%include "plugin_base.hpp"

%typemap(out) (std::uint8_t[16]) {
  $result = PyMemoryView_FromMemory(reinterpret_cast<char *>($1), 16, PyBUF_READ);
}
%include "io_core.hpp"
