==========
 RikerBot
==========
--------------------------------------
 The Next Generation of Minecraft Bot
--------------------------------------

Development repo for a C++20/Python based Minecraft bot. Not much to see here
right now.

Things Riker can do today:
 * Connect
 * Very technically parse all packets, as long as they're not compressed or
   encrypted.

Feature Roadmap:
 * Authentication
 * |ss| Encryption |se|
 * Compression
 * Chunk Parsing
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
* Botan-2 (crypto, zlib)
* SWIG 4.0
* Python3
* The latest git of `python-minecraft-data <https://github.com/SpockBotMC/python-minecraft-data>`_

Get Involved
------------
Please open issues or better yet, a pull request for things you would like to
see in Riker. You can find `me <https://github.com/nickelpro>`_ on Freenode
#mcdevs as ``nickelpro`` or reach out to me through email or social media.

.. |ss| raw:: html

   <strike>

.. |se| raw:: html

   </strike>
