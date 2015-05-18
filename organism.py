class OrganismError(Exception):
  def __init__(self, value):
    self.__value = value
  def __str__(self):
    return repr(self.__value)


import logging

from swig_modules.automata import Organism as C_Organism
from update_handler import UpdateHandler
import grid_object

logger = logging.getLogger(__name__)


""" A small helper class that makes retrieving nested items possible. """
class AttributeHelper:
  """ attributes: The set of attributes managed by this instance. """
  def __init__(self, attributes):
    self._attributes = attributes

    # A list of handlers that apply to this organism.
    self.__handlers = []

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
    logger.log_and_raise(AttributeError,
        "Organism has no attribute '%s'." % (name))

  """ Returns: A dictionary containing all the attributes. """
  def get_all_attributes(self):
    return self._attributes

""" The Python representation of an organism. """
class Organism(grid_object.GridObject, AttributeHelper):
  """ index: The index into the grid_objects array of the simulation this
  organism is part of.
  grid: The grid that this organism is part of.
  index: The organism's index in the simulation list.
  position: The position of the object on the grid, in the form (x, y). """
  def __init__(self, grid, index, position):
    # Data read from a configuration file that describes this organism.
    self._attributes = {}

    # Handlers that apply to this organism.
    self.__handlers = []

    # Metabolism handler for this organism. A handler will initialize it,
    # because it is unique depending on the organism.
    self.metabolism = None

    # Underlying C++ organism. This object is shared with the Python GridObject
    # superclass, which makes sense seeing that the C++ version of Organism
    # inherits from GridObject.
    self._object = C_Organism(grid, index)
    if not self._object.Initialize(position[0], position[1]):
      logger.log_and_raise(OrganismError, "Failed to initialize organism.")

  """ Updates the status of this organism. Should be run every iteration.
  iteration_time: Simulation time since the last iteration.
  Returns: True if it proceeds normally, false if this organism is dead or
  otherwise defunct. """
  def update(self, iteration_time):
    logger.debug("Updating organism %d." % (self.get_index()))

    if not self.is_alive():
      # It died.
      logger.info("Organism %d is dead." % (self.get_index()))
      return False

    # Run handlers.
    for handler in self.__handlers:
      handler.handle_organism(self, iteration_time)

    return True

  """ Sets the organism's attributes.
  attributes: The attribute data to set. """
  def set_attributes(self, attributes):
    logger.debug("Setting attributes of organism %d to %s." % \
        (self.get_index(), str(attributes)))

    self._attributes = attributes

    # Figure out which handlers apply to us.
    UpdateHandler.set_handlers_static_filtering(self)

  """ Returns whether or not the organism is alive. """
  def is_alive(self):
    return self._object.IsAlive()

  """ Adds a handler as one that will handle this organism when it is updated.
  handler: handler to add. """
  def add_handler(self, handler):
    self.__handlers.append(handler)

  """ Updates the position of the organism. """
  def update_position(self):
    if not self._object.UpdatePosition():
      logger.log_and_raise(OrganismError,
          "Updating organism %d position failed." % (self.get_index()))

  """ Runs the default conflict handler on this organism. """
  def default_conflict_handler(self):
    logger.info("Using default conflict handler for %d." % (self.get_index()))

    if not self._object.DefaultConflictHandler():
      # This is actually a significant error, because we either failed for a
      # reason other than being conflicted or failed to resolve the conflict.
      logger.log_and_raise(OrganismError,
          "Organism %d was either not conflicted or conflict"
          "resolution failed." % (self.get_index()))

  """ Get the handlers that apply to this organism. """
  def get_handlers(self):
    return self.__handlers

  """ Returns: The scientific name of the organism. (genus species) """
  def scientific_name(self):
    return "%s %s" % (self.Taxonomy.Genus, self.Taxonomy.Species)
