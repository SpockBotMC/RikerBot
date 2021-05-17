# Copied wholesale from SpockBot, thanks Gjum
import copy


def get_settings(defaults, settings):
  return dict(copy.deepcopy(defaults), **settings)


def pl_announce(*args):
  def inner(cl):
    cl.pl_announce = args
    return cl

  return inner


class PluginBase:
  """A base class for cleaner plugin code.
  Extending from PluginBase allows you to declare any requirements, default
  settings, and event listeners in a declarative way. Define the appropriate
  attributes on your subclass and enjoy cleaner code.
  """
  requires = ()
  defaults = {}
  events = {}

  def __init__(self, ploader, settings):
    # Load the plugin's settings.
    self.settings = get_settings(self.defaults, settings)

    # Load all the plugin's dependencies.
    if isinstance(self.requires, str):
      setattr(self, self.requires.lower(), ploader.require(self.requires))
    else:
      for requirement in self.requires:
        setattr(self, requirement.lower(), ploader.require(requirement))

    # Setup the plugin's event handlers.
    if self.events:
      ev = ploader.require('Event')
      for event in self.events.keys():
        if hasattr(self, self.events[event]):
          ev.register_callback(event, getattr(self, self.events[event]))
        else:
          raise AttributeError(f"{self.__class__.__name__} object has no "
                               f"attribute '{self.events[event]}'")


# Most C Plugins are used only for dependency resolution, they don't stick
# around to own their core. Therefore we need to release ownership at
# the Python level and hand it over to the plugin loader. This plugin does that
# generically
class CPluginBase(PluginBase):
  core = None

  def __init__(self, ploader, settings):
    self.core(ploader, True).thisown = 0
