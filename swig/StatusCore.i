%module CStatusCore
%{
#include "status_core.hpp"
%}

%feature ("flatnested");

%include <stdint.i>
%include <std_string.i>

%warnfilter(401) StatusCore;
%include "status_core.hpp"
