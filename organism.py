class OrganismError(Exception):
  def __init__(self, value):
    self.__value = value
  def __str__(self):
    return repr(self.__value)


import logging
import random

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
    raise AttributeError("Organism has no attribute '%s'." % (name))

  """ Returns: A dictionary containing all the attributes. """
  def get_all_attributes(self):
    return self._attributes


""" The Python representation of an organism. """
class Organism(grid_object.GridObject, AttributeHelper):
  """ index: The index into the grid_objects array of the simulation this
  organism is part of.
  grid: The grid that this organism is part of.
  position: The position of the object on the grid, in the form (x, y).
  sex: Optional parameter, allows us to specify the sex of the organism. 1 means
       male, 0 means female. """
  def __init__(self, grid, position, sex=None):
    # Data read from a configuration file that describes this organism.
    self._attributes = {}

    # Handlers that apply to this organism.
    self.__handlers = []
    self.__grid = grid

    # The sex of the organism. This is determined at creation time.
    if sex != None:
      self.__sex = sex
    else:
      # Choose a random one.
      self.__sex = random.randint(0, 1)
      logger.debug("Setting organism sex: %d\n", self.__sex)
    # Whether or not the organism is pregnant.
    self.__pregnant = False

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

  """ Sets the organism's attributes. Also does some initialization that can
  only be done after the attributes are set.
  attributes: The attribute data to set. """
  def set_attributes(self, attributes):
    logger.debug("Setting attributes of organism %d to %s." % \
        (self.get_index(), str(attributes)))

    self._attributes = attributes

    # Figure out which handlers apply to us.
    UpdateHandler.set_handlers_static_filtering(self)

    # As soon as an organism comes online, we need to create movement factors
    # representing it and add them for every organism that will be affected by
    # this one. It doesn't matter if we add movement factors to plants, because
    # their positions never get updated anyway.
    for organism in self.grid_objects:
      if isinstance(organism, Organism):
        if (hasattr(self, "Prey") and organism.scientific_name() in self.Prey):
          # Add a movement factor that causes them to flee us.
          organism.add_factor_from_organism(self, True)
          # Add a movement factor that causes us to be attracted to them.
          self.add_factor_from_organism(organism, False)
        elif (hasattr(organism, "Prey") and \
              self.scientific_name() in organism.Prey):
          # Add a movement factor that causes them to be attracted to us.
          organism.add_factor_from_organism(self, False)
          # Add a movement factor that causes us to flee them.
          self.add_factor_from_organism(organism, True)

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
    logger.debug("Got conflicted object with index %d." % \
                 (conflicted.get_index()))

    # Associate this object with a Python grid object.
    conflicted = grid_object.GridObject.get_by_index(conflicted.get_index())
    if conflicted == self:
      logger.log_and_raise(ValueError, "Got conflict with myself?!")

    # Decide whether to use predation.
    if self.__handle_predation(conflicted):
      # That worked, we're done.
      return

    # Handle mating. Whether this is successful or not, we still need to run the
    # default conflict handler to actually move an organism.
    self.__handle_sexual_reproduction(conflicted)
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

  """ Handles sexual reproduction. This is the most complicated type of
  reproduction, because it requires two organisms in the same location. That is
  why it is implemented as a conflict handler.
  Args:
    conflicted: The organism we are conflicting with.
  Returns:
    True if the conflict was resolved, False otherwise. """
  def __handle_sexual_reproduction(self, conflicted):
    # First of all, we need to be the same species.
    if not self.same_species(conflicted):
      return False

    if self.get_sex() + conflicted.get_sex() != 1:
      # Despite SCOTUS, we don't implement gay marriage.
      return False

    # TODO (danielp): Eventually we'll need a better way of determining who's
    # attracted to whom, but for now, we'll just use a fixed (configurable)
    # probability of amorous intentions. (Note that, for now, all our
    # love-making is entirely consentual.)
    attracted_to_them = random.random()
    if attracted_to_them > self.Reproduction.WantsSex:
      return False
    attracted_to_us = random.random()
    if attracted_to_us > conflicted.Reproduction.WantsSex:
      return False

    # Now they're going to do it like they do on the Discovery Channel.
    logger.info("Organism %d is mating with organism %d." % \
                (self.get_index(), conflicted.get_index()))
    # Of course, there's only a certain probability that they actually made a
    # baby.
    conceived = random.random()
    if conceived < self.Reproduction.ConceptionProbability:
      if not self.get_sex():
        # We're pregnant.
        logger.info("Organism %d is pregnant." % (self.get_index()))
        self.__pregnant = True
      else:
        # We're the daddy.
        logger.info("Organism %d is pregnant." % (conflicted.get_index()))
        conflicted.__pregnant = True

    return True

  """ Gets the sex of this organism. We could make jokes about the name of this
  method, but we're not going to.
  Returns: 0 if the organism is female, 1 if it's male. """
  def get_sex(self):
    return self.__sex

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

    # Remove any lingering references to ourselves hanging around in the C++
    # code.
    for organism in grid_object.GridObject.grid_objects:
      if not isinstance(organism, Organism):
        continue
      organism.cleanup_organism(self)

  """ Adds a movement factor from a specific organism.
  organism: The organism to use for the factor.
  prey: True if we are the prey of that organism. Otherwise, we are the
  predator. """
  def add_factor_from_organism(self, organism, prey):
    logger.debug("%d: Adding factor for %d. Prey: %s" % (self.get_index(),
                                                         organism.get_index(),
                                                         prey))

    if prey:
      strength = self.Metabolism.Animal.PredatorFactorStrength
      visibility = self.Metabolism.Animal.PredatorFactorVisibility
    else:
      strength = self.Metabolism.Animal.PreyFactorStrength
      visibility = self.Metabolism.Animal.PreyFactorVisibility

    # Actually add the factor.
    self._object.AddFactorFromOrganism(organism._object, strength,
                                         visibility)

  """ Removes all references to another organism from this organism.
  organism: The organism to remove references to. """
  def cleanup_organism(self, organism):
    logger.debug("%d: Cleaning up %d." % (self.get_index(),
                                          organism.get_index()))

    self._object.CleanupOrganism(organism._object)

  """ Sets how far away the organism can percieve movement factors.
  vision: The new value for the organism's vision. """
  def set_vision(self, vision):
    self._object.set_vision(vision)

  """ Returns: The organism's vision, which is how far away it can perceive
  movement factors. """
  def get_vision(self):
    return self._object.get_vision()

  """ Decides whether two organisms are of the same species.
  Args:
    other: The other organism we are checking with.
  Returns: True if they are, False otherwise. """
  def same_species(self, other):
    return (self.Taxonomy.Genus == other.Taxonomy.Genus and \
            self.Taxonomy.Species == other.Taxonomy.Species)

  """ Returns: True if organism is pregnant, False otherwise. """
  def get_pregnant(self):
    return self.__pregnant
