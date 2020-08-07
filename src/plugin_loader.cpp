#include "plugin_loader.hpp"

namespace rkr {

void* PluginLoader::get_class(std::string class_name) {
  if(class_map.contains(class_name))
    return class_map[class_name];
  return nullptr;
}

void PluginLoader::provide_class(std::string class_name, void *class_ptr) {
  class_map[class_name] = class_ptr;
}

} //namespace rkr
