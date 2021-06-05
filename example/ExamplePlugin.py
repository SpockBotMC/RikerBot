# An example plugin that will respond to any chat message with, "Hello,
# I am RikerBot".

from rikerbot import PluginBase, pl_announce, proto, logger


class ExampleCore:
  def __init__(self, greeting_string="Hello, I am RikerBot"):
    self.greeting_string = greeting_string


# The pl_announce decorator tells the plugin loader about any interfaces
# provided by the plugin, and is required for dependency resolution. These
# interfaces are usually called "Cores", and can be requested by other plugins
# to use as APIs.
@pl_announce("Example")
# All plugins inherit from the PluginBase, while not strictly required for
# Python plugins, it provides many helpful utilities for your plugin to
# automatically interact with the plugin loader.
class ExamplePlugin(PluginBase):

  # The `requires` iterable lists any other interfaces ("Cores") your plugin
  # needs in order to function. In this case we need the IO Core in order to
  # send out our chat message.
  # The `requires` strings are case sensitive, and will be assigned to your
  # plugin object as lower-case attributes upon initializing the PluginBase.
  requires = ("IO", )

  # The `defaults` dict is a list of default settings that you want for your
  # plugin. Any settings not provided by the user will be initialized with
  # these settings upon initializing the PluginBase.
  defaults = {
      'greeting_string': "Hello, I am Rikerbot",
  }

  # The `events` dict maps events to callbacks from your plugin object. The
  # event system will call your method when another plugin signals the event
  # has occured.
  events = {
      # All protocol events use the direction of the packet, either
      # "Clientbound" or "Serverbound", followed by the name of the packet
      # given by Minecraft Data.
      'ClientboundChat': 'handle_incoming_chat'
  }

  # Init is always handed two parameters, the plugin loader itself, and a
  # settings dictionary, which may be empty. When using PluginBase, you should
  # immediately initialize it to setup your requires/defaults/events fields
  def __init__(self, ploader, settings):
    super().__init__(ploader, settings)

    # The greeting_string setting will be "Hello, I am RikerBot" unless a user
    # has provided a custom setting.
    self.core = ExampleCore(settings['greeting_string'])

    # Cores are provided to other plugins using the plugin loader's .provide()
    # method. The plugin loader does not restrict you to just class objects,
    # any PyObject can be provided to other Python plugins using this method.
    # By the end of initialization, any names you declared in pl_announce must
    # be provided to the plugin loader.
    # In this example, other plugins could use the ExampleCore in order to
    # change the greeting string.
    ploader.provide("Example", self.core)

    # You do not need to use the `requires` iterable to load the Event Core,
    # it is loaded implicity by the plugin loader and is always available.
    # In this case, you can use the .require() method to get the Event Core,
    # and this is typically done when you won't need it outside of
    # initialization.
    event = ploader.require("Event")

    # When creating an event, you need to register it with the Event Core. The
    # return value will be a handle you can use to emit events later.
    self.greeting_sent = event.register_event("example_greeting_sent")

    # For this example we will need the Event Core later, so we'll hold onto
    # it. In a normal plugin you would just include it in the `requires` list
    # if this were the case.
    self.event = event

  # Callbacks to object methods are handed two pieces of data, the event id
  # that is responsible for the call (which is the handle that was created by
  # register_event), and some data.
  # The event_id field can be used to distinguish which event is responsible
  # for the callback when registering a method for multiple events, but that
  # pattern is discouraged.
  # The data is dependent on the event, and might be `None` if no data is
  # provided by the event. For this example the data will be the
  # ClientboundChat packet.
  #
  # !!!WARNING!!! Data is read-only, if you need to alter data, make a copy
  # of it first. This is due to how C++ plugins interact with the event system.
  def handle_incoming_chat(self, event_id, packet):
    # Riker provides its own logging facilities, the interface is similar to
    # the Python logging module
    logger.info(f"Received Message: {packet.message}")

    # Packets have no useful methods, they are just collections of fields to be
    # filled and then sent to I/O. Typically only lower level plugins should
    # be dealing with raw packets, but for this Example it's convenient to
    # illustrate the capability.
    response = proto.ServerboundChat()

    # The fields of packets are named based on the Minecraft Data naming
    # conventions.
    response.message = self.core.greeting_string

    self.io.encode_packet(response)

    # Events with no data will be emitted to Python and C++ plugins with the
    # data field of their callbacks filled with None/nullptr. When a PyObject
    # is emitted as data, it will be emitted in its native form unless a
    # TypeQuery string is provided to convert it to a C/C++ object. That is
    # advanced usage that you don't usually need to worry about though.
    self.event.emit(self.greeting_sent)


if __name__ == "__main__":
  from rikerbot import SimpleClient
  # Settings dicts map a name to a dictionary of settings
  settings = {
      'example': {
          'greeting_string': input("Please provide a greeting: ")
      }
  }
  # Plugin lists are lists of tuples that provide a name (to be looked up in
  # the settings dict), and a plugin associated with that name. The plugin will
  # be provided with the settings associated with its name.
  plugins = [('example', ExamplePlugin)]
  client = SimpleClient(plugins, settings, online_mode=False)
  client.start(host="localhost", port=25565)
