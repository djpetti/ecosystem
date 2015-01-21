%module automata
%include typemaps.i

%{
#include "../organism.h"
#include "../grid.h"
using namespace automata;
%}

class Organism {
 public:
  Organism(Grid *grid, int index, int x, int y);
  void SetIndex(int index);
  void SetVision(int vision);
  void SetSpeed(int speed);
  bool SetPosition(int x, int y);
  void GetPosition(int *OUTPUT, int *OUTPUT);
  bool UpdatePosition();
  void AddFactor(int x, int y, int strength, int visibility = -1);
  void AddFactorFromOrganism(Organism *organism, int strength,
      int visibility = -1);
};

class Grid {
 public:
  Grid();
  ~Grid();
  bool Initialize(int x_size, int y_size);
};
