from rikerbot.PluginBase import CPluginBase, pl_announce
from .CIOCore import IOCore


@pl_announce('IO')
class IOPlugin(CPluginBase):
  requires = ('Event', 'Exec')
  core = IOCore

  def __init__(self, ploader, settings) -> None:
    self.core(ploader, ploader.require('Exec').get_ctx(), True).thisown = 0
