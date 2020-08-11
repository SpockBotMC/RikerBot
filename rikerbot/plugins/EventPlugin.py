from riker.PluginBase import PluginBase, pl_announce
from .CEventCore import EventCore

@pl_announce('Event')
class EventPlugin(PluginBase):
  def __init__(self, ploader, settings):
    # EventPlugin exists purely for dependency resolution, it doesn't stick
    # around to own the EventCore. Therefore we need to release ownership at
    # the Python level and hand it over to the plugin loader.
    core = EventCore(ploader, True);
    core.thisown = 0
