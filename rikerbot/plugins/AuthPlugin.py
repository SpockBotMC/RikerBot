from hashlib import sha1
from urllib import request, error
import json

from rikerbot import logger
from rikerbot.proto.yggdrasil import Yggdrasil
from rikerbot.PluginBase import PluginBase, pl_announce


class AuthCore:
  def __init__(self, event, online_mode, auth_timeout):
    self.event = event
    self.online_mode = online_mode
    self.auth_timeout = auth_timeout
    self.ygg = Yggdrasil()
    self._username = None
    self.login_success = event.register_event('auth_login_success')
    self.login_error = event.register_event('auth_login_error')

  def set_username(self, username):
    self.ygg.username = username

  username = property(lambda self: self._username, set_username)

  def set_password(self, password):
    self.ygg.password = password

  password = property(lambda self: bool(self.ygg.password), set_password)

  def set_client_token(self, token):
    self.ygg.client_token = token

  client_token = property(lambda self: self.ygg.client_token, set_client_token)

  def set_access_token(self, token):
    self.ygg.access_token = token

  access_token = property(lambda self: self.ygg.access_token, set_access_token)

  def set_access_token(self, token):
    self.ygg.access_token = token

  access_token = property(lambda self: self.ygg.access_token, set_access_token)

  selected_profile = property(lambda self: self.ygg.selected_profile)

  def login(self):
    if not self.online_mode:
      self._username = self.ygg.username
      return True
    if self.ygg.login():
      self._username = self.ygg.selected_profile['name']
      logger.info(f"Successful Login, username is: {self._username}")
      self.event.emit(self.login_success)
      return True
    self.event.emit(self.login_error)


@pl_announce('Auth')
class AuthPlugin(PluginBase):
  requires = ('Event', 'IO')
  events = {
      'ClientboundEncryptionBegin': 'handle_session_auth',
  }
  defaults = {
      'online_mode': True,
      'auth_timeout': 3,  # No idea how long this should be, 3s seems good
      'auth_quit': True,
      'sess_quit': True,
  }

  def __init__(self, ploader, settings):
    super().__init__(ploader, settings)
    self.auth_timeout = self.settings['auth_timeout']
    self.core = AuthCore(self.event, self.auth_timeout,
                         self.settings['online_mode'])
    ploader.provide('Auth', self.core)
    self.session_auth = self.event.register_event("auth_session_success")

  def handle_session_auth(self, event_id, packet):
    # Server ID is blank for Notchian servers, but some custom servers sitll
    # use it
    sev_id = packet.serverId.encode('ascii')
    digest = sha1(sev_id + self.io.shared_secret + packet.publicKey).digest()
    data = json.dumps({
        'accessToken':
        self.core.ygg.access_token,
        'selectedProfile':
        self.core.ygg.selected_profile['id'],
        'serverId':
        format(int.from_bytes(digest, 'big', signed=True), 'x')
    }).encode('utf-8')
    url = 'https://sessionserver.mojang.com/session/minecraft/join'
    req = request.Request(url, data, {'Content-Type': 'application/json'})
    try:
      rep = request.urlopen(req, timeout=self.auth_timeout)
      rep = rep.read().decode('utf-8')
    except error.URLError:
      rep = "Couldn't connect to sessionserver.mojang.com"
    if rep:
      logger.warning(rep)
    else:
      logger.info("Successful Session Auth")
      self.event.emit(self.session_auth, packet,
                      "mcd::ClientboundEncryptionBegin *")
