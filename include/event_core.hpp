#ifndef EVENT_CORE_HPP
#define EVENT_CORE_HPP

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <iostream>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <string>
#include <vector>

#include "plugin_loader.hpp"

namespace rkr {

class EventCore {
public:
  typedef std::uint64_t ev_id_type;
  typedef std::uint64_t cb_id_type;
  typedef std::function<void(ev_id_type, const void*)> event_cb;

  EventCore(rkr::PluginLoader& ploader);
  ev_id_type register_event(std::string event_name);

  cb_id_type register_callback(ev_id_type event_id, event_cb cb);
  cb_id_type register_callback(ev_id_type event_id, PyObject *cb);

  void register_callback(std::string event_name, event_cb cb);
  void register_callback(std::string event_name, PyObject *cb);

  int unregister_callback(ev_id_type event_id, cb_id_type cb_id);

  void emit(ev_id_type event_id);
  void emit(ev_id_type event_id, const void* data);
  void emit(ev_id_type event_id, PyObject* data);
  void emit(ev_id_type event_id, const void* data,
      const std::string& type_query);

private:
  class channel;
  std::unordered_map<std::string, ev_id_type> event_map;
  std::vector<channel> event_channels;

  class channel {
  public:
    ev_id_type event_id;

    channel(ev_id_type id) : event_id(id), next_callback_id(0) {}

    cb_id_type subscribe(event_cb cb) {
      cb_id_type cb_id = get_free_id();
      event_cbs.emplace_back(cb_id, cb);
      return cb_id;
    }

    cb_id_type subscribe(PyObject* cb) {
      cb_id_type cb_id = get_free_id();
      py_cbs.emplace_back(cb_id, cb);
      return cb_id;
    }

    int unsubscribe(cb_id_type cb_id) {
      for(auto it = event_cbs.begin(); it != event_cbs.end(); it++) {
        if(it->first == cb_id) {
          event_cbs.erase(it);
          free_ids.push_back(cb_id);
          return 0;
        }
      }
      for(auto it = py_cbs.begin(); it != py_cbs.end(); it++) {
        if(it->first == cb_id) {
          py_cbs.erase(it);
          free_ids.push_back(cb_id);
          return 0;
        }
      }
      return -1;
    }

    void emit(const void* data) {
      for(auto& el : event_cbs)
        el.second(event_id, data);
    }

    void emit(PyObject* data) {
      for(auto& el : py_cbs) {
        Py_XDECREF(PyObject_CallFunctionObjArgs(el.second, data, NULL));
      }
    }

    void emit() {
      for(auto& el : event_cbs)
        el.second(event_id, nullptr);
      for(auto& el : py_cbs)
        Py_XDECREF(PyObject_CallFunctionObjArgs(el.second, Py_None, NULL));
    }

  private:
    cb_id_type next_callback_id;
    std::vector<cb_id_type> free_ids;
    std::vector<std::pair<cb_id_type, event_cb>> event_cbs;
    std::vector<std::pair<cb_id_type, PyObject*>> py_cbs;

    cb_id_type get_free_id() {
      cb_id_type ret;
      if(!free_ids.empty()) {
        ret = free_ids.back();
        free_ids.pop_back();
        return ret;
      }
      return next_callback_id++;
    }
  };
};

} // namespace rkr
#endif
