#!/usr/bin/python3

import os
import shutil
import unittest

import sys
sys.path.append("swig_modules")

from automata import Grid as C_Grid
import library

# Tests the library class.
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

    self.assertEqual(organism.get_position(), (0, 0))
    self.assertEqual(organism.get_index(), 0)
    self.assertEqual(organism.CommonName, "Test Species")

if __name__ == "__main__":
  unittest.main()
