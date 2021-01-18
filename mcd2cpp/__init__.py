import minecraft_data
from . import blocks, particles, protocol, shapes

def run(version):
  mcd = minecraft_data(version)
  protocol.run(mcd)
  particles.run(mcd)
  shapes.run(mcd)
  blocks.run(mcd)
