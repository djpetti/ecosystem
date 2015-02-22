#!/usr/bin/python3

import os
import shutil
import unittest

import sys
sys.path.append("swig_modules")

from modified_logger import Logger
# This has to happen before anything we import tries to create a logger.
Logger.set_path("test_log.log")

from automata import Grid as C_Grid
import grid_object
import library
import organism
import visualization


""" Tests the organism class. """
class TestOrganism(unittest.TestCase):
  def setUp(self):
    grid = C_Grid(10, 10)
    self.__organism = organism.Organism(grid, 0, (0, 0))

  """ Does loading and setting attributes work as expected? """
  def test_attributes(self):
    test_attributes = {"Scalar": 42,
                       "Dict": {"item1": 1, "item2": 2},
                       "List": ["Spam", "Eggs"]}

    self.__organism.set_attributes(test_attributes)

    # Try loading them.
    self.assertEqual(self.__organism.Scalar, 42)
    self.assertEqual(self.__organism.Dict.item1, 1)
    self.assertEqual(self.__organism.Dict.item2, 2)
    self.assertEqual(self.__organism.List[0], "Spam")

    # Get entire dicts.
    self.assertEqual(self.__organism.get_all_attributes(), test_attributes)
    self.assertEqual(self.__organism.Dict.get_all_attributes(),
                     test_attributes["Dict"])

    # Get something that doesn't exist.
    with self.assertRaises(organism.OrganismError):
      self.__organism.Nonexistent


""" Tests the library class. """
class TestLibrary(unittest.TestCase):
  def setUp(self):
    self.__library = library.Library("test_library")
    self.__grid = C_Grid(10, 10)

    self.__make_test_library()

  def tearDown(self):
    shutil.rmtree("test_library")

  """ Makes a species library for testing. """
  def __make_test_library(self):
    os.mkdir("test_library")

    species_yaml = \
        "CommonName: Test Species\n" \
        "Taxonomy:\n" \
        "  Domain: TestDomain\n" \
        "  Kingdom: TestKingdom\n" \
        "  Phylum: TestPhylum\n" \
        "  Class: TestClass\n" \
        "  Order: TestOrder\n" \
        "  Family: TestFamily\n" \
        "  Genus:  TestGenus\n" \
        "  Species: TestSpecies\n"

    test_file = open("test_library/test_species.yaml", "w")
    test_file.write(species_yaml)
    test_file.close()

  """ Can we load an organism successfully? """
  def test_load(self):
    # These various notations should work.
    self.__library.load_organism("test species", self.__grid, 0, (0, 0))
    organism = \
        self.__library.load_organism("Test Species", self.__grid, 0, (0, 0))

    # Test that everything came out as we expected it to.
    self.assertEqual(organism.get_position(), (0, 0))
    self.assertEqual(organism.get_index(), 0)
    self.assertEqual(organism.CommonName, "Test Species")
    self.assertEqual(organism.Taxonomy.Domain, "TestDomain")


""" Tests for visualizations. """
class TestVisualizations(unittest.TestCase):
  def setUp(self):
    self.__grid = C_Grid(100, 100)
    self.__grid_object = grid_object.GridObject(self.__grid, 0, 5, 5)

    self.__grid_vis = visualization.GridVisualization(100, 100)
    self.__grid_object_vis = visualization.GridObjectVisualization(
        self.__grid_vis, self.__grid_object, "green")

  """ Can we move the view around correctly? """
  def test_movement(self):
    # Try moving and see that our window position got updated accordingly.
    x_size, y_size = self.__grid_vis.get_square_size()

    x_pos_orig, y_pos_orig = self.__grid_vis.get_window_position()

    self.__grid_vis.move_right()
    x_pos, y_pos = self.__grid_vis.get_window_position()
    self.assertEqual(x_pos, x_pos_orig + x_size)
    self.assertEqual(y_pos, y_pos_orig)

    self.__grid_vis.move_left()
    x_pos, y_pos = self.__grid_vis.get_window_position()
    self.assertEqual(x_pos, x_pos_orig)
    self.assertEqual(y_pos, y_pos_orig)

    self.__grid_vis.move_down()
    x_pos, y_pos = self.__grid_vis.get_window_position()
    self.assertEqual(y_pos, y_pos_orig + y_size)
    self.assertEqual(x_pos, x_pos_orig)

    self.__grid_vis.move_up()
    x_pos, y_pos = self.__grid_vis.get_window_position()
    self.assertEqual(y_pos, y_pos_orig)
    self.assertEqual(x_pos, x_pos_orig)

    # Make sure we can't move out of bounds.
    self.__grid_vis.move_left()
    x_pos, y_pos = self.__grid_vis.get_window_position()
    self.assertEqual(x_pos, x_pos_orig)
    self.assertEqual(y_pos, y_pos_orig)

    self.__grid_vis.move_up()
    x_pos, y_pos = self.__grid_vis.get_window_position()
    self.assertEqual(x_pos, x_pos_orig)
    self.assertEqual(y_pos, y_pos_orig)

  """ Do grid object visualizations work the way we want them to? """
  def test_grid_object_vis(self):
    x_size, y_size = self.__grid_vis.get_square_size()
    x, y = self.__grid_object_vis.get_pixel_position()

    self.assertEqual((x, y), self.__grid_vis.get_actual_coordinates((5, 5)))
    self.assertEqual(x, x_size * 5 + x_size / 2.0)
    self.assertEqual(y, y_size * 5 + y_size / 2.0)

    # Move the linked GridObject and make sure update() catches it.
    self.__grid_object.set_position((10, 10))
    self.__grid_vis.update()
    x, y = self.__grid_object_vis.get_pixel_position()

    self.assertEqual((x, y), self.__grid_vis.get_actual_coordinates((10, 10)))
    self.assertEqual(x, x_size * 10 + x_size / 2.0)
    self.assertEqual(y, y_size * 10 + y_size / 2.0)


if __name__ == "__main__":
  unittest.main()
