# Copied wholesale from SpockBot, thanks Gjum

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
      setattr(self, self.requires.lower(), ploader.requires(self.requires))
    else:
      for requirement in self.requires:
        setattr(self, requirement.lower(), ploader.requires(requirement))

    # Setup the plugin's event handlers.
    for event in self.events.keys():
      if hasattr(self, self.events[event]):
        ploader.reg_event_handler(event, getattr(self, self.events[event]))
      else:
        raise AttributeError(f"{self.__class__.__name__} object has no "
            f"attribute '{self.events[event]}'")
