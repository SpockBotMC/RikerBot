from riker.PluginBase import PluginBase, pl_announce

@pl_announce('Start')
class StartPlugin(PluginBase):
  requires = ('Event', 'IO')
  events = {
    'io_connect': 'handle_connect'
  }

  def __init__(self, ploader, settings):
    super().__init__(ploader, settings)
    ploader.provide("Start", self.start)

  def start(self):
    self.io.connect("localhost", "25565")
    self.io.run()

  def handle_connect(self, connect_data):
    self.io.kill = 1
