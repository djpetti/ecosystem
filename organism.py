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
  position: The position of the object on the grid, in the form (x, y). """
  def __init__(self, grid, position):
    # Data read from a configuration file that describes this organism.
    self._attributes = {}

    # Handlers that apply to this organism.
    self.__handlers = []
    self.__grid = grid

    # Metabolism handler for this organism. A handler will initialize it,
    # because it is unique depending on the organism.
    self.metabolism = None

    # Underlying C++ organism. This object is shared with the Python GridObject
    # superclass, which makes sense seeing that the C++ version of Organism
    # inherits from GridObject.
    self._object = C_Organism(self.__grid,
                              grid_object.GridObject.current_index)
    if not self._object.Initialize(position[0], position[1]):
      logger.log_and_raise(OrganismError, "Failed to initialize organism.")

    # Add the organism to the list of grid objects.
    grid_object.GridObject._add_object(self)

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

  """ Resolves a conflict in the best way possible. """
  def handle_conflict(self):
    # Get the organism that we are conflicting with. We are going to be in the
    # conflicted slot, so we want the pending organism.
    position = self.get_position()
    conflicted = self._object.GetConflict()
    logger.debug("Got pending object with index %d." % \
                 (conflicted.get_index()))

    # Associate this object with a Python grid object.
    conflicted = grid_object.GridObject.get_by_index(conflicted.get_index())
    if conflicted == self:
      logger.log_and_raise(ValueError, "Got conflict with myself?!")

    # Decide whether to use predation.
    if self.__handle_predation(conflicted):
      # That worked, we're done.
      return

    # Fall back on the default conflict handler.
    self.__default_conflict_handler()

  """ Runs the default conflict handler on this organism. """
  def __default_conflict_handler(self):
    logger.info("Using default conflict handler for %d." % (self.get_index()))

    if not self._object.DefaultConflictHandler():
      # This is actually a significant error, because we either failed for a
      # reason other than being conflicted or failed to resolve the conflict.
      logger.log_and_raise(OrganismError,
          "Organism %d was either not conflicted or conflict"
          "resolution failed." % (self.get_index()))

  """ Checks whether this conflict should be resolved through predation.
  conflicted: The organism we are conflicting with.
  Returns: True if the conflict was resolved, False if it couldn't be. """
  def __handle_predation(self, conflicted):
    try:
      conflicted_prey = conflicted.Prey
    except AttributeError:
      # Conflicted is not a type of organism that can eat us.
      conflicted_prey = []
    try:
      our_prey = self.Prey
    except AttributeError:
      # We are not a type of organism that can eat anything.
      our_prey = []

    if self.scientific_name() in conflicted_prey:
      # We are going to get eaten.
      logger.info("Organism %d is consuming organism %d." % \
                   (conflicted.get_index(), self.get_index()))
      conflicted.metabolism.Consume(self.metabolism)
      # Now we're dead.
      self.die()
      return True
    elif conflicted.scientific_name() in our_prey:
      # We are going to eat them.
      logger.info("Organism %d is consuming organism %d." % \
                  (self.get_index(), conflicted.get_index()))
      self.metabolism.Consume(conflicted.metabolism)
      # Now they're dead.
      conflicted.die()
      return True

    return False

  """ Get the handlers that apply to this organism. """
  def get_handlers(self):
    return self.__handlers

  """ Returns: The scientific name of the organism. (genus species) """
  def scientific_name(self):
    return "%s %s" % (self.Taxonomy.Genus, self.Taxonomy.Species)

  """ Causes the organism to die. """
  def die(self):
    logger.info("Organism %d is dying." % (self.get_index()))
    self._object.Die()

    # Delete ourselves from the grid_objects array and from the grid.
    self.delete()
