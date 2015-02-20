import sys
sys.path.append("swig_modules")

from automata import Organism as C_Organism
import grid_object

class OrganismError(Exception):
  def __init__(self, value):
    self.value = value
  def __str__(self):
    return repr(self.value)

""" The Python representation of an organism. """
class Organism(grid_object.GridObject):
  """ index: The index into the grid_objects array of the simulation this
  organism is part of.
  grid: The grid that this organism is part of.
  position: The position of the object on the grid, in the form (x, y). """
  def __init__(self, grid, index, position):
    # Data read from a configuration file that describes this organism.
    self.__attributes = {}

    # Underlying C++ organism. This object is shared with the Python GridObject
    # superclass, which makes sense seeing that the C++ version of Organism
    # inherits from GridObject.
    self._object = C_Organism(grid, index)
    if not self._object.Initialize(position[0], position[1]):
      raise OrganismError("Failed to initialize organism.")

  """ A convenient interface for getting this organism's attributes.
  name: The name of the attribute to get.
  Returns: The value of this attribute. """
  def __getattr__(self, name):
    if name in self.__attributes.keys():
      return self.__attributes[name]
    raise OrganismError("Organism has no attribute '%s'." % (name))

  """ Updates the status of this organism. Should be run every iteration. """
  def update(self):
    self.__organism.UpdatePosition()

  """ Sets the organism's attributes.
  attributes: The attribute data to set. """
  def set_attributes(self, attributes):
    self.__attributes = attributes
