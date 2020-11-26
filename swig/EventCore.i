%module CEventCore
%{
#include "event_core.hpp"
%}

%feature ("flatnested");

%include <stdint.i>
%include <std_string.i>
%include "plugin_base.hpp"
%include "event_core.hpp"
