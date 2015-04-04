import logging

logger = logging.getLogger(__name__)

import yaml
try:
  from yaml import CLoader as Loader
except ImportError:
  logger.warning("Falling back on Python yaml parser.")
  from yaml import Loader

from organism import Organism


class LibraryError(Exception):
  def __init__(self, value):
    self.__value = value
  def __str__(self):
    return repr(self.__value)


""" Class designed for importing and managing species from a species library.
"""
class Library:
  """ library_location: Where the library from which we want to import species
  is located. """
  def __init__(self, library_location):
    self.__library = library_location

  """ Loads an organism from the library.
  name: The organism's scientific name.
  grid: The grid to place this organism on.
  index: The index of the organism in the simulation grid.
  position: Where on the grid to place this organism, in the form (x, y).
  Returns: An organism object containing this organism. """
  def load_organism(self, name, grid, index, position):
    logger.debug("Loading '%s' from '%s'." % (name, self.__library))

    name = name.lower()
    # Add underscore
    name = name.replace(" ", "_")

    organism_file = open("%s/%s.yaml" % (self.__library, name))
    data = yaml.load(organism_file, Loader = Loader)
    organism_file.close()

    organism = Organism(grid, index, position)
    organism.set_attributes(data)

    if grid.scale() < 0:
      # This is the first organism we added.
      logger.info("Setting grid scale to %f." % (organism.Scale))
      grid.set_scale(organism.Scale)
    elif organism.Scale != grid.scale():
      logger.log_and_raise(LibraryError,
          "Mismatch between object scale %f and grid scale %f." % \
          (organism.Scale, grid.scale()))

    return organism
