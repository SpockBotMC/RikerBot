#ifndef PLUGIN_LOADER_HPP
#define PLUGIN_LOADER_HPP

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "plugin_base.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace rkr {

class PluginLoader {
public:
  PluginBase* require(const std::string& class_name);
  PyObject* py_require(const std::string& class_name);

  void provide(const std::string& class_name, rkr::PluginBase* class_ptr,
      bool own = false);
  void provide(const std::string& class_name, PyObject* pyo);

private:
  std::vector<std::unique_ptr<PluginBase>> owned;
  std::unordered_map<std::string, PluginBase*> class_map;
  std::unordered_map<std::string, PyObject*> pyo_map;
};

} // namespace rkr
#endif
