from rikerbot.PluginBase import CPluginBase, pl_announce
from .CExecCore import ExecCore


@pl_announce('Exec')
class ExecPlugin(CPluginBase):
  requires = ('Event', )
  core = ExecCore


from .CEventCore import EventCore


@pl_announce('Event')
class EventPlugin(CPluginBase):
  core = EventCore


from .CIOCore import IOCore


@pl_announce('IO')
class IOPlugin(CPluginBase):
  requires = ('Event', 'Exec')
  core = IOCore

  def __init__(self, ploader, settings) -> None:
    self.core(ploader, ploader.require('Exec').get_ctx(), True).thisown = 0


from .CStatusCore import StatusCore


@pl_announce('Status')
class StatusPlugin(CPluginBase):
  requires = ('Event', 'IO')
  core = StatusCore


from .CWorldCore import WorldCore


@pl_announce('World')
class WorldPlugin(CPluginBase):
  requires = ('Event', )
  core = WorldCore


from .CTimerCore import TimerCore


@pl_announce('Timer')
class TimerPlugin(CPluginBase):
  requires = ('Exec', )
  core = TimerCore

  def __init__(self, ploader, settings) -> None:
    self.core(ploader, ploader.require('Exec').get_ctx(), True).thisown = 0
