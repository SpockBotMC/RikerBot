import copy

# "plugins" is a list of (name, plugin) tuples, where "name" corresponds to an
# entry in the settings dict
def solve_dependencies(ploader, plugins, settings = None):
  announce = {}
  loaded = []
  failed_dependencies = []
  plugins = copy.copy(plugins)
  settings = settings if settings else {}

  def load_plugin(plugin_tuple):
    name, plugin = plugin_tuple
    requirements = plugin.requires
    if isinstance(requirements, str):
      requirements = (requirements,)
    for requirement in requirements:
      if requirement in announce and requirement not in loaded:
        load_plugin(announce[requirement])
        loaded.append(requirement)
      elif requirement not in announce:
        failed_dependencies.append(requirement)
    plugin(ploader, settings.get(name, {}))

  for name, plugin in plugins:
    if hasattr(plugin, 'pl_announce'):
      for ident in plugin.pl_announce:
        announce[ident] = (name, plugin)

  while plugins:
    load_plugin(plugins.pop())

  return failed_dependencies
