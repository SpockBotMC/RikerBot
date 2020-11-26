%module CLogger
%{
#include "logger.hpp"
%}

%feature ("flatnested");

%include <stdint.i>
%include <std_string.i>

%include "logger.hpp"
