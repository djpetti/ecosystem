from tkinter import *
import logging
import random

from organism import Organism


logger = logging.getLogger(__name__)


""" Represents a grid visualization. """
class GridVisualization:
  """ x_size: The horizontal size of the grid.
  y_size: The vertical size of the grid. """
  def __init__(self, x_size, y_size):
    self.__x_size = x_size
    self.__y_size = y_size
    logger.info("Making grid visualization for %dx%d grid." % (x_size, y_size))

    # Tkinter indices for all the objects on the canvas this class is
    # responsible for.
    self.__canvas_objects = []
    # All the GridObjectVisualizations on this grid.
    self.__grid_objects = []

    self.__window = Tk()

    # Fill the whole screen.
    self.__width = self.__window.winfo_screenwidth()
    self.__height = self.__window.winfo_screenheight()
    logger.debug("Grid window size: %dx%d." % (self.__width, self.__height))
    self.__canvas = Canvas(width = self.__width, height = self.__height)
    self.__canvas.pack()

    # Save where the center of our view on the grid is.
    self.__window_x = self.__width / 2.0
    self.__window_y = self.__height / 2.0
    logger.debug("Grid window center: (%d, %d)." % \
        (self.__window_x, self.__window_y))

    self.__draw_grid_lines()
    self.__do_key_bindings()

  def __del__(self):
    self.window.destroy()

  """ Draw the lines that make up the grid representation. """
  def __draw_grid_lines(self):
    # Figure out how big we can make the squares.
    self.__square_x_size = self.__width / self.__x_size
    self.__square_y_size = self.__height / self.__y_size
    # Impose a minimum size on the squares.
    self.__square_x_size = max(self.__square_x_size, 25)
    self.__square_y_size = max(self.__square_y_size, 25)
    logger.debug("Grid square size: %dx%d." % \
        (self.__square_x_size, self.__square_y_size))

    # Save the size of the grid.
    self.__grid_x_size = self.__x_size * self.__square_x_size
    self.__grid_y_size = self.__y_size * self.__square_y_size
    logger.debug("Total grid pixel size: %dx%d." % \
        (self.__grid_x_size, self.__grid_y_size))

    # Draw vertical grid lines.
    for i in range(0, self.__x_size):
      x_pos = i * self.__square_x_size

      # Give lines that are multiples of ten more weight.
      color = "gray"
      if not i % 10:
        color = "black"

      index = self.__canvas.create_line(x_pos, 0, x_pos, self.__grid_y_size,
          fill = color)
      self.__canvas_objects.append(index)
    # Draw horizontal grid lines.
    color = "gray"
    for i in range(0, self.__y_size):
      y_pos = i * self.__square_y_size

      color = "gray"
      if not i % 10:
        color = "black"

      index = self.__canvas.create_line(0, y_pos, self.__grid_x_size, y_pos,
          fill = color)
      self.__canvas_objects.append(index)

  """ Sets the key bindings for moving the grid around if it's too big. """
  def __do_key_bindings(self):
    self.__window.bind("<Left>", self.move_left)
    self.__window.bind("<Right>", self.move_right)
    self.__window.bind("<Up>", self.move_up)
    self.__window.bind("<Down>", self.move_down)

  """ Moves all the objects involved in this visualization.
  Note that in order for the grid object visualizations to
  move correctly, someone must call update() on them.
  x: How many pixels to move in the x direction.
  y: How many pixels to move in the y direction. """
  def __move_all(self, x, y):
    for canvas_object in self.__canvas_objects:
      self.__canvas.move(canvas_object, x, y)

    self.__window_x -= x
    self.__window_y -= y
    logger.debug("Moving grid visualization window to (%d, %d)." % \
        (self.__window_x, self.__window_y))

  """ These methods move the view in various directions. """
  def move_left(self, *args):
    # Check that we can still move.
    if self.__window_x - self.__width / 2.0 > 0:
      self.__move_all(self.__square_x_size, 0)

  def move_right(self, *args):
    # Check that we can still move.
    if self.__window_x + self.__width / 2.0 < self.__grid_x_size:
      self.__move_all(-self.__square_x_size, 0)

  def move_up(self, *args):
    # Check that we can still move.
    if self.__window_y - self.__height / 2.0 > 0:
      self.__move_all(0, self.__square_y_size)

  def move_down(self, *args):
    # Check that we can still move.
    if self.__window_y + self.__height / 2.0 < self.__grid_y_size:
      self.__move_all(0, -self.__square_y_size)

  """ Converts coordinates on the grid to coordinates on the canvas.
  position: The position on the grid in the form (x, y).
  Returns: The canvas coordinates in the form (x, y). """
  def get_actual_coordinates(self, position):
    x = position[0] * self.__square_x_size
    y = position[1] * self.__square_y_size

    # We want to be in the middle of the square.
    x += self.__square_x_size / 2.0
    y += self.__square_y_size / 2.0

    # Factor in the simulated window position.
    x -= self.__window_x - self.__width / 2.0
    y -= self.__window_y - self.__height / 2.0

    return (x, y)

  """ Returns: The canvas the grid is drawn on. """
  def get_canvas(self):
    return self.__canvas

  """ Returns: The size of a grid square in the form (x, y). """
  def get_square_size(self):
    return (self.__square_x_size, self.__square_y_size)

  """ Returns: The current position of the simulated window on the grid in the
  form (x, y)."""
  def get_window_position(self):
    return (self.__window_x, self.__window_y)

  """ Adds a new GridObjectVisualization.
  grid_object: The GridObjectVisualization instance to add. """
  def add_grid_object(self, grid_object):
    self.__grid_objects.append(grid_object)

  """ Updates all the GridObjectVisualization's on this grid. """
  def update(self):
    to_delete = []
    for grid_object in self.__grid_objects:
      if not grid_object.update():
        # Organism is dead. Get rid of the visualization.
        to_delete.append(grid_object)
    for organism in to_delete:
      self.__grid_objects.remove(organism)

    self.__window.update()

  """ Returns: All the grid objects in this visualization. """
  def get_grid_objects(self):
    return self.__grid_objects

