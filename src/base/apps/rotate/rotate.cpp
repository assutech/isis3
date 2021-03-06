#include "Isis.h"
#include "ProcessRubberSheet.h"

#include "rotate.h"

using namespace std; 
using namespace Isis;

void IsisMain() {
  ProcessRubberSheet p;

  // Open the input cube
  Cube *icube = p.SetInputCube ("FROM");

  // Set up the transform object
  UserInterface &ui = Application::GetUserInterface();
  Transform *transform = new Rotate(icube->Samples(), icube->Lines(),
                                        ui.GetDouble("DEGREES"));

  // Determine the output size
  int samples = transform->OutputSamples();
  int lines = transform->OutputLines();

  // Allocate the output file
  p.SetOutputCube ("TO", samples, lines, icube->Bands());

  // Set up the interpolator
  Interpolator *interp;
  if (ui.GetString("INTERP") == "NEARESTNEIGHBOR") {
    interp = new Interpolator(Interpolator::NearestNeighborType);
  }
  else if (ui.GetString("INTERP") == "BILINEAR") {
    interp = new Interpolator(Interpolator::BiLinearType);
  }
  else if (ui.GetString("INTERP") == "CUBICCONVOLUTION") {
    interp = new Interpolator(Interpolator::CubicConvolutionType);
  }
  else {
    string msg = "Unknow value for INTERP [" +
                 ui.GetString("INTERP") + "]";
    throw iException::Message(iException::Programmer,msg,_FILEINFO_);
  }

  p.StartProcess(*transform, *interp);
  p.EndProcess();

  delete transform;
  delete interp;
}

