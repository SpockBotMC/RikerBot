#include "event_core.hpp"
#include "swigpyrun.hpp"

namespace rkr {

EventCore::EventCore(PluginLoader& ploader, bool ownership)
    : PluginBase("rkr::EventCore *") {
  ploader.provide("Event", this, ownership);
}

ev_id_type EventCore::register_event(const std::string& event_name) {
  if(event_map.contains(event_name))
    return event_map[event_name];
  ev_id_type id {event_channels.size()};
  event_map[event_name] = id;
  event_channels.emplace_back(id);
  return id;
}

cb_id_type EventCore::register_callback(ev_id_type event_id, event_cb cb) {
  return event_channels[event_id].subscribe(cb);
}

cb_id_type EventCore::register_callback(
    const std::string& event_name, event_cb cb) {
  return event_channels[register_event(event_name)].subscribe(cb);
}

cb_id_type EventCore::register_callback(ev_id_type event_id, PyObject* cb) {
  return event_channels[event_id].subscribe(cb);
}

cb_id_type EventCore::register_callback(
    const std::string& event_name, PyObject* cb) {
  return event_channels[register_event(event_name)].subscribe(cb);
}

void EventCore::unregister_callback(ev_id_type event_id, cb_id_type cb_id) {
  if(std::any_of(event_stack.cbegin(), event_stack.cend(),
         [event_id](auto i) { return event_id == i; }))
    to_remove[event_id].push_back(cb_id);
  else
    event_channels[event_id].unsubscribe(cb_id);
}

void EventCore::emit(ev_id_type event_id) {
  event_stack.push_back(event_id);
  event_channels[event_id].emit();
  event_stack.pop_back();
  if(to_remove.contains(event_id))
    clean_callbacks(event_id);
}

void EventCore::emit(ev_id_type event_id, const void* data) {
  event_stack.push_back(event_id);
  event_channels[event_id].emit(data);
  event_stack.pop_back();
  if(to_remove.contains(event_id))
    clean_callbacks(event_id);
}

// ToDo: Need to error check these SWIG_TypeQuery's probably
void EventCore::emit(
    ev_id_type event_id, const void* data, const std::string& type_query) {
  event_stack.push_back(event_id);
  event_channels[event_id].emit(data);
  PyObject* pyo {SWIG_NewPointerObj(
      const_cast<void*>(data), SWIG_TypeQuery(type_query.c_str()), 0)};
  event_channels[event_id].emit(pyo);
  Py_DECREF(pyo);
  event_stack.pop_back();
  if(to_remove.contains(event_id))
    clean_callbacks(event_id);
}

void EventCore::emit(ev_id_type event_id, PyObject* data) {
  event_stack.push_back(event_id);
  event_channels[event_id].emit(
      const_cast<const void*>(static_cast<void*>(data)));
  event_channels[event_id].emit(data);
  event_stack.pop_back();
  if(to_remove.contains(event_id))
    clean_callbacks(event_id);
}

void EventCore::emit(
    ev_id_type event_id, PyObject* data, const std::string& type_query) {
  event_stack.push_back(event_id);
  void* ptr;
  SWIG_ConvertPtr(data, &ptr, SWIG_TypeQuery(type_query.c_str()), 0);
  event_channels[event_id].emit(const_cast<const void*>(ptr));
  event_channels[event_id].emit(data);
  event_stack.pop_back();
  if(to_remove.contains(event_id))
    clean_callbacks(event_id);
}

void EventCore::clean_callbacks(ev_id_type event_id) {
  for(auto& el : to_remove[event_id])
    event_channels[event_id].unsubscribe(el);
  to_remove.erase(event_id);
}

} // namespace rkr
