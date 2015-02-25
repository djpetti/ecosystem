import logging
import sys

""" A logger subclass with some minor changes to make implementation easier. """
class Logger(logging.Logger):
  """ What file we're going to log to. """
  path = ""

  @staticmethod
  def set_path(path):
    Logger.path = path

  """ name: The name of this logger.
  path: Path to the file where we are logging. """
  def __init__(self, name):
    super().__init__(name)

    # I like it configured a certain way, so we might as well do that here.
    self.setLevel(logging.DEBUG)

    # Log important stuff to the console.
    console = logging.StreamHandler()
    console.setLevel(logging.INFO)
    # Log everything to a file.
    logfile = logging.FileHandler(Logger.path)
    logfile.setLevel(logging.DEBUG)

    formatter = logging.Formatter(
        "%(processName)s(%(process)d): %(levelname)s " \
        "at %(msecs)d: %(filename)s: %(lineno)d: %(message)s")
    console.setFormatter(formatter)
    logfile.setFormatter(formatter)

    self.addHandler(console)
    self.addHandler(logfile)

  """ This method logs a message like normal and then raises an exception.
  exception: The type of exception to throw.
  message: The message to log. """
  def log_and_raise(self, exception, message, *args, **kwargs):
    self.error(message, *args, **kwargs)
    raise exception(message, *args, **kwargs)

  """ Logs at the critical level and aborts.
  message: The message to log. """
  def fatal(self, message, *args, **kwargs):
    self.critical(message, *args, **kwargs)
    sys.exit(1)

# Use this as the default logger.
logging.setLoggerClass(Logger)
