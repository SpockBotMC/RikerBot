from setuptools import setup, find_packages, Extension, Distribution
from setuptools.command.build_ext import build_ext
import os
import pathlib
import shutil

suffix = '.pyd' if os.name == 'nt' else '.so'
build_tool = 'Ninja' if shutil.which('ninja') else 'Unix Makefiles'
build_type = 'Release'


class RKRDistribution(Distribution):
  def iter_distribution_names(self):
    for pkg in self.packages or ():
      yield pkg
    for module in self.py_modules or ():
      yield module


class RKRExtension(Extension):
  def __init__(self, path):
    self.path = path
    super().__init__(pathlib.PurePath(path).name, [])


def find_extensions(directory):
  extensions = []
  for path, _, filenames in os.walk(directory):
    for filename in filenames:
      filename = pathlib.PurePath(filename)
      if pathlib.PurePath(filename).suffix == suffix:
        extensions.append(RKRExtension(os.path.join(path, filename.stem)))
  return extensions


class build_RKRExtensions(build_ext):
  def run(self):
    self.announce("Configuring CMake", level=3)
    source_dir = pathlib.PurePath(__file__).parent
    build_dir = source_dir / 'build' / build_type

    self.spawn([
        'cmake', '--no-warn-unused-cli',
        '-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE',
        f'-DCMAKE_BUILD_TYPE:STRING={build_type}', f'-G{build_tool}',
        f'-H{source_dir}', f'-B{build_dir}'
    ])

    self.announce("Building binaries", level=3)

    self.spawn([
        'cmake', '--build',
        str(build_dir), '--config',
        str(build_type), '--target', 'rikerbot_all', '-j', '14'
    ])

    self.extensions = find_extensions('rikerbot')

    for ext in self.extensions:
      source = f"{ext.path}{suffix}"
      ext_dir = pathlib.PurePath(self.get_ext_fullpath(ext.name)).parent
      os.makedirs(f"{ext_dir / pathlib.PurePath(ext.path).parent}",
                  exist_ok=True)
      shutil.copy(f"{source}", f"{ext_dir}/{source}")


setup(
    name='rikerbot',  # This comment prevents yapf formatting
    description='RikerBot is a framework for creating Python Minecraft Bots '
    'with C++ extensions',
    license='zlib',
    long_description=open('ReadMe.rst').read(),
    version='0.0.2',
    url='https://github.com/SpockBotMC/RikerBot',
    packages=find_packages(exclude=["mcd2cpp"]),
    keywords=['minecraft'],
    author="N. Vito Gamberini",
    author_email="vito@gamberini.email",
    classifiers=[
        'Development Status :: 2 - Pre-Alpha',
        'License :: OSI Approved :: zlib/libpng License',
        'Programming Language :: C++',
        'Programming Language :: Python :: 3 :: Only'
    ],
    ext_modules=[RKRExtension("rikerbot")],
    distclass=RKRDistribution,
    cmdclass={'build_ext': build_RKRExtensions})
