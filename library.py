import copy
import logging

logger = logging.getLogger(__name__)

import yaml
try:
  from yaml import CLoader as Loader
except ImportError:
  logger.warning("Falling back on Python yaml parser.")
  from yaml import Loader

from organism import Organism


class LibraryError(Exception):
  def __init__(self, value):
    self.__value = value
  def __str__(self):
    return repr(self.__value)


""" Merges two yaml data structures together, using the contents of defaults
to fill in anything not specified in the other data structure.
target: The main tree that we are adding defaults to.
defaults: The default values for anything not specified in the target tree.
Returns: A new version of the target tree, with the defaults incorporated.
"""
def _merge_trees(target, defaults):
  # An edge case if if we have no defaults, in which case, we just return
  # targets.
  if not defaults:
    return target

  # First, flatten both trees.
  target_paths = _flatten_tree(target)
  default_paths = _flatten_tree(defaults)
  merged_paths = copy.deepcopy(target_paths)

  # Add any defaults paths that don't exist in the target.
  for path in default_paths:
    # We don't care about the item at the end for the comparison.
    if path[:len(path) - 1] not in [x[:len(path) - 1] for x in target_paths]:
      # Add the default version.
      merged_paths.append(path)

  return _expand_tree(merged_paths)

""" Takes a yaml tree and determines all the possible paths that one can follow
from the top to the bottom.
tree: The tree to flatten.
Returns: A list of lists. Each list contains keys for each item in the path. """
def _flatten_tree(tree):
  # Make a list of "paths" that we can take down the defaults tree.
  paths = []
  level_stack = []
  current_path = []
  current_level = copy.deepcopy(tree)
  while True:
    while len(current_level) > 0:
      key = list(current_level.keys())[0]
      current_path.append(key)

      item = current_level[key]

      if type(item) is dict:
        # Remove the key so we can't use it again.
        del(current_level[key])

        # We have to go down another level.
        level_stack.append(current_level)
        current_level = item
      else:
        # We're the lowest we can go, so we've finished the path.
        del(current_level[key])

        # Add the item at the end of the path.
        to_add = copy.deepcopy(current_path)
        to_add.append(item)

        paths.append(to_add)
        # Assume the next one will just different end point.
        current_path.pop()

    # If we got to here, we exhausted all the endpoints at this level and have
    # to go up one.
    if len(level_stack) == 0:
      # We got back to the top, so we're done.
      break
    current_level = level_stack.pop()
    current_path.pop()

  return paths

""" Takes a flattened tree and turns it back into a yaml dict.
flat_tree: The tree to unflatten.
Returns: A dict containing the yaml. """
def _expand_tree(flat_tree):
  tree = {}
  branch_stack = []
  current_branch = {}
  for path in flat_tree:
    # Eliminate anything in the path that's already in the tree.
    redundant = 0
    search_level = tree
    for key in path:
      if key in search_level:
        # This is redundant.
        redundant += 1
        # Look for the next key at the next level down.
        search_level = search_level[key]
      else:
        # We have eliminated all the redundant parts.
        break

    # Delete the first redundant items.
    limited_path = path[redundant:]

    # Build up the whole branch, starting from the back.
    partial_branch = limited_path[len(limited_path) - 1]
    for i in reversed(range(1, len(limited_path) - 1)):
      key = limited_path[i]
      partial_branch = {key: partial_branch}

    # Put the partial branch where it goes.
    search_level[limited_path[0]] = partial_branch

  return tree


""" Class designed for importing and managing species from a species library.
"""
class Library:
  """ library_location: Where the library from which we want to import species
  is located. """
  def __init__(self, library_location):
    self.__library = library_location

  """ Loads an organism from the library.
  name: The organism's scientific name.
  grid: The grid to place this organism on.
  position: Where on the grid to place this organism, in the form (x, y).
  Returns: An organism object containing this organism. """
  def load_organism(self, name, grid, position):
    logger.debug("Loading '%s' from '%s'." % (name, self.__library))

    name = name.lower()
    # Add underscore
    name = name.replace(" ", "_")

    organism_file = open("%s/%s.yaml" % (self.__library, name))
    data = yaml.load(organism_file, Loader=Loader)
    organism_file.close()

    # Read defaults and use them to populate anything not specified.
    defaults_file = open("%s/defaults.yaml" % (self.__library))
    defaults = yaml.load(defaults_file, Loader=Loader)
    defaults_file.close()

    # Incorporate the defaults into our original data.
    merged = _merge_trees(data, defaults)

    organism = Organism(grid, position)
    organism.set_attributes(merged)

    if grid.scale() < 0:
      # This is the first organism we added.
      logger.info("Setting grid scale to %f." % (organism.Scale))
      grid.set_scale(organism.Scale)
    elif organism.Scale != grid.scale():
      logger.log_and_raise(LibraryError,
          "Mismatch between object scale %f and grid scale %f." % \
          (organism.Scale, grid.scale()))

    return organism
