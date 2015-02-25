import time

""" A loop that runs a particular number of iterations per second. """
class PhasedLoop:
  # A list of all the loops we've created.
  loops = []

  """ Comparator function for rate sorting.
  loop: The loop to generate a "key" for. """
  @staticmethod
  def compare_rates(loop):
    return loop.get_rate()

  """ Limits to the loop with the fastest rate. Meant to be called once every
  iteration of a multi-phase loop, and then should_run() should be called on
  each individual phase to determine whether it should run this iteration. """
  @classmethod
  def limit_fastest(cls):
    cls.loops[0].__do_limit()

  """ rate: The target number of iterations per second. """
  def __init__(self, rate):
    self.__ticks = 1.0 / rate
    self.__last_run_time = 0

    PhasedLoop.loops.append(self)
    # Keep the list sorted by rate.
    PhasedLoop.loops.sort(key = PhasedLoop.compare_rates)
    PhasedLoop.loops.reverse()

  def __del__(self):
    # Remove from the global list.
    PhasedLoop.loops.remove(self)

  """ Sets the target number of iterations per second for this loop.
  rate: Iterations per second. """
  def set_rate(self, rate):
    self.__ticks = 1.0 / rate

  """ Returns: The current rate of the loop. """
  def get_rate(self):
    return 1.0 / self.__ticks

  """ Should be called every iteration of a loop. It checks that the proper
  amount of time has passed and delays execution until it has.
  rate: The target number of iterations per second. """
  def limit(self):
    new_time = self.__do_limit()
    self.__last_run_time = new_time

  """ Only does the waiting, does not update __last_run_time.
  Returns: The time it got when it started. """
  def __do_limit(self):
    new_time = time.time()
    elapsed = new_time - self.__last_run_time
    sleep_for = max(0, self.__ticks - elapsed)
    time.sleep(sleep_for)
    return new_time

  """ Used for running multiple things on different phases in the same loop.
  This function returns true if this particular loop should be run on this
  particular cycle. """
  def should_run(self):
    # Check if we should run this cycle for this particular loop.
    new_time = time.time()
    elapsed = new_time - self.__last_run_time

    if elapsed >= self.__ticks:
      # We've waited enough time. Run it.
      self.__last_run_time = new_time
      return True
    return False
