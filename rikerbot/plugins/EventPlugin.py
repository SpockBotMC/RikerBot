from rikerbot.PluginBase import CPluginBase, pl_announce
from .CEventCore import EventCore


@pl_announce('Event')
class EventPlugin(CPluginBase):
  core = EventCore
