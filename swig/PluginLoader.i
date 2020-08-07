%module PluginLoader
%{
#include "plugin_loader.hpp"
%}

%include <std_string.i>
%ignore get_class;
%rename(get_class) py_get_class;
%include "plugin_loader.hpp"
