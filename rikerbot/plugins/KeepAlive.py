# forget the ink, the milk, the blood—
# all was washed clean with the flood
# we rose up from the falling waters
# the fallen rain’s own sons and daughters
#
# and none of this, none of this matters.
#   - Don Paterson, Rain

from rikerbot.PluginBase import PluginBase
from rikerbot import proto


class KeepAlivePlugin(PluginBase):
  requires = ('IO')
  events = {'ClientboundKeepAlive': 'handle_keep_alive'}

  def __init__(self, ploader, settings):
    super().__init__(ploader, settings)

  def handle_keep_alive(self, event_id, in_packet):
    out_packet = proto.ServerboundKeepAlive()
    out_packet.keepAliveId = in_packet.keepAliveId
    self.io.encode_packet(out_packet)
