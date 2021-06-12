from setuptools import setup, Extension, Distribution
from setuptools.command.build_ext import build_ext
import os
import pathlib
import shutil

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

    mod = pathlib.PurePath(__file__).parent / 'rikerbot'
    shutil.copytree(str(mod), os.path.join(self.build_lib, 'rikerbot'),
                    ignore=shutil.ignore_patterns('*.pyc', '__pycache__'),
                    dirs_exist_ok=True)


setup(
    name='rikerbot',
    description='RikerBot is a framework for creating Python Minecraft Bots '
    'with C++ extensions',
    license='zlib',
    long_description=open('ReadMe.rst').read(),
    version='0.0.3',
    url='https://github.com/SpockBotMC/RikerBot',
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
    cmdclass={'build_ext': build_RKRExtensions},
)
