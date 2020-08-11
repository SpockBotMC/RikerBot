==========
 RikerBot
==========
--------------------------------------
 The Next Generation of Minecraft Bot
--------------------------------------

Development repo for a C++20/Python based Minecraft bot. Not much to see here
right now.

Things Riker can do today:
 * Full protocol support, including authentication

Why build Riker and not work on an existing project?:
 * Built from the ground up around code generation. This makes the maintenance
   burden of keeping up with Mojang much lower than some other projects.
 * Fun and Accessible Python API when you want it, High Performance C++ API
   when you need it.
 * If you want to build a modern Minecraft bot using only open source
   technology (not modding the Mojang client, Malmo-style), your only option
   today is Mineflayer_. Mineflayer and all of PrismarineJS are excellent
   projects, but maybe you're not a NodeJS programmer. RikerBot brings much
   needed diversity to the Minecraft bot ecosystem.

Feature Roadmap:
 * Chunk Parsing :running:
 * Physics
 * Pathfinding

Framework Nice-To-Haves:
 * Multi-threaded job system
 * More packets accessible in Python

Housekeeping:
 * Spin mcd2cpp out into its own project

Build Requiremenets
--------------------

* cmake
* C++20 compiler, likely only GCC 10.1+ works right now
* Boost >= 1.72 (asio, logging, streambufs)
* zlib (compression)
* Botan-2 (crypto)
* SWIG 4.0
* Python3
* The latest git of `python-minecraft-data <https://github.com/SpockBotMC/python-minecraft-data>`_

Get Involved
------------
Please open issues or better yet, a pull request for things you would like to
see in Riker. You can find `me <https://github.com/nickelpro>`_ on Freenode
#mcdevs as ``nickelpro`` or reach out to me through email or social media.

Special Thanks
--------------
Lots of people have contributed to the third-party Minecraft community, too
many to list here. These are some people who's work I interact with everytime
I work on Minecraft.

* `TkTech <https://github.com/TkTech>`_, **Grand Poobah of Third-Party
  Minecraft**, keeps the lights on for us lesser devs.

* `Pokechu22 <https://github.com/Pokechu22>`_, **Master Scribe and Artisan**,
  documents all things Minecraft great and small and maintains irreplacable
  infrastructure like Burger.

* `rom1504 <https://github.com/rom1504>`_, **Chairman of the Prismarine**, this
  project literally doesn't exist without the hard work of Rom and all the
  other Prismarine contributors.

And thanks to my friends `gjum <https://github.com/Gjum>`_, and
`gamingrobot <https://github.com/gamingrobot>`_. SpockBot walked so that
RikerBot could run and knowing that people liked my little block game robot
enough to so significantly improve it meant and means a lot.


.. _Mineflayer: https://github.com/PrismarineJS/mineflayer
