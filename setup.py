from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext
import os
import pathlib
import shutil

suffix = '.pyd' if os.name == 'nt' else '.so'

# top_level.txt is still wrong, but this is the best solution I could come up
# with for installing pre-compiled binaries and ensuring the wheel is marked
# as non-pure. Distutils is truly a wasteland. Thankfully, we're basically
# guaranteed not to conflict with anything and top_level.txt doesn't affect
# installation at all.
class RKRExtension(Extension):
  def __init__(self, path):
    self.path = path
    super().__init__(pathlib.PurePath(path).name, [])

class build_RKRExtensions(build_ext):
  def run(self):
    for ext in (x for x in self.extensions if isinstance(x, RKRExtension)):
      source = f"{ext.path}{suffix}"
      build_dir = pathlib.PurePath(self.get_ext_fullpath(ext.name)).parent
      os.makedirs(f"{build_dir}/{pathlib.PurePath(ext.path).parent}",
          exist_ok = True)
      shutil.copy(f"{source}", f"{build_dir}/{source}")

def find_extensions(directory):
  extensions = []
  for path, _, filenames in os.walk(directory):
    for filename in filenames:
      filename = pathlib.PurePath(filename)
      if pathlib.PurePath(filename).suffix == suffix:
        extensions.append(RKRExtension(os.path.join(path, filename.stem)))
  return extensions

# Very fragile check that the project has actually been compiled, while I work
# on a real solution to https://github.com/SpockBotMC/RikerBot/issues/4
if  not os.path.isfile('rikerbot/CLogger.py'):
  raise RuntimeError("Rikerbot must be compiled with cmake before it can be "
      "installed using this setup.py script")

setup(
  name = 'rikerbot',
  description = 'RikerBot is a framework for creating Python Minecraft Bots '
      'with C++ extensions',
  license = 'zlib',
  long_description = open('ReadMe.rst').read(),
  version = '0.0.1',
  url = 'https://github.com/SpockBotMC/RikerBot',
  packages = find_packages(exclude = ['mcd2cpp']),
  keywords = ['minecraft'],
  author = "N. Vito Gamberini",
  author_email = "vito@gamberini.email",
  classifiers = [
    'Development Status :: 2 - Pre-Alpha',
    'License :: OSI Approved :: zlib/libpng License',
    'Programming Language :: C++',
    'Programming Language :: Python :: 3 :: Only'
  ],
  ext_modules = find_extensions("rikerbot"),
  cmdclass = {'build_ext': build_RKRExtensions}
)
