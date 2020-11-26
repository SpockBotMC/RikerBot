%module CPluginLoader
%{
#include "plugin_loader.hpp"
%}

%feature ("flatnested");

%include <std_string.i>
%ignore require;
%rename(require) py_require;
%include "plugin_loader.hpp"
