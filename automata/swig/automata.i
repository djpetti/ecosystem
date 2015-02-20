%module automata
%include typemaps.i

%{
#include "../grid.h"
#include "../grid_object.h"
#include "../organism.h"
using namespace automata;
%}

class GridObject {
 public:
  GridObject(Grid *grid, int index);
  bool Initialize(int x, int y);
  void set_index(int index);
  int get_index() const;
  bool SetPosition(int x, int y);
  void get_position(int *OUTPUT, int *OUTPUT) const;
};

class Organism {
 public:
  Organism(Grid *grid, int index);
  bool Initialize(int x, int y);
  void set_index(int index);
  int get_index() const;
  void SetVision(int vision);
  void SetSpeed(int speed);
  bool SetPosition(int x, int y);
  void get_position(int *OUTPUT, int *OUTPUT) const;
  bool UpdatePosition();
  void AddFactor(int x, int y, int strength, int visibility = -1);
  void AddFactorFromOrganism(Organism *organism, int strength,
      int visibility = -1);
  bool DefaultConflictHandler(Organism *organism1, Organism *organism2);
};

class Grid {
 public:
  Grid(int x_size, int y_size);
  ~Grid();
  bool Update();
};
