from rikerbot.PluginBase import CPluginBase, pl_announce
from .CStatusCore import StatusCore

@pl_announce('Status')
class StatusPlugin(CPluginBase):
  core = StatusCore
