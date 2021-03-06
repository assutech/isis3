#include "Isis.h"
#include "InterestOperator.h"
#include "InterestOperatorFactory.h"
#include "ProcessByBrick.h"
#include "Pixel.h"
#include "Cube.h"
#include "iException.h"

using namespace std; 
using namespace Isis;

void Operate(Isis::Buffer &in, Isis::Buffer &out);

InterestOperator *iop;
Cube cube;
int boxcarLines, boxcarSamples;

void IsisMain() {
  // We will be processing by line
  ProcessByBrick p;
  p.SetBrickSize(1,1,1);
  p.SetOutputBrickSize(1,1,1);
  UserInterface &ui = Application::GetUserInterface();

  // Basic settings
  p.SetInputCube("FROM");
  p.SetOutputCube("TO");
  Pvl pvl = Pvl(ui.GetFilename("PVL"));

  cube.Open(ui.GetFilename("FROM"));

  try {
    // Get info from the operator group
    // Set the pvlkeywords that need to be set to zero
    PvlGroup &op = pvl.FindGroup("Operator",Pvl::Traverse);
    boxcarSamples = op["Samples"];
    boxcarLines = op["Lines"];
    op["DeltaLine"]=0;
    op["DeltaSamp"]=0;
    op["MinimumInterest"]=0.0;
    Application::Log(op);
  } catch (iException &e) {
    std::string msg = "Improper format for InterestOperator PVL ["+pvl.Filename()+"]";
    throw iException::Message(iException::User,msg,_FILEINFO_);
  }

  iop = InterestOperatorFactory::Create(pvl);

  // Start the processing
  p.StartProcess(Operate);
  p.EndProcess();
}

// Call Operate once per pixel to get the interest for every pixel in the input cube.
void Operate(Isis::Buffer &in, Isis::Buffer &out) {
  try {
    int sample = in.Sample();
    int line = in.Line();

    iop->Operate(cube, sample, line);


    out[0] = iop->InterestAmount();

  } catch (iException &e) {
    e.Report();
  }
}
