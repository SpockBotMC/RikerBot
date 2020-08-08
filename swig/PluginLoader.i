%module CPluginLoader
%{
#include "plugin_loader.hpp"
%}

%include <std_string.i>
%ignore require;
%rename(require) py_require;
%include "plugin_loader.hpp"
