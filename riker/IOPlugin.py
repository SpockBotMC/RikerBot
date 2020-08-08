from .PluginBase import PluginBase, pl_announce
from .CIOCore import IOCore

@pl_announce('IO')
class IOPlugin(PluginBase):
  requires = ('Event',)
  def __init__(self, ploader, settings):
    # IOPlugin exists purely for dependency resolution, it doesn't stick around
    # to own the IOCore. Therefore we need to release ownership at the Python
    # level and hand it over to the plugin loader.
    core = IOCore(ploader, True);
    core.thisown = 0
