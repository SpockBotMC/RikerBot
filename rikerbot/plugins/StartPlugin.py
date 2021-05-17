from rikerbot.PluginBase import PluginBase, pl_announce
from rikerbot import proto


@pl_announce('Start')
class StartPlugin(PluginBase):
  requires = ('Event', 'Exec', 'IO', 'Auth')
  events = {
      'io_connect': 'handle_connect',
  }

  def __init__(self, ploader, settings):
    super().__init__(ploader, settings)
    ploader.provide("Start", self.start)

  def start(self, host="localhost", port="25565"):
    port = str(port)
    self.auth.login()
    self.io.connect(host, port)
    self.exec.run()

  def handle_connect(self, event_id, connect_data):
    packet = proto.ServerboundSetProtocol()
    packet.protocolVersion = proto.MC_PROTO_VERSION
    packet.nextState = proto.LOGIN
    packet.serverHost = connect_data.host
    packet.serverPort = connect_data.port
    self.io.encode_packet(packet)
    packet = proto.ServerboundLoginStart()
    packet.username = self.auth.username
    self.io.encode_packet(packet)
