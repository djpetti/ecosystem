import logging

import sys
sys.path.append("swig_modules")

from automata import Organism as C_Organism
import grid_object

logger = logging.getLogger(__name__)

class OrganismError(Exception):
  def __init__(self, value):
    self.value = value
  def __str__(self):
    return repr(self.value)

""" A small helper class that makes retrieving nested items possible. """
class AttributeHelper:
  """ attributes: The set of attributes managed by this instance. """
  def __init__(self, attributes):
    self._attributes = attributes

  """ Gets an attribute stored in the collection of this helper.
  name: The attribute name. """
  def __getattr__(self, name):
    if name in self._attributes.keys():
      attribute = self._attributes[name]
      if type(attribute) is dict:
        # We can go down another level.
        return AttributeHelper(attribute)

      # We're at the bottom.
      return attribute
    logger.log_and_raise(OrganismError,
        "Organism has no attribute '%s.'" % (name))

  """ Returns: A dictionary containing all the attributes. """
  def get_all_attributes(self):
    return self._attributes

""" The Python representation of an organism. """
class Organism(grid_object.GridObject, AttributeHelper):
  """ index: The index into the grid_objects array of the simulation this
  organism is part of.
  grid: The grid that this organism is part of.
  position: The position of the object on the grid, in the form (x, y). """
  def __init__(self, grid, index, position):
    # Data read from a configuration file that describes this organism.
    self._attributes = {}

    # Underlying C++ organism. This object is shared with the Python GridObject
    # superclass, which makes sense seeing that the C++ version of Organism
    # inherits from GridObject.
    self._object = C_Organism(grid, index)
    if not self._object.Initialize(position[0], position[1]):
      logger.log_and_raise(OrganismError, "Failed to initialize organism.")

  """ Updates the status of this organism. Should be run every iteration.
  Returns: True if it proceeds normally, false if this organism is dead or
  otherwise defunct."""
  def update(self):
    logger.debug("Updating organism %d." % (self.get_index()))

    if not self.is_alive():
      # It died.
      logger.info("Organism %d is dead." % (self.get_index()))
      return False

    if not self._object.UpdatePosition():
      logger.debug("Updating organism %d position failed." % \
          (self.get_index()))

      # Check to see if we have a conflict we can resolve.
      if not self._object.DefaultConflictHandler():
        # This is actually a significant error, because we either failed for a
        # reason other than being conflicted or failed to resolve the conflict.
        logger.log_and_raise(OrganismError,
            "Organism %d was either not conflicted or conflict"
            "resolution failed." % (self.get_index()))

    return True

  """ Sets the organism's attributes.
  attributes: The attribute data to set. """
  def set_attributes(self, attributes):
    logger.debug("Setting attributes of organism %d to %s." % \
        (self.get_index(), str(attributes)))

    self._attributes = attributes

  """ Returns whether or not the organism is alive. """
  def is_alive(self):
    return self._object.IsAlive()
