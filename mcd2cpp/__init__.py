import minecraft_data
from . import blocks, particles, protocol

def run(version):
  mcd = minecraft_data(version)
  protocol.run(mcd)
  particles.run(mcd)
  blocks.run(mcd)
