import rikerbot.proto
import rikerbot.CLogger as logger

from rikerbot.proto import mc_position, mc_uuid

from .plugins.IOPlugin import IOPlugin
from .plugins.EventPlugin import EventPlugin
from .plugins.StartPlugin import StartPlugin
from .plugins.AuthPlugin import AuthPlugin
from .plugins.KeepAlive import KeepAlivePlugin
from .plugins.WorldPlugin import WorldPlugin

from .DependencySolver import solve_dependencies
from .PluginBase import PluginBase, pl_announce
from .PluginLoader import PluginLoader

default_plugins = [
  ('start', StartPlugin),
  ('io', IOPlugin),
  ('event', EventPlugin),
  ('auth', AuthPlugin),
  ('keepalive', KeepAlivePlugin),
  ('world', WorldPlugin),
]

from .SimpleClient import SimpleClient
