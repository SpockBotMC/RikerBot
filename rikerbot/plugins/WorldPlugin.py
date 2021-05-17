from rikerbot.PluginBase import CPluginBase, pl_announce
from .CWorldCore import WorldCore


@pl_announce('World')
class WorldPlugin(CPluginBase):
  requires = ('Event', )
  core = WorldCore
