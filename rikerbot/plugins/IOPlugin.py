from rikerbot.PluginBase import CPluginBase, pl_announce
from .CIOCore import IOCore

@pl_announce('IO')
class IOPlugin(CPluginBase):
  requires = ('Event',)
  core = IOCore
