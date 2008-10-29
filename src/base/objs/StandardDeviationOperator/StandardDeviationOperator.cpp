#include "StandardDeviationOperator.h"
#include "Chip.h"
#include "Statistics.h"

namespace Isis {
  /**
   * This method returns the amount of interest for the given chip.
   * 
   * @param chip 
   * 
   * @return the amount of interest for this chip
   */
  double StandardDeviationOperator::Interest (Chip &chip) {
    Statistics stats;
    stats.SetValidRange(p_minimumDN, p_maximumDN);
    for (int i=0; i<chip.Samples(); i++) {
        double pixels[chip.Lines()];
        int n=0;
        for (int j=0; j<chip.Lines(); j++) {
          if (!IsSpecial(chip(i+1,j+1))){
            pixels[n] = chip(i+1,j+1);
            n++;
            }
        }
        stats.AddData(pixels, n);
    }

    return stats.StandardDeviation();
  }
}

extern "C" Isis::InterestOperator *StandardDeviationOperatorPlugin (Isis::Pvl &pvl) {
  return new Isis::StandardDeviationOperator(pvl);
}

