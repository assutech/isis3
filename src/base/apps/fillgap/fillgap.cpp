#include "Isis.h"

#include <string>

#include "ProcessBySample.h"
#include "ProcessByLine.h"
#include "ProcessBySpectra.h"
#include "DataInterp.h"
#include "SpecialPixel.h"

using namespace std; 
using namespace Isis;

static DataInterp::InterpType iType(DataInterp::Cubic);

void fill(Buffer  &in, Buffer &out);

void IsisMain() {
  UserInterface &ui = Application::GetUserInterface();

  // set spline interpolation to user requested type
  string splineType = ui.GetString("INTERP");
  if (splineType == "LINEAR") {
    iType = DataInterp::Linear; 
  } 
  else if (splineType == "AKIMA") {
    iType = DataInterp::Akima;
  }
  else {
    iType = DataInterp::Cubic;
  }

  //Set null direction to the user defined direction
 string  Dir = ui.GetString("DIRECTION");
  if(Dir == "SAMPLE"){
    ProcessBySample p;
    p.SetInputCube("FROM");
    p.SetOutputCube("TO");  
    p.StartProcess(fill);
    p.EndProcess();
  }
  else if (Dir == "LINE") {
    ProcessByLine p;
    p.SetInputCube("FROM");
    p.SetOutputCube("TO");   
    p.StartProcess(fill);
    p.EndProcess(); 
  } 
  else if (Dir == "BAND") {
    ProcessBySpectra p;
    //Check number of bands to see if this will be allowed.
    p.SetInputCube("FROM");
    p.SetOutputCube("TO");   
    p.StartProcess(fill);
    p.EndProcess(); 
  } 
}

void fill(Buffer &in, Buffer &out) { 
  DataInterp spline(iType);
  
  for (int i=0; i < in.size(); i++) {
    if (!IsSpecial(in[i])) {
      spline.addPoint((double) (i+1),in[i]);
    }
  }
  spline.Compute();
  for (int j = 0 ; j < out.size() ; j++) {
    out[j] = spline.Evaluate((double) (j+1));
  }
  return;
}
