import os
import json

import rikerbot


class SimpleClient:
  def __init__(self, extra_plugins=None, settings=None, online_mode=True, *,
               username='', tokens_path=None):
    extra_plugins = [] if extra_plugins is None else extra_plugins
    settings = {} if settings is None else settings
    if tokens_path is None:
      self.tokens_path = os.path.join(os.getcwd(), "token")

    self.ploader = rikerbot.PluginLoader()
    plugins = rikerbot.default_plugins + extra_plugins
    for failed in rikerbot.solve_dependencies(self.ploader, plugins, settings):
      rikerbot.logger.warning(f"Failed to meet plugin dependency: {failed}")

    ev = self.ploader.require('Event')
    ev.register_callback('auth_login_success', self.write_token)
    self._start = self.ploader.require('Start')
    self.auth = self.ploader.require('Auth')

    self.online_mode = online_mode

    if online_mode:
      if os.path.isfile(self.tokens_path):
        with open(self.tokens_path, 'r') as f:
          tokens = json.loads(f.read())
        self.auth.client_token = tokens['client_token']
        self.auth.access_token = tokens['access_token']
        self.auth.ygg.selected_profile = tokens['selected_profile']
      else:
        self.auth.username = username if username else input("Login Email: ")
        self.auth.password = input("Password: ")
    else:
      self.auth.username = username if username else input("Username: ")

  def __del__(self):
    rikerbot.delete_PluginLoader(self.ploader)

  def start(self, host="localhost", port=25565):
    self._start(host, port)

  def write_token(self, _, __):
    with open(self.tokens_path, 'w') as f:
      f.write(
          json.dumps({
              'client_token': self.auth.client_token,
              'access_token': self.auth.access_token,
              'selected_profile': self.auth.selected_profile
          }))
