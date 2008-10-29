#include "Isis.h"

#include <string>

#include "Cube.h"
#include "Process.h"
#include "Histogram.h"
#include "Pvl.h"

using namespace std; 
using namespace Isis;

void IsisMain() {
  UserInterface &ui = Application::GetUserInterface();
  Process p;

  // Get the histogram
  Cube *icube = p.SetInputCube("FROM");

  double validMin = Isis::ValidMinimum;
  double validMax = Isis::ValidMaximum;

  if(ui.WasEntered("VALIDMIN")) {
    validMin = ui.GetDouble("VALIDMIN");
  }

  if(ui.WasEntered("VALIDMAX")) {
    validMax = ui.GetDouble("VALIDMAX");
  }

  Histogram *stats = icube->Histogram(1, validMin, validMax);

  // Construct a label with the results
  PvlGroup results("Results");
  results += PvlKeyword ("From", icube->Filename());
  if (stats->ValidPixels() != 0) {
    results += PvlKeyword("Average",stats->Average());
    results += PvlKeyword ("StandardDeviation", stats->StandardDeviation());
    results += PvlKeyword ("Variance", stats->Variance());
    // These statistics only worked on a histogram
    results += PvlKeyword ("Median", stats->Median());
    results += PvlKeyword ("Mode", stats->Mode());
    results += PvlKeyword ("Skew", stats->Skew());
    results += PvlKeyword ("Minimum", stats->Minimum());
    results += PvlKeyword ("Maximum", stats->Maximum());
    results += PvlKeyword ("Sum", stats->Sum());
  }
  results += PvlKeyword ("TotalPixels", stats->TotalPixels());
  results += PvlKeyword ("ValidPixels", stats->ValidPixels());
  results += PvlKeyword ("OverValidMaximumPixels", stats->OverRangePixels());
  results += PvlKeyword ("UnderValidMinimumPixels", stats->UnderRangePixels());
  results += PvlKeyword ("NullPixels", stats->NullPixels());
  results += PvlKeyword ("LisPixels", stats->LisPixels());
  results += PvlKeyword ("LrsPixels", stats->LrsPixels());
  results += PvlKeyword ("HisPixels", stats->HisPixels());
  results += PvlKeyword ("HrsPixels", stats->HrsPixels());

  // Write the results to the output file if the user specified one
  if (ui.WasEntered("TO")) {
    Pvl temp;
    temp.AddGroup(results);
    temp.Write(ui.GetFilename("TO"));
  }

  delete stats;

  // Write the results to the log
  Application::Log(results);
}