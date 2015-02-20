import sys
sys.path.append("swig_modules")

from automata import GridObject as C_GridObject

class GridObjectError(Exception):
  def __init__(self, value):
    self.value = value
  def __str__(self):
    return repr(self.value)

""" A generic superclass for representing objects that are on the grid.
Basically a thin wrapper around the C++ version of this class."""
class GridObject(C_GridObject):
  """ index: The object's index in the simulation's grid_objects array.
  x: The x coordinate of the object.
  y: The y coordinate of the object. """
  def __init__(self, grid, index, x, y):
    self._object = C_GridObject(grid, index);

    if not self._object.Initialize(x, y):
      raise GridObjectError("Failed to initialize grid object.")

  """ Does whatever changes that are required for this object from one iteration
  to the next. """
  def update(self):
    raise NotImplementedError("'update' must be implemented by subclass.")

  """ Changes the index for this object.
  index: The new index. """
  def set_index(self, index):
    self._object.set_index(index)

  """ Gets the current index for this object.
  Returns: The object's index. """
  def get_index(self):
    return self._object.get_index()

  """ Sets the current position of this object.
  position: The object's position in the form (x, y). """
  def set_position(self, position):
    if not self._object.SetPosition(position[0], position[1]):
      raise GridObjectError("Failed to set the object's position.")

  """ Returns: The current object's position in the form (x, y). """
  def get_position(self):
    return tuple(self._object.get_position())
