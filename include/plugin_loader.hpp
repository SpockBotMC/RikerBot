#ifndef PLUGIN_LOADER_HPP
#define PLUGIN_LOADER_HPP

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include<string>
#include<unordered_map>
#include "plugin_base.hpp"

namespace rkr {

class PluginLoader {
public:
  PluginBase* get_class(std::string class_name);
  PyObject* py_get_class(std::string class_name);

  void provide_class(std::string class_name, rkr::PluginBase* class_ptr);
private:
  std::unordered_map<std::string, PluginBase*> class_map;
};


} // namespace rkr
#endif
