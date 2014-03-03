#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdlib>

using namespace std;

double X;

#include "individual.hpp"

int main() {
  srand(time(NULL));
  indiv i;
  i.generate(5);
  i.evaluate_print();
  i.calc_size();
  cout << "Size = " << i.get_size() << endl;
}
