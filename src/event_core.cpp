#include "event_core.hpp"
#include "swigpyrun.hpp"

namespace rkr {

EventCore::EventCore(PluginLoader& ploader, bool ownership) :
    PluginBase("rkr::EventCore *") {
  ploader.provide("Event", this, ownership);
}

ev_id_type EventCore::register_event(std::string event_name) {
  if(event_map.contains(event_name))
    return event_map[event_name];
  ev_id_type id = event_channels.size();
  event_map[event_name] = id;
  event_channels.emplace_back(id);
  return id;
}

cb_id_type EventCore::register_callback(ev_id_type event_id, event_cb cb) {
  return event_channels[event_id].subscribe(cb);
}

cb_id_type EventCore::register_callback(std::string event_name, event_cb cb) {
  ev_id_type event_id = register_event(event_name);
  return event_channels[event_id].subscribe(cb);
}

cb_id_type EventCore::register_callback(ev_id_type event_id, PyObject *cb) {
  return event_channels[event_id].subscribe(cb);
}

cb_id_type EventCore::register_callback(std::string event_name, PyObject *cb) {
  ev_id_type event_id = register_event(event_name);
  return event_channels[event_id].subscribe(cb);
}

int EventCore::unregister_callback(ev_id_type event_id,
    cb_id_type cb_id) {
  return event_channels[event_id].unsubscribe(cb_id);
}

void EventCore::emit(ev_id_type event_id) {
  event_channels[event_id].emit();
}

void EventCore::emit(ev_id_type event_id, const void* data) {
  event_channels[event_id].emit(data);
}

// ToDo: Need to error check these SWIG_TypeQuery's probably
void EventCore::emit(ev_id_type event_id, const void* data,
    const std::string& type_query) {
  event_channels[event_id].emit(data);
  PyObject *pyo = SWIG_NewPointerObj(const_cast<void*>(data),
      SWIG_TypeQuery(type_query.c_str()), 0);
  event_channels[event_id].emit(pyo);
  Py_DECREF(pyo);
}

void EventCore::emit(ev_id_type event_id, PyObject *data) {
  event_channels[event_id].emit(const_cast<const void*>(
      static_cast<void*>(data)));
  event_channels[event_id].emit(data);
}
void EventCore::emit(ev_id_type event_id, PyObject* data,
    const std::string& type_query) {
  void *ptr;
  SWIG_ConvertPtr(data, &ptr, SWIG_TypeQuery(type_query.c_str()), 0);
  event_channels[event_id].emit(const_cast<const void*>(ptr));
  event_channels[event_id].emit(data);
}

} // namespace rkr
