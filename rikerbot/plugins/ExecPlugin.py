from rikerbot.PluginBase import CPluginBase, pl_announce
from .CExecCore import ExecCore


@pl_announce('Exec')
class ExecPlugin(CPluginBase):
  requires = ('Event', )
  core = ExecCore
