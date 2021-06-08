# Copied wholesale from SpockBot, thanks Gjum
import copy


def get_settings(defaults, settings):
  return dict(copy.deepcopy(defaults), **settings)


def pl_announce(*args):
  def inner(cl):
    cl.pl_announce = args
    return cl

  return inner

def on_events(*events):
    """ 
    Register a method for callback when any of the given events are emitted, 
    by adding an entry in the class' events dict.

    The class the method belongs to must inherit from PluginBase.
    
    :param events: string or iterable containing names of events to listen to
    """
    if (isinstance(events, str)):
        events = (events,)
    def decorator(function):
        function.subscribe_to = events
        return function

    return decorator


class PluginBase:
  """A base class for cleaner plugin code.
  Extending from PluginBase allows you to declare any requirements, default
  settings, and event listeners in a declarative way. Define the appropriate
  attributes on your subclass and enjoy cleaner code.
  """
  requires = ()
  defaults = {}
  events = {}

  # Used for on_events decorator
  def __init_subclass__(cls, **kwargs):
    for method in vars(cls).values():
      if hasattr(method, "subscribe_to"):
        for event_name in method.subscribe_to:
          if event_name not in cls.events:
            cls.events[event_name] = [method.__name__]
          else:
            # Make str to list if not already
            cls.events[event_name] = [cls.events[event_name]] if isinstance(cls.events[event_name], str) else cls.events[event_name]
            cls.events[event_name].append(method.__name__)

  def __init__(self, ploader, settings):
    # Load the plugin's settings.
    self.settings = get_settings(self.defaults, settings)
    self.ploader = ploader

    # Load all the plugin's dependencies.
    if isinstance(self.requires, str):
      setattr(self, self.requires.lower(), ploader.require(self.requires))
    else:
      for requirement in self.requires:
        setattr(self, requirement.lower(), ploader.require(requirement))

    # Setup the plugin's event handlers.
    if self.events:
      ev = ploader.require('Event')
      for event_name, callbacks in self.events.items():
        if (isinstance(callbacks, str)):
          callbacks = (callbacks,)
        for callback in callbacks:
          if hasattr(self, callback):
            ev.register_callback(event_name, getattr(self, callback))
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
