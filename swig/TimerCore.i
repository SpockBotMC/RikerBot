%module CTimerCore
%{
#include "timer_core.hpp"
%}

%feature ("flatnested");

%include <stdint.i>
%include <std_string.i>

%warnfilter(401) TimerCore;
%include "timer_core.hpp"
