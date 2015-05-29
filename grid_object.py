import logging

from swig_modules.automata import GridObject as C_GridObject


logger = logging.getLogger(__name__)


class GridObjectError(Exception):
  def __init__(self, value):
    self.value = value
  def __str__(self):
    return repr(self.value)


""" A generic superclass for representing objects that are on the grid.
Basically a thin wrapper around the C++ version of this class."""
class GridObject(C_GridObject):
  # A global list of all the grid objects in this simulation.
  grid_objects = []
  # The current index we are on.
  current_index = 0

  """ grid: The grid to create this object on.
  position: The coordinates of the objects."""
  def __init__(self, grid, position):
    self._object = C_GridObject(grid, GridObject.current_index);

    if not self._object.Initialize(position[0], position[1]):
      raise GridObjectError("Failed to initialize grid object.")

    GridObject._add_object(self)

  """ We always have references sitting around in the grid_objects list, so this
  method deletes it manually.
  IMPORTANT: This should be called whenever someone is done with a grid object,
  otherwise, it will NEVER be garbage collected! """
  def delete(self):
    # Remove ourselves from the grid.
    self.remove()

    # Remove ourselves from the list. To do this, we need to decrement the
    # indices of everyone behind us.
    for i in range(self.get_index() + 1, len(GridObject.grid_objects)):
      grid_object = GridObject.grid_objects[i]
      grid_object.set_index(i - 1)

    GridObject.grid_objects.pop(self.get_index())

  """ Add a new object to the list of grid objects, and increments
  current_index.
  grid_object: The object to add. """
  @classmethod
  def _add_object(cls, grid_object):
    logger.debug("Adding grid object with index %d." % (cls.current_index))

    cls.grid_objects.append(grid_object)
    cls.current_index += 1

  """ Gets a grid object from the list of grid objects by index.
  index: The index of the object we are looking for.
  Returns: The grid object with the specified index. """
  @classmethod
  def get_by_index(cls, index):
    grid_object = cls.grid_objects[index]
    if grid_object.get_index() != index:
      logger.log_and_raise(ValueError,
          "Grid object has set index %d, but is at index %d." % \
          (grid_object.get_index(), index))

    return grid_object

  """ Removes all the objects from the grid_objects list and resets
  current_index. This could easily break stuff, so it should only really be used
  for testing purposes or if you really know what you are doing. """
  @classmethod
  def clear_objects(cls):
    cls.grid_objects = []
    cls.current_index = 0

  """ Does whatever changes that are required for this object from one iteration
  to the next. """
  def update(self):
    logger.log_and_raise(NotImplementedError,
        "'update' must be implemented by subclass.")

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
    logger.debug("Setting position of object %d to %s." % \
        (self.get_index(), str(position)))

    if not self._object.SetPosition(position[0], position[1]):
      logger.log_and_raise(GridObjectError,
          "Failed to set the object's position.")

  """ Returns: The current object's position in the form (x, y). """
  def get_position(self):
    return tuple(self._object.get_position())

  """ Remove ourselves from the grid. It is okay to call this more than once.
  """
  def remove(self):
    if not self._object.RemoveFromGrid():
      logger.log_and_raise(GridObjectError,
          "Failed to remove the object from the grid.")
