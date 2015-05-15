class HandlerError(Exception):
  def __init__(self, value):
    self.__value = value

  def __str__(self):
    return repr(self.__value)


import inspect
import logging
import sys

from organism import OrganismError
from swig_modules.automata import PlantMetabolism
import user_handlers


logger = logging.getLogger(__name__)


""" Defines a common superclass for all update handlers. An update handler is
something that gets run every iteration of the simulation on a filtered subset
of the organisms being updated. This framework is designed so that users
can easily implement custom handlers. """
class UpdateHandler:
  """ A list of all the handlers currently known to this simulation. """
  handlers = []

  """ Checks if an organism matches the static filtering criteria for all the
  registered handlers, and add the handler to the organism's list of handlers.
  """
  @classmethod
  def set_handlers_static_filtering(cls, organism):
    for handler in cls.handlers:
      if handler.check_static_filters(organism):
        # It meets the criteria.
        organism.add_handler(handler)

  """ All subclasses should call this constructor. """
  def __init__(self):
    # A dictionary story what attribute values we are filtering for.
    self.__static_filters = {}

    # Register handler.
    logger.info("Registering handler '%s'." % (self.__class__.__name__))
    UpdateHandler.handlers.append(self)

  """ Specifies that only organisms that have a particular attribute set in a
  particular way will be handled by this handler. These handlers will be run
  once upon creation of every new organism.
  attribute: Specified attribute.
  values: What that attribute needs to equal. Can be a list if there are
  multiple things. """
  def filter_attribute(self, attribute, value):
    collection = (type(value) == list or type(value) == tuple)
    if not collection:
      if attribute in self.__static_filters.keys():
        # A single value for an attribute we have added.
        self.__static_filters[attribute].append(value)
      else:
        # A single value for an attribute we haven't yet added.
        self.__static_filters[attribute] = [value]
    else:
      if attribute in self.__static_filters.keys():
        # A collection for an attribute we have added.
        self.__static_filters[attribute].extend(list(value))
      else:
        # A collection for an attribute we haven't yet added.
        self.__static_filters[attribute] = list(value)

  """ Specifies a custom dynamic filter that every organism that meets the
  criteria of the static filters gets put through every time it is being
  updated.
  organism: The organism to check if we should handle.
  Returns: True if we should handle this organism, false if we shouldn't. """
  def dynamic_filter(self, organism):
    # By default, do nothing.
    return True

  """ Runs the actual body of the handler. This is designed to be implemented by
  the user in superclasses.
  organism: The organism to run the handler on. """
  def run(self, organism):
    raise NotImplementedError("'run' must be implemented by subclass.")

  """ Determines whether a paticular organism meets the static filtering
  criteria for this handler.
  organism: The organism to check.
  Returns: True if it does, false if it doesn't."""
  def check_static_filters(self, organism):
    for attribute in self.__static_filters.keys():
      # Handle nested attributes intelligently.
      lowest_attribute = organism
      for subcategory in attribute.split("."):
        try:
          lowest_attribute = getattr(lowest_attribute, subcategory)
        except AttributeError:
          # This attribute doesn't exist.
          return False

      if lowest_attribute not in self.__static_filters[attribute]:
        # It doesn't meet the criteria.
        return False

    # Only if we get through everything does it meet the criteria.
    return True

  """ Checks the dynamic filters, and if the organism passes, it calls run.
  organism: The organism to run the handler on. """
  def handle_organism(self, organism):
    if self.dynamic_filter(organism):
      self.run(organism)


""" Handler for animals. """
class AnimalHandler(UpdateHandler):
  def __init__(self):
    super().__init__()

    self.filter_attribute("Taxonomy.Kingdom", ["Opisthokonta", "Animalia"])

  def run(self, organism):
    # Update animal position.
    logger.debug("Old position of %d: %s" % \
        (organism.get_index(), organism.get_position()))

    try:
      organism.update_position()
    except OrganismError:
      # Check to see if we have a conflict we can resolve.
      organism.default_conflict_handler()

    logger.debug("New position of %d: %s" % \
        (organism.get_index(), organism.get_position()))

""" Handler for plants. """
class PlantHandler(UpdateHandler):
  def __init__(self):
    super().__init__()

    self.filter_attribute("Taxonomy.Kingdom", "Plantae")

  def run(self, organism):
    # Initialize the metabolism simulator if we haven't already.
    if not organism.metabolism:
      logger.debug("Initializing metabolism simulation for organism %d." %
                   (organism.get_index()))

      # Figure out efficiency.
      if organism.Metabolism.Photosynthesis.Pathway == "C3":
        efficiency = organism.Metabolism.Photosynthesis.C3Efficiency
      elif organism.Metabolism.Photosynthesis.Pathway == "C4":
        efficiency = organism.Metabolism.Photosynthesis.C4Efficiency
      else:
        raise ValueError("Invalid photosynthesis pathway: '%s'" % \
                         (organism.Metabolism.Photosynthesis.Pathway))

      mass = organism.Metabolism.Plant.SeedlingMass

      # Figure out the amount of leaf area.
      try:
        area_mean = organism.Metabolism.Plant.MeanLeafArea
      except AttributeError:
        logger.warning("Using default leaf area mean for plant '%d'." % \
                       (organism.get_index()))
        # Calculate a plausible leaf area based on the scale.
        area_mean = 0.5 * (organism.Scale ** 2)
      try:
        area_stddev = organism.Metabolism.Plant.LeafAreaStddev
      except AttributeError:
        logger.warning("Using default leaf area stddev for plant '%d'." % \
                       (organism.get_index()))
        # Calculate a plausible leaf standard deviation based on the area.
        area_stddev = area_mean * 0.3

      cellulose = organism.Metabolism.Plant.Cellulose
      hemicellulose = organism.Metabolism.Plant.Hemicellulose
      lignin = organism.Metabolism.Plant.Lignin

      args = [mass, efficiency, area_mean, area_stddev, cellulose,
              hemicellulose, lignin]
      logger.debug("Constructing PlantMetabolism with args: %s" % (args))
      organism.metabolism = PlantMetabolism(*args)

    # Request that the plant stays in the same place. (If we don't do this, it
    # won't generate a conflict if something else tries to move here.)
    logger.debug("Plant position: %s" % (str(organism.get_position())))
    organism.set_position(organism.get_position())


# Go and register all the update handlers.
handlers = inspect.getmembers(sys.modules["user_handlers"],
    inspect.isclass)
handlers.extend(inspect.getmembers(sys.modules[__name__], inspect.isclass))
for handler in handlers:
  if (issubclass(handler[1], UpdateHandler) and handler[0] != "UpdateHandler"):
    # We want to filter to only handlers.
    handler[1]()
