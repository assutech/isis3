#include <iostream>
#include <cstdlib>

#include "iException.h"
#include "PolygonSeeder.h"
#include "PolygonSeederFactory.h"
#include "Pvl.h"
#include "PvlGroup.h"
#include "Preference.h"

using namespace Isis;

int main () {
  Isis::Preference::Preferences(true);

  try {
    PvlGroup alg("PolygonSeederAlgorithm");
    alg += PvlKeyword("Name","Grid");
    alg += PvlKeyword("MinimumThickness", 0.5);
    alg += PvlKeyword("MinimumArea", 10);
    alg += PvlKeyword("XSpacing", 11);
    alg += PvlKeyword("YSpacing", 11);

    PvlObject o("AutoSeed");
    o.AddGroup(alg);

    Pvl pvl;
    pvl.AddObject(o);
    std::cout << pvl << std::endl << std::endl;

    PolygonSeeder *ps = PolygonSeederFactory::Create(pvl);

    std::cout << "Test to make sure Parse did it's job" << std::endl;
    std::cout << "MinimumThickness = " << ps->MinimumThickness() << std::endl;
    std::cout << "MinimumArea = " << ps->MinimumArea() << std::endl;

    std::cout << "No reason to test GridPolygonSeeder, so we're done" << std::endl;

  }
  catch (iException &e) {
    e.Report();
  }

  return 0;
}
