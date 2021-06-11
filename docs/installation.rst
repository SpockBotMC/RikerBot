============
Installation
============

Build Requirements
==================

First, ensure you have all the build requirements available to build RikerBot.

Build Requirements:

* C++20 compiler (Only GCC 11.1+ as of writing, Clang 12 trunk works too)
* CPython  >= 3.5
* SWIG_    >= 4.0
* CMake_   >= 3.18
* Any cmake-supported build system, Ninja_ is recommended for performance
* Boost_   >= 1.73
* Botan_   >= 2.0.0
* zlib_, any version from this millenia
* python-minecraft-data_, latest
* setuptools_, any recent version
* wheel_, any recent version


Build Proccess
==============

Once you've got the requirements, you can build and install the framework from
the source root directory with::

  pip install .

RikerBot should be successfully installed. You can verify the process worked by
running ``import rikerbot`` from the Python REPL.

If you're interested in developing C++ extensions yourself you may wish to
build locally to preserve the cmake cache between compiles, you can do this
with::

  python setup.py bdist_wheel

This will create a ``dist`` folder containing the compiled module, which can
be installed with::

  pip install [file].whl

You may also wish to simply use cmake directly, and this is also supported.

.. _SWIG: http://www.swig.org/
.. _cmake: https://cmake.org/
.. _Ninja: https://ninja-build.org/
.. _Boost: https://www.boost.org/
.. _Botan: https://botan.randombit.net/
.. _zlib: https://zlib.net/
.. _python-minecraft-data: https://pypi.org/project/minecraft-data
.. _setuptools: https://pypi.org/project/setuputils/
.. _wheel: https://pypi.org/project/wheel/
