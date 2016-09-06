%module automata
%include typemaps.i
%include std_vector.i

%{
#include "../grid.h"
#include "../grid_object.h"
#include "../organism.h"
#include "../metabolism/plant_metabolism.h"
#include "../metabolism/animal_metabolism.h"
using namespace ::automata;
using namespace ::automata::metabolism;
%}

%include metabolism.i

class GridObject {
 public:
  GridObject(Grid *grid, int index);
  bool Initialize(int x, int y);
  void set_index(int index);
  int get_index() const;
  bool SetPosition(int x, int y);
  void get_position(int *OUTPUT, int *OUTPUT) const;
  bool RemoveFromGrid();
  GridObject *GetConflict();
};

namespace std {
  %template(GridObjectVector) vector<GridObject *>;
}

class Organism : public GridObject {
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
  bool DefaultConflictHandler(int max_depth);
  void Die();
  bool IsAlive() const;
  GridObject *GetConflict();
  void CleanupOrganism(const Organism &organism);
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

class PlantMetabolism : public Metabolism {
 public:
  PlantMetabolism(double mass, double efficiency, double area_mean,
                  double area_stddev, double cellulose, double hemicellulose,
                  double lignin);
  ~PlantMetabolism();
  void Update(int time);
  void UseEnergy(double amount);
  double mass() const;
  double energy() const;
};

class AnimalMetabolism : public Metabolism {
 public:
  AnimalMetabolism(double mass, double fat_mass, double body_temp,
                   double scale, double drag_coefficient);
  ~AnimalMetabolism();
  void Update(int time);
  void UseEnergy(double amount);
  double mass() const;
  double energy() const;

  void Consume(Metabolism *metabolism);
  void Move(double distance, int time);

  void UpdatePregnancy(int gestation_cycles, int cycle_time, double birth_mass);
  void Reproduce(double mass);
};
