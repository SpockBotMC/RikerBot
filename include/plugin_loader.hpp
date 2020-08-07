#ifndef PLUGIN_LOADER_HPP
#define PLUGIN_LOADER_HPP

#include<string>
#include<unordered_map>

namespace rkr {

class PluginLoader {
public:
  void* get_class(std::string class_name);
  void provide_class(std::string class_name, void *class_ptr);
private:
  std::unordered_map<std::string, void*> class_map;
};


} // namespace rkr
#endif
