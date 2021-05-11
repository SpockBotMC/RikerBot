#ifndef EVENT_CORE_HPP
#define EVENT_CORE_HPP

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <cstdint>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "plugin_base.hpp"
#include "plugin_loader.hpp"

namespace rkr {

typedef std::uint64_t ev_id_type;
typedef std::uint64_t cb_id_type;
typedef std::function<void(ev_id_type, const void*)> event_cb;

class EventCore : public PluginBase {
public:
  EventCore(rkr::PluginLoader& ploader, bool ownership = false);
  ev_id_type register_event(const std::string& event_name);

  cb_id_type register_callback(ev_id_type event_id, event_cb cb);
  cb_id_type register_callback(ev_id_type event_id, PyObject* cb);
  cb_id_type register_callback(const std::string& event_name, event_cb cb);
  cb_id_type register_callback(const std::string& event_name, PyObject* cb);

  void unregister_callback(ev_id_type event_id, cb_id_type cb_id);

  void emit(ev_id_type event_id);
  void emit(ev_id_type event_id, const void* data);
  void emit(
      ev_id_type event_id, const void* data, const std::string& type_query);
  void emit(ev_id_type event_id, PyObject* data);
  void emit(ev_id_type event_id, PyObject* data, const std::string& type_query);

private:
  class channel;
  std::unordered_map<std::string, ev_id_type> event_map;
  std::vector<channel> event_channels;
  std::unordered_map<ev_id_type, std::vector<cb_id_type>> to_remove;
  std::vector<ev_id_type> event_stack;

  void clean_callbacks(ev_id_type event_id);

  class channel {
  public:
    ev_id_type event_id;

    channel(ev_id_type id) : event_id(id) {}

    cb_id_type subscribe(event_cb cb) {
      cb_id_type cb_id = get_free_id();
      event_cbs.emplace_back(cb_id, cb);
      return cb_id;
    }

    cb_id_type subscribe(PyObject* cb) {
      cb_id_type cb_id = get_free_id();
      Py_INCREF(cb);
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
        auto ev_id = PyLong_FromUnsignedLongLong(event_id);
        PyObject* result =
            PyObject_CallFunctionObjArgs(el.second, ev_id, data, NULL);
        if(!result) {
          PyErr_Print();
          exit(-1);
        }
        Py_DECREF(result);
        Py_DECREF(ev_id);
      }
    }

    void emit() {
      for(auto& el : event_cbs)
        el.second(event_id, nullptr);
      for(auto& el : py_cbs) {
        auto ev_id = PyLong_FromUnsignedLongLong(event_id);
        PyObject* result =
            PyObject_CallFunctionObjArgs(el.second, ev_id, Py_None, NULL);
        if(!result) {
          PyErr_Print();
          exit(-1);
        }
        Py_DECREF(result);
        Py_DECREF(ev_id);
      }
    }

  private:
    cb_id_type next_callback_id = 0;
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
