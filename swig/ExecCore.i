%module CExecCore
%{
#include "exec_core.hpp"
%}

%feature ("flatnested");

%include <stdint.i>
%include <std_string.i>

%warnfilter(401) ExecCore;
%include "exec_core.hpp"
