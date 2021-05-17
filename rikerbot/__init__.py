# if it doesn't come bursting out of you
# in spite of everything,
# don't do it.
# unless it comes unasked out of your
# heart and your mind and your mouth
# and your gut,
# don't do it.
# if you have to sit for hours
# staring at your computer screen
# or hunched over your
# typewriter
# searching for words,
# don't do it
#   - Charles Bukowski, so you want to be a writer?

import rikerbot.proto
import rikerbot.CLogger as logger

from rikerbot.proto import mc_position, mc_uuid

from .plugins.IOPlugin import IOPlugin
from .plugins.EventPlugin import EventPlugin
from .plugins.ExecPlugin import ExecPlugin
from .plugins.StartPlugin import StartPlugin
from .plugins.AuthPlugin import AuthPlugin
from .plugins.KeepAlive import KeepAlivePlugin
from .plugins.WorldPlugin import WorldPlugin
from .plugins.StatusPlugin import StatusPlugin

from .DependencySolver import solve_dependencies
from .PluginBase import PluginBase, pl_announce
from .PluginLoader import PluginLoader, make_PluginLoader, delete_PluginLoader

default_plugins = [
    ('start', StartPlugin),
    ('io', IOPlugin),
    ('event', EventPlugin),
    ('auth', AuthPlugin),
    ('keepalive', KeepAlivePlugin),
    ('world', WorldPlugin),
    ('status', StatusPlugin),
    ('exec', ExecPlugin),
]

from .SimpleClient import SimpleClient
