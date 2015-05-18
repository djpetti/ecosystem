#!/usr/bin/python3

from modified_logger import Logger
# This has to happen before anything uses a Logger.
Logger.set_path("simulation.log")

import logging
logger = logging.getLogger(__name__)
import select
import sys

import yaml
try:
  from yaml import CLoader as Loader
except ImportError:
  logger.warning("Falling back on Python yaml parser.")
  from yaml import Loader

from library import Library
from simulation import Simulation


def main():
  if len(sys.argv) != 2:
    print("Usage: main.py conf_file")
    sys.exit()

  # Read configuration from file.
  logger.info("Reading configuration from '%s'." % (sys.argv[1]))
  config_file = open(sys.argv[1])
  config = yaml.load(config_file, Loader = Loader)
  config_file.close()

  # Load all the organisms specified.
  if "Organisms" not in config:
    logger.fatal("Invalid config, needs 'Organisms' section.")

  if ("GridXSize" not in config and "GridYSize" not in config):
    logger.fatal("Invalid config, needs GridXSize and GridYSize")
  if "IterationTime" not in config:
    logger.fatal("Invalid config, needs IterationTime.")
  simulation = Simulation(config["GridXSize"], config["GridYSize"],
                          config["IterationTime"])

  # Add them to the simulation.
  for organism in config["Organisms"]:
    for i in range(0, organism["Quantity"]):
      library = Library(organism["Library"])
      simulation.add_organism(organism["Library"], organism["Name"])

  # Start it running.
  logger.info("Delegating to simulation process.")
  simulation.start()

  # Wait forever.
  select.select([], [], [])

  logger.critical("Exiting main.py.")


if __name__ == "__main__":
  main()
