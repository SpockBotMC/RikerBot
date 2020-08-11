from rikerbot.PluginBase import PluginBase
from rikerbot import proto

class KeepAlivePlugin(PluginBase):
  requires = ('IO')
  events = {
    'ClientboundKeepAlive': 'handle_keep_alive'
  }

  def __init__(self, ploader, settings):
    super().__init__(ploader, settings)

  def handle_keep_alive(self, event_id, in_packet):
    out_packet = proto.ServerboundKeepAlive()
    out_packet.keepAliveId = in_packet.keepAliveId
    self.io.encode_packet(out_packet)
