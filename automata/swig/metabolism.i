%module automata

%{
#include "../metabolism/metabolism.h"
using namespace ::automata::metabolism;
%}

class Metabolism {
 public:
  Metabolism(double mass);
  virtual ~Metabolism();

  virtual void Update(int time) = 0;
  virtual void UseEnergy(double amount) = 0;

  double mass() const { return mass_; }
  double energy() const { return energy_; }
};
