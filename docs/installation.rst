.. _installation:

============
Installation
============

Build Requirements
==================

First, ensure you have all the build requirements available to build RikerBot.

Build Requirements:

* C++20 compiler (Only GCC 10.1+ as of writing, Clang 12 trunk works too)
* CPython  >= 3.5
* SWIG_    >= 4.0
* CMake_   >= 3.18
* Any cmake-supported build system, Ninja_ is recommended for performance
* Boost_   >= 1.72
* Botan_   >= 2.0.0
* zlib_, any version from this millenia
* python-minecraft-data_, latest
* setuptools_, any recent version
* wheel_, any recent version


Build Proccess
==============

Once you've got the requirements, you need to build the C++ extensions using
cmake. If you are only interested in installing RikerBot as a user of the
framework and not developing C++ extensions yourself, you can build all of the
default extensions by running the following command in the project root::

  cmake . && cmake --build . --target rikerbot_all

Otherwise configure the cmake project however you like with your normal cmake
workflow.

Once the extensions are built, you can build the python wheel by running the
following in the project root::

  python setup.py bdist_wheel

This will create a ``dist`` folder in the project root containing a ``.whl``
file. You can install this file with ``pip install [file]`` and RikerBot should
be successfully installed. You can verify the process worked by running
``import rikerbot`` from the Python REPL.

.. _SWIG: http://www.swig.org/
.. _cmake: https://cmake.org/
.. _Ninja: https://ninja-build.org/
.. _Boost: https://www.boost.org/
.. _Botan: https://botan.randombit.net/
.. _zlib: https://zlib.net/
.. _python-minecraft-data: https://pypi.org/project/minecraft-data
.. _setuptools: https://pypi.org/project/setuputils/
.. _wheel: https://pypi.org/project/wheel/
