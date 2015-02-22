from tkinter import *
import logging


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
  x: How many pixels to move in the x direction.
  y: How many pixels to move in the y direction. """
  def __move_all(self, x, y):
    for canvas_object in self.__canvas_objects:
      self.__canvas.move(canvas_object, x, y)
    # Move grid objects.
    for grid_object in self.__grid_objects:
      grid_object.move(x, y)

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
    for grid_object in self.__grid_objects:
      grid_object.update()


""" These represent objects that move around on the grid visualization. """
class GridObjectVisualization:
  """ grid_object: Specifies a grid_object that this visualization is linked to.
  grid: The grid visualization that this object should appear on.
  color: What color the object is on the grid. """
  def __init__(self, grid, grid_object, color):
    self.__object = grid_object
    self.__grid = grid
    self.__color = color
    logger.debug("Making visualization for object %d." % \
        (self.__object.get_index()))

    # The tkinter index of the object.
    self.__index = 0

    # Register with the grid.
    self.__grid.add_grid_object(self)
    # Place the object initially.
    self.update()

  """ Draws the object in a specific location.
  position: Position on the grid to draw the object in, in the form (x, y). """
  def __draw(self, position):
    x, y = self.__grid.get_actual_coordinates(position)
    x_size, y_size = self.__grid.get_square_size()
    logger.debug("%d: Drawing at (%d, %d)." % \
        (self.__object.get_index(), x, y))

    canvas = self.__grid.get_canvas()
    coordinates = (x - x_size / 2.0, y - y_size / 2.0,
        x + x_size / 2.0, y + y_size / 2.0)
    if not self.__index:
      # Create the object for the first time.
      self.__index = canvas.create_oval(*coordinates,
          fill = self.__color, outline = self.__color)
    else:
      canvas.coords(self.__index, *coordinates)

  """ Checks if the object we are linked to has moved and update this object's
  position accordingly. """
  def update(self):
    position = self.__object.get_position()
    self.__draw(position)

  """ Moves the object visualization.
  x: How many pixels to move in the x directions.
  y: How many pixels to move in the y directions. """
  def move(self, x, y):
    canvas = self.__grid.get_canvas()
    canvas.move(self.__index, x, y)

  """ Returns: The position of the object on the canvas. """
  def get_pixel_position(self):
    canvas = self.__grid.get_canvas()

    coordinates = canvas.coords(self.__index)
    x = (coordinates[0] + coordinates[2]) / 2.0
    y = (coordinates[1] + coordinates[3]) / 2.0
    return (x, y)
