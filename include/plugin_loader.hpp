#ifndef PLUGIN_LOADER_HPP
#define PLUGIN_LOADER_HPP

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include<string>
#include<unordered_map>
#include "plugin_base.hpp"
#include <vector>
#include <memory>

namespace rkr {

class PluginLoader {
public:
  PluginBase* require(std::string class_name);
  PyObject* py_require(std::string class_name);

  void provide(std::string class_name, rkr::PluginBase* class_ptr,
      bool own = false);
  void provide(std::string class_name, PyObject* pyo);

private:
  std::vector<std::unique_ptr<PluginBase>> owned;
  std::unordered_map<std::string, PluginBase*> class_map;
  std::unordered_map<std::string, PyObject *> pyo_map;
};


} // namespace rkr
#endif
