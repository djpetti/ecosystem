%module automata
%include typemaps.i
%include std_vector.i

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

namespace std {
  %template(GridObjectVector) vector<GridObject *>;
}

class Organism {
 public:
  Organism(Grid *grid, int index);
  bool Initialize(int x, int y);
  void set_index(int index);
  int get_index() const;
  void set_vision(int vision);
  int get_vision() const;
  void set_speed(int speed);
  int get_speed() const;
  bool SetPosition(int x, int y);
  void get_position(int *OUTPUT, int *OUTPUT) const;
  bool UpdatePosition(int use_x = -1, int use_y = -1);
  void AddFactor(int x, int y, int strength, int visibility = -1);
  void AddFactorFromOrganism(Organism *organism, int strength,
      int visibility = -1);
  bool DefaultConflictHandler();
  void Die();
  bool IsAlive() const;
};

class Grid {
 public:
  Grid(int x_size, int y_size);
  ~Grid();
  void GetConflicted(::std::vector<GridObject *> *OUTPUT,
      ::std::vector<GridObject *> *OUTPUT);
  bool Update();
  double scale() const;
  void set_scale(double scale);
};
