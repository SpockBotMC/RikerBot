%module CIOCore
%{
#include "io_core.hpp"
%}

%feature ("flatnested");

%include <stdint.i>
%include <std_string.i>
%include <pybuffer.i>

%typemap(out) (std::uint8_t[16]) {
  $result = PyBytes_FromStringAndSize(reinterpret_cast<char*>($1), 16);
}

%warnfilter(401) IOCore;
%include "io_core.hpp"
