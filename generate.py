import sys

version = sys.argv[1].replace(".", "_")
swig_interface = (
  f'%module Proto{version}',
  '%{',
  f'#include "proto_{version}.hpp"',
  '%}',
  '%feature ("flatnested");',
  '',
  '%include <stdint.i>',
  '%include <std_string.i>',
  '%include "typemaps/unique_ptr.i"',
  '%unique_ptr(mcd::Packet)',
  '%rename(_property) property;',
  '',
  '%typemap(out) (std::vector<char>) {',
  '  $result = PyBytes_FromStringAndSize($1.data(), $1.size());',
  '}',
  '',
  '%include "datautils.hpp"',
  '%include "particletypes.hpp"',
  f'%include "proto_{version}.hpp"',
  '',
)

with open(f"Proto{version}.i", "w") as f:
  f.write("\n".join(swig_interface))

with open(f"MinecraftProtocol.py", "w") as f:
  f.write(f"from .Proto{version} import *\n")

with open(f"minecraft_protocol.hpp", "w") as f:
  f.write(f'#include "proto_{version}.hpp"\n')

import mcd2cpp
mcd2cpp.run(sys.argv[1])
