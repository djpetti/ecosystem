#!/usr/bin/python3

import copy
import os
import shutil
import unittest

from modified_logger import Logger
# This has to happen before anything we import tries to create a logger.
Logger.set_path("test_log.log")

from swig_modules.automata import Grid as C_Grid
import grid_object
import library
import organism
import update_handler
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
    with self.assertRaises(AttributeError):
      self.__organism.Nonexistent


""" Tests the library class. """
class TestLibrary(unittest.TestCase):
  # Example yaml that gets used for testing.
  _TEST_YAML = {"key1": 1, "key2": {"key3": 3, "key4": {"key5": 5}},
                "key6": 6}

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
        "  Species: TestSpecies\n" \
        "Scale: 1\n"

    test_file = open("test_library/test_species.yaml", "w")
    test_file.write(species_yaml)
    test_file.close()

    # Empty defaults file.
    defaults_file = open("test_library/defaults.yaml", "w")
    defaults_file.close()

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

  """ Do the flatten_tree and expand_tree functions work properly? """
  def test_flatten_tree(self):
    expected_paths = [["key1", 1], ["key2", "key3", 3],
                      ["key2", "key4", "key5", 5], ["key6", 6]]

    paths = library._flatten_tree(self._TEST_YAML)
    # They could be ordered differently.
    for path in paths:
      self.assertIn(path, expected_paths)

    # Unflatten it again.
    tree = library._expand_tree(paths)
    self.assertEqual(tree, self._TEST_YAML)

  """ Does the merge_trees function work properly? """
  def test_merge_trees(self):
    defaults_yaml = {"key1": 7, "key7": 7, "key2": {"key4": 8, "key8": 8}}
    expected_results = {"key1": 1, "key7": 7,
                        "key2": {"key3": 3, "key4": {"key5": 5}, "key8": 8},
                        "key6": 6}

    merged_tree = library._merge_trees(self._TEST_YAML, defaults_yaml)
    self.assertEqual(expected_results, merged_tree)


""" Tests for visualizations. """
class TestVisualizations(unittest.TestCase):
  def setUp(self):
    test_attributes = {"Visualization": {"Color": "#00FF00"}}

    self.__grid = C_Grid(100, 100)
    self.__grid_object = organism.Organism(self.__grid, 0, (5, 5))
    self.__grid_object.set_attributes(test_attributes)

    self.__grid_vis = visualization.GridVisualization(100, 100)
    self.__grid_object_vis = visualization.GridObjectVisualization(
        self.__grid_vis, self.__grid_object)

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


""" Tests for update handlers. """
class TestUpdateHandler(unittest.TestCase):
  """ An UpdateHandler subclass for testing purposes. """
  class TestingHandler(update_handler.UpdateHandler):
    def __init__(self):
      super().__init__()

      self.filter_attribute("CommonName", "Test Species")
      self.filter_attribute("Taxonomy.Domain",
          ["TestDomain", "TestDomain2"])

    def dynamic_filter(self, organism):
      # Kind of a silly filter, but easy to use for testing purposes.
      if organism.get_index():
        return True
      return False

    def run(self, organism, *args):
      # Kind of a dumb way to detect if the handler ran, but it works.
      raise RuntimeError("Ran handler.")

  def setUp(self):
    test_attributes1 = {"CommonName": "Test Species",
        "Taxonomy": {"Domain": "TestDomain"}}
    test_attributes2 = copy.deepcopy(test_attributes1)
    test_attributes2["Taxonomy"]["Domain"] = "TestDomain2"
    test_attributes3 = copy.deepcopy(test_attributes1)
    test_attributes3["Taxonomy"]["Domain"] = "TestDomain3"

    grid = C_Grid(10, 10)

    # Simply instantiating our handler should register it.
    self.__test_handler = TestUpdateHandler.TestingHandler()

    self.__organism1 = organism.Organism(grid, 0, (0, 0))
    self.__organism2 = organism.Organism(grid, 1, (1, 1))
    self.__organism3 = organism.Organism(grid, 2, (2, 2))
    self.__organism1.set_attributes(test_attributes1)
    self.__organism2.set_attributes(test_attributes2)
    self.__organism3.set_attributes(test_attributes3)

  """ Do static filters filter what we want them to? """
  def test_static_filter(self):
    handlers = self.__organism1.get_handlers()
    self.assertIn(self.__test_handler, handlers)

    handlers = self.__organism2.get_handlers()
    self.assertIn(self.__test_handler, handlers)

    handlers = self.__organism3.get_handlers()
    self.assertNotIn(self.__test_handler, handlers)

    # Organism 3 should not get past the static filter.
    self.__organism3.update(0)

  """ Do dynamic filters work properly? """
  def test_dynamic_filter(self):
    # The dynamic filter should block this one.
    self.__organism1.update(0)

    # The other one should work.
    with self.assertRaises(RuntimeError):
      self.__organism2.update(0)

if __name__ == "__main__":
  unittest.main()
