#!/usr/bin/python3

import os
import shutil
import unittest

import sys
sys.path.append("swig_modules")

from automata import Grid as C_Grid
import library
import organism

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

if __name__ == "__main__":
  unittest.main()
