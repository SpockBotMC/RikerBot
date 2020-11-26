%module CStatusCore
%{
#include "status_core.hpp"
%}

%feature ("flatnested");

%include <stdint.i>
%include <std_string.i>

%include "plugin_base.hpp"
%include "status_core.hpp"
