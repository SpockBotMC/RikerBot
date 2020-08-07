#include "event_core.hpp"
#include "swigpyrun.hpp"

namespace rkr {

EventCore::EventCore(PluginLoader& ploader) {
  ploader.provide_class("event", this);
}

EventCore::ev_id_type EventCore::register_event(std::string event_name) {
  if(event_map.contains(event_name))
    return event_map[event_name];
  EventCore::ev_id_type id = event_channels.size();
  event_map[event_name] = id;
  event_channels.emplace_back(id);
  return id;
}

EventCore::cb_id_type EventCore::register_callback(
    EventCore::ev_id_type event_id, EventCore::event_cb cb) {
  return event_channels[event_id].subscribe(cb);
}

void EventCore::register_callback(std::string event_name,
    EventCore::event_cb cb) {
  EventCore::ev_id_type event_id = register_event(event_name);
  event_channels[event_id].subscribe(cb);
}

EventCore::cb_id_type EventCore::register_callback(
    EventCore::ev_id_type event_id, PyObject *cb) {
  return event_channels[event_id].subscribe(cb);
}

void EventCore::register_callback(std::string event_name, PyObject *cb) {
  EventCore::ev_id_type event_id = register_event(event_name);
  event_channels[event_id].subscribe(cb);
}

int EventCore::unregister_callback(EventCore::ev_id_type event_id,
    EventCore::cb_id_type cb_id) {
  return event_channels[event_id].unsubscribe(cb_id);
}

void EventCore::emit(EventCore::ev_id_type event_id) {
  event_channels[event_id].emit();
}

void EventCore::emit(EventCore::ev_id_type event_id, const void* data) {
  event_channels[event_id].emit(data);
}

void EventCore::emit(EventCore::ev_id_type event_id, PyObject *data) {
  event_channels[event_id].emit(data);
}

void EventCore::emit(EventCore::ev_id_type event_id, const void* data,
    const std::string& type_query) {
  event_channels[event_id].emit(data);
  PyObject *pyo = SWIG_NewPointerObj(const_cast<void*>(data),
      SWIG_TypeQuery(type_query.c_str()), 0);
  event_channels[event_id].emit(pyo);
  Py_DECREF(pyo);
}

} // namespace rkr
