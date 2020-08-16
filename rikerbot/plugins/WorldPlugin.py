from rikerbot.PluginBase import PluginBase, pl_announce
from .CWorldCore import WorldCore

@pl_announce('World')
class WorldPlugin(PluginBase):
  requires = ('Event',)
  def __init__(self, ploader, settings):
    # WorldPlugin exists purely for dependency resolution, it doesn't stick
    # around to own the WorldCore. Therefore we need to release ownership at
    # the Python level and hand it over to the plugin loader.
    core = WorldCore(ploader, True);
    core.thisown = 0
