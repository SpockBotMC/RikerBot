%module CEventCore
%{
#include "event_core.hpp"
%}

%feature ("flatnested");

%include <stdint.i>
%include <std_string.i>

%warnfilter(401) EventCore;
%include "event_core.hpp"
