class SimulationError(Exception):
  def __init__(self, value):
    self.value = value
  def __str__(self):
    return repr(self.value)


from multiprocessing import Process, Value

import logging
import random
import sys
sys.path.append("swig_modules")

from library import Library
from phased_loop import PhasedLoop
import automata
import visualization


logger = logging.getLogger(__name__)


""" Controls a simulation. """
class Simulation:
  """ x_size: The horizontal size of this simulation's grid. """
  """ y_size: The vertical size of this simulation's grid. """
  def __init__(self, x_size, y_size):
    self.__x_size = x_size
    self.__y_size = y_size

    # A list of organisms to get loaded as soon as we fork.
    self.__to_load = []

    # Generate random sets of non-repeating numbers that we will use for placing
    # grid objects.
    self.__random_x = list(range(0, x_size))
    self.__random_y = list(range(0, y_size))
    random.shuffle(self.__random_x)
    random.shuffle(self.__random_y)

    # The separate process that will be used to run the simulation.
    self.simulation_process = Process(target = self.__run_simulation_process)
    # The current iteration of the simulation.
    self.__iteration = Value("i", 0)

  """ Do necessary initialization, then run forever. """
  def __run_simulation_process(self):
    # The grid for this simulation.
    self.__grid = automata.Grid(self.__x_size, self.__y_size)
    # The visualization of the grid for this simulation.
    self.__grid_vis = visualization.GridVisualization(
        self.__x_size, self.__y_size)

    # The list of objects on the grid.
    self.__grid_objects = []

    # The frequency for updating the graphics.
    graphics_limiter = PhasedLoop(30)
    # The frequency for updating the simulation.
    # TODO(danielp): Make this rate user-settable.
    simulation_limiter = PhasedLoop(1)

    # Load all the organisms that we needed to load.
    for organism in self.__to_load:
      library_name = organism[0]
      name = organism[1]
      x_pos = organism[2]
      y_pos = organism[3]

      library = Library(library_name)
      organism = library.load_organism(name, self.__grid,
          len(self.__grid_objects), (x_pos, y_pos))
      logger.info("Adding new grid object with index %d at (%d, %d)." %
          (len(self.__grid_objects), x_pos, y_pos));

      self.__grid_objects.append(organism)

      # Add a visualization for the organism.
      visualization.GridObjectVisualization(self.__grid_vis, organism)

    # Now run the simulation.
    while True:
      PhasedLoop.limit_fastest()

      if simulation_limiter.should_run():
        # Run the simulation.
        self.__run_iteration()
      if graphics_limiter.should_run():
        self.__grid_vis.update()

  """ Completely update the grid a single time. """
  def __run_iteration(self):
    # Update the status of all objects.
    to_delete = []
    for grid_object in self.__grid_objects:
      if not grid_object.update():
        # Organism died. Remove it.
        to_delete.append(grid_object)

    for organism in to_delete:
      self.__grid_objects.remove(organism)

    # Update the grid.
    if not self.__grid.Update():
      logger.log_and_raise("Grid Update() failed unexpectedly.")

    self.__iteration.value += 1
    logger.debug("Running iteration %d." % (self.__iteration.value))

  """ Start the simulation. """
  def start(self):
    # The simulation gets run in a separate process.
    self.simulation_process.start()

  """ Get the current iteration number.
  Returns: The current iteration number. """
  def get_iterations(self):
    return self.__iteration.value

  """ Adds a new organism to the simulation.
  library: The object to add.
  name: The name of the species. """
  def add_organism(self, library, name):
    # Pick a random position for the organism.
    if not len(self.__random_x):
      # We're out of space.
      logger.log_and_raise(SimulationError,
          "Cannot place object, no space on grid.")
    x_pos = self.__random_x.pop()
    y_pos = self.__random_y.pop()

    self.__to_load.append((library, name, x_pos, y_pos))
