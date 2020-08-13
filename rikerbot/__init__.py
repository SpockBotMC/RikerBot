import rikerbot.proto
import rikerbot.CLogger as logger

from .plugins.IOPlugin import IOPlugin
from .plugins.EventPlugin import EventPlugin
from .plugins.StartPlugin import StartPlugin
from .plugins.AuthPlugin import AuthPlugin
from .plugins.KeepAlive import KeepAlivePlugin

from .DependencySolver import solve_dependencies
from .PluginBase import PluginBase, pl_announce
from .PluginLoader import PluginLoader

default_plugins = [
  ('start', rikerbot.StartPlugin),
  ('io', rikerbot.IOPlugin),
  ('event', rikerbot.EventPlugin),
  ('auth', rikerbot.AuthPlugin),
  ('keepalive', rikerbot.KeepAlivePlugin),
]

from .SimpleClient import SimpleClient
