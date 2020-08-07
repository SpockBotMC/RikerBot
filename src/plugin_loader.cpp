#include "plugin_loader.hpp"
#include "swigpyrun.hpp"

namespace rkr {

PluginBase* PluginLoader::get_class(std::string class_name) {
  if(class_map.contains(class_name))
    return class_map[class_name];
  return nullptr;
}

PyObject* PluginLoader::py_get_class(std::string class_name) {
  if(!class_map.contains(class_name))
    Py_RETURN_NONE;
  PluginBase* pl = class_map[class_name];
  if(pl->type_query)
    return SWIG_NewPointerObj(static_cast<void*>(pl),
        SWIG_TypeQuery(pl->type_query.value().c_str()), 0);
  Py_RETURN_NONE;
}

void PluginLoader::provide_class(std::string class_name,
    PluginBase* class_ptr) {
  class_map[class_name] = class_ptr;
}

} //namespace rkr