""" These represent objects that move around on the grid visualization. """
class GridObjectVisualization:
  # A dictionary of the selected colors for each species.
  used_colors = {}

  """ grid_object: Specifies a grid_object that this visualization is linked to.
  grid: The grid visualization that this object should appear on. """
  def __init__(self, grid, grid_object):
    self.__object = grid_object
    self.__grid = grid

    # Use the specified color, or choose a random one.
    try:
      self.__color = grid_object.Visualization.Color
    except AttributeError:
      if grid_object.scientific_name() not in self.used_colors.keys():
        # Pick a new color for the species.
        self.__color = "#%02X%02X%02X" % \
            (random.randint(0, 255), random.randint(0, 255),
             random.randint(0, 255))
        self.used_colors[grid_object.scientific_name()] = self.__color
      else:
        # Use the same color.
        self.__color = self.used_colors[grid_object.scientific_name()]

    logger.debug("Making visualization for object %d." % \
        (self.__object.get_index()))

    # The tkinter index of the object.
    self.__index = 0
    # Tkinter index of the object sex indicator.
    self.__text_index = 0

    # Register with the grid.
    self.__grid.add_grid_object(self)
    # Place the object initially.
    self.update()

  """ Removes canvas object. """
  def __del__(self):
    canvas = self.__grid.get_canvas()
    canvas.delete(self.__index)
    if self.__text_index:
      canvas.delete(self.__text_index)

  """ Draws the object in a specific location.
  position: Position on the grid to draw the object in, in the form (x, y). """
  def __draw(self, position):
    x, y = self.__grid.get_actual_coordinates(position)
    x_size, y_size = self.__grid.get_square_size()

    canvas = self.__grid.get_canvas()
    coordinates = (x - x_size / 2.0, y - y_size / 2.0,
        x + x_size / 2.0, y + y_size / 2.0)
    if not self.__index:
      # Create the object for the first time.
      self.__index = canvas.create_oval(*coordinates,
          fill = self.__color, outline = self.__color)

      if (isinstance(self.__object, Organism) and \
          self.__object.Taxonomy.Kingdom in ["Opisthokonta", "Animalia"]):
        if self.__object.get_sex():
          text = "M"
        else:
          text = "F"
        self.__text_index = canvas.create_text(x, y, text=text)
    else:
      canvas.coords(self.__index, *coordinates)
      if self.__text_index:
        canvas.coords(self.__text_index, x, y)

  """ Checks if the object we are linked to has moved and update this object's
  position accordingly.
  Returns: True if it updates properly, False if object is now dead. """
  def update(self):
    if isinstance(self.__object, Organism):
      if not self.__object.is_alive():
        logger.debug("Removing visualization because organism is dead.")
        return False

    position = self.__object.get_position()
    self.__draw(position)
    return True

  """ Moves the object visualization.
  x: How many pixels to move in the x directions.
  y: How many pixels to move in the y directions. """
  def move(self, x, y):
    canvas = self.__grid.get_canvas()
    canvas.move(self.__index, x, y)
    if self.__text_index:
      canvas.move(self.__text_index, x, y)

  """ Returns: The position of the object on the canvas. """
  def get_pixel_position(self):
    canvas = self.__grid.get_canvas()

    coordinates = canvas.coords(self.__index)
    x = (coordinates[0] + coordinates[2]) / 2.0
    y = (coordinates[1] + coordinates[3]) / 2.0
    return (x, y)

  """ Returns: The underlying grid object that this visualization represents.
  """
  def get_underlying_object(self):
    return self.__object

  """ Returns: The color of the visualization. """
  def get_color(self):
    return self.__color

""" A nifty display pane that shows what species each little dot on the grid
represents. """
class Key:
  # How tall each entry in the key is in pixels.
  _ENTRY_HEIGHT = 50

  """ visualization: The grid visualization that this key corresponds to. """
  def __init__(self, visualization):
    self.__parent = visualization

    # Where the start of our next entry should be.
    self.__entry_position = 0
    # What species we already have in our key.
    self.__known_species = []

    # Create a separate window.
    self.__window = Tk()
    self.__canvas = Canvas(self.__window, width = 200,
                           height = self.__window.winfo_screenwidth())
    self.__canvas.pack()

    for visualization in self.__parent.get_grid_objects():
      grid_object = visualization.get_underlying_object()
      if hasattr(grid_object, "scientific_name"):
        # This is an organism which we want to display.
        if grid_object.scientific_name() not in self.__known_species:
          self.__add_entry(visualization)

          self.__known_species.append(grid_object.scientific_name())

  """ Adds an entry to the table in the key for a new species.
  visualization: A grid object visualization representing a member of this
  species. """
  def __add_entry(self, visualization):
    color = visualization.get_color()
    grid_object = visualization.get_underlying_object()

    # Draw the icon.
    center_y = self.__entry_position + self._ENTRY_HEIGHT / 2
    self.__canvas.create_oval(15, center_y - 10, 35, center_y + 10,
        fill = color, outline = color)

    # Draw the name.
    self.__canvas.create_text(112.5, center_y,
                              text = grid_object.scientific_name())

    # Draw the lower line.
    line_y = self.__entry_position + self._ENTRY_HEIGHT
    self.__canvas.create_line(0, line_y, 200, line_y, fill = "gray")

    self.__entry_position += self._ENTRY_HEIGHT

  """ Updates the underlying tkinter window. """
  def update(self):
    self.__window.update()
