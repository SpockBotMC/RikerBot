#include "plugin_loader.hpp"
#include "swigpyrun.hpp"

namespace rkr {

PluginBase* PluginLoader::require(const std::string& class_name) {
  if(class_map.contains(class_name))
    return class_map[class_name];
  return nullptr;
}

PyObject* PluginLoader::py_require(const std::string& class_name) {
  if(class_map.contains(class_name)) {
    if(auto pl {class_map[class_name]}; pl->type_query)
      return SWIG_NewPointerObj(static_cast<void*>(pl),
          SWIG_TypeQuery(pl->type_query.value().c_str()), 0);
    Py_RETURN_NONE;
  } else if(pyo_map.contains(class_name)) {
    PyObject* pyo {pyo_map[class_name]};
    Py_INCREF(pyo);
    return pyo;
  }
  Py_RETURN_NONE;
}

void PluginLoader::provide(
    const std::string& class_name, PluginBase* class_ptr, bool own) {
  if(own)
    owned.emplace_back(class_ptr);
  class_map[class_name] = class_ptr;
}

void PluginLoader::provide(const std::string& class_name, PyObject* pyo) {
  Py_INCREF(pyo);
  if(pyo_map.contains(class_name))
    Py_DECREF(pyo_map[class_name]);
  pyo_map[class_name] = pyo;
}

} // namespace rkr
