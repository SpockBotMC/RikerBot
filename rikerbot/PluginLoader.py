# Okay so here's the deal:
# Ownership of objects that get passed between C++ and Python is tricky.
# Generally speaking, the best idea is for C++/Python to share ownership of
# pure Python objects, and for C++ to take ownership of pure C++ ones.
#
# In practice this is handled by the Plugin Loader, which will take over
# ownership of C++ objects and stores them in an "owned" vector, and for Python
# objects it will correctly increment/decrement their reference count. The
# problem then is this, quis custodiet ipsos custodes? Who owns the plugin
# loaders?
plugin_loader_list = []
# ^ This list right here does, take that Socrates
#
# You may wonder why this is necessary. It's necessary because if you build any
# sort of "Client" class you're going to be tempted to drop the reference
# you're holding to the PluginLoader, it's not needed so why keep it? Because
# it owns everything, and if you drop it you will segfault and not understand
# why. And then you will file a bug report, and I will have to explain that
# Python lied to you and memory management does matter.
#
# We could of course just leak the PluginLoaders' memory, allow them to ride
# eternal, shiny and chrome! Realistically they're going to be around for the
# life of the program anyway, and there is rarely more than one. But this is
# sacrilege in all of the popular programming sects, so we don't do that.
#
# Thus my list and I, we own the plugin loader, so you don't have to.
# You're welcome.

from .CPluginLoader import PluginLoader as _PluginLoader


class PluginLoader(_PluginLoader):
  def __init__(self):
    super().__init__()
    if plugin_loader_list:
      self.pl_id = plugin_loader_list[-1].pl_id + 1
    else:
      self.pl_id = 0
    plugin_loader_list.append(self)


def make_PluginLoader():
  pl = PluginLoader()
  return pl.pl_id, pl


def delete_PluginLoader(pl):
  if isinstance(pl, int):
    pl = next(filter(lambda x: x.pl_id == pl, plugin_loader_list), None)
  if not pl or pl not in plugin_loader_list:
    return None
  plugin_loader_list.remove(pl)
  return pl.pl_id
