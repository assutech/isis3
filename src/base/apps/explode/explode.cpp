#include "Isis.h"
#include "ProcessByLine.h"
#include "iException.h"
#include "Filename.h"

using namespace std; 
using namespace Isis;

void CopyBand (Buffer &in, Buffer &out);

void IsisMain() {
  // Get the cube to explode
  Process p;
  Cube *icube = p.SetInputCube("FROM");
  int samps = icube->Samples();
  int lines = icube->Lines();
  int bands = icube->Bands();
  string infile = icube->Filename();
  
  // We the output filename so we can add attributes and extensions
  UserInterface &ui = Application::GetUserInterface();
  string outbase = ui.GetFilename("TO");
  CubeAttributeOutput &outatt = ui.GetOutputAttribute("TO");

  // Loop and extract each band
  for (int band=1; band<=bands; band++) {
    int pband = icube->PhysicalBand(band);
    iString sband(pband);

    ProcessByLine p2;
    Progress *prog = p2.Progress();
    prog->SetText("Exploding band " + sband); 
    
    CubeAttributeInput inatt("+" + sband);
    p2.SetInputCube(infile,inatt);
    
    string outfile = outbase + ".band";
    if (pband / 1000 == 0) {
      outfile += "0";
      if (pband / 100 == 0) {
        outfile += "0";
        if (pband / 10 == 0) {
          outfile += "0";
        }
      }
    }
    outfile += sband + ".cub";
    p2.SetOutputCube(outfile,outatt,samps,lines,1);

    p2.StartProcess(CopyBand);
    p2.EndProcess();
  }

  // Cleanup
  p.EndProcess();
}

// Line processing routine
void CopyBand (Buffer &in, Buffer &out) {
  for (int i=0; i<in.size(); i++) {
    out[i] = in[i];
  }
}
