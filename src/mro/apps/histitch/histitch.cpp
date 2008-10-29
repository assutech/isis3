#include <cmath>
#include "Isis.h"
#include "ProcessByLine.h"
#include "SpecialPixel.h"
#include "Pvl.h"

#include "Buffer.h"
#include "Statistics.h"
#include "MultivariateStatistics.h"

using namespace Isis;
using namespace std;

void histitch (vector<Buffer *> &in, vector<Buffer *> &out);
void getStats(std::vector<Isis::Buffer *> &in,
              std::vector<Isis::Buffer *> &out);

Statistics stats0;
Statistics stats1;
MultivariateStatistics stats;

struct ChannelInfo {
  int ChnNumber;
  unsigned nLines;
  unsigned nSamples;
  unsigned offset;
  ChannelInfo() : ChnNumber(1), nLines(0),nSamples(0), offset(0) { }
};

static ChannelInfo fromData[2];
double average0 = Isis::Null;
double average1= Isis::Null;
double coeff;
string balance;
int seamSize;
int skipSize;

void IsisMain() {

//  Get user interface to test for input conditions
  UserInterface &ui = Application::GetUserInterface();
  balance = ui.GetString("BALANCE");
  seamSize = ui.GetInteger("SEAMSIZE");
  skipSize = ui.GetInteger ("SKIP");
  coeff = 1;

  // Define the processing to be by line
  ProcessByLine p;
  
  //Set string to gather ProductIds.
  string stitchedProductIds;

// Obtain lines and samples of the input files.  Note that conditions
// are obtained from the first input file only unless provided from a
// second file
  Cube *icube1 = p.SetInputCube("FROM1");
  fromData[0].nLines   = fromData[1].nLines   = icube1->Lines();
  fromData[0].nSamples = fromData[1].nSamples = icube1->Samples();

  PvlGroup &from1Archive = icube1->GetGroup("ARCHIVE");
  PvlGroup &from1Instrument = icube1->GetGroup("INSTRUMENT");
  fromData[0].ChnNumber = from1Instrument["ChannelNumber"];

  stitchedProductIds = (string)from1Archive["ProductId"][0];

//  Set initial conditions for one input file
  if (fromData[0].ChnNumber == 1) { 
    fromData[0].offset = 0;
  }
  else  {
    if (fromData[0].ChnNumber != 0) {
      string msg = "FROM1 channel number must be 1 or 2";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    fromData[0].offset = fromData[0].nSamples;
  }

//  Only get the second input file if entered by the user
  if (ui.WasEntered("FROM2")) {
    Cube *icube2 = p.SetInputCube("FROM2");   
    fromData[1].nLines   = icube2->Lines();
    fromData[1].nSamples = icube2->Samples();

    //Test to make sure input files are compatable
    PvlGroup &from2Archive = icube2->GetGroup("ARCHIVE");

    //Make sure observation id's are the same
    string from1ObsId = from1Archive["ObservationId"];
    string from2ObsId = from2Archive["ObservationId"];
    if ( from1ObsId != from2ObsId ) {
      string msg = "The input files Observation Id's are not compatable";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }
    stitchedProductIds = "(" + stitchedProductIds + ", " +
                         (string)from2Archive["ProductId"][0] + ")";

    PvlGroup &from2Instrument = icube2->GetGroup("INSTRUMENT");

    //Make sure CCD Id's are the same
    string from1CcdId = from1Instrument["CCDId"];
    string from2CcdId = from2Instrument["CCDId"];
    if ( from1CcdId != from2CcdId ) {
      string msg = "The input files CCD Id's are not compatable";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

    //Make sure channel numbers are equal to 0 & 1
    fromData[1].ChnNumber = from2Instrument["ChannelNumber"];
    if ( !((fromData[0].ChnNumber == 0) && (fromData[1].ChnNumber == 1)) && 
         !((fromData[0].ChnNumber == 1) && (fromData[1].ChnNumber == 0)) ) {
      string msg = "The input files Channel numbers must be equal to 0 and 1";
      throw iException::Message(iException::User,msg,_FILEINFO_);
    }

//  Set up offsets
    if (fromData[0].ChnNumber == 1) {
      fromData[0].offset = 0;
      fromData[1].offset = fromData[0].nSamples;
    }
    else {
      fromData[1].offset = 0;
      fromData[0].offset = fromData[1].nSamples;
    }
  }

  unsigned int LinesOut = max(fromData[0].nLines, fromData[1].nLines);
  unsigned int SampsOut = fromData[0].nSamples + fromData[1].nSamples;
  unsigned int BandsOut = 1;

  Cube *ocube = p.SetOutputCube("TO",SampsOut, LinesOut, BandsOut);

  // Change Channel Number on output cube to 2
  PvlGroup &InstrumentOut = ocube->GetGroup("INSTRUMENT");
  InstrumentOut["ChannelNumber"] = 2;

  // Set StitchedChannels and Stitched ProductIds keywords
  if (ui.WasEntered("FROM2")) {
    InstrumentOut += PvlKeyword("StitchedChannels", "(0,1)");
    InstrumentOut += PvlKeyword("StitchedProductIds", stitchedProductIds);
  }
  else {
    InstrumentOut += PvlKeyword("StitchedChannels", iString(fromData[0].ChnNumber));
    InstrumentOut += PvlKeyword("StitchedProductIds", stitchedProductIds);
  }

  if(balance == "TRUE"){
    ProcessByLine pAvg;
    
    if(ui.WasEntered("FROM2")) {

      if(fromData[0].ChnNumber == 0) {
        pAvg.SetInputCube("FROM1");
        pAvg.SetInputCube("FROM2");
      }

      if(fromData[1].ChnNumber == 0) {
        pAvg.SetInputCube("FROM2");
        pAvg.SetInputCube("FROM1");
      }
      
      stats.Reset();
      pAvg.StartProcess (getStats);
      pAvg.EndProcess ();
        
      average0 = stats.X().Average();
      average1 = stats.Y().Average();
      if(average1 != Isis::Null) {  
        coeff = average0/average1;
      }
    }

  } // end if balance = TRUE

  // Begin processing the input cubes to output cube.
  p.StartProcess(histitch);
  // All Done
  p.EndProcess();
}

void getStats(std::vector<Buffer *> &in, std::vector<Buffer *> &out){
  Buffer &channel0 = *in[0];
  Buffer &channel1 = *in[1];
  double x,y;


    for (int i = (skipSize-1); i < ((skipSize-1)+seamSize+1); i++) {
    
      // set the x value 
      x = channel0[i];
      // set the y value 
      y = channel1[channel1.size()-(skipSize+1)-i] ;
      stats.AddData(&x, &y, 1);
    }
       
}

// Line processing routine
void histitch (vector<Buffer *> &in, vector<Buffer *> &out) {
  Buffer &ot = *out[0];

//  Initialize the buffer
  for (int n = 0 ; n < ot.size() ; n++) ot[n] = NULL8;

//  Place the channel data into the output buffer
  vector<Buffer *>::iterator ibuf;
  int ifrom;
  for (ibuf = in.begin(), ifrom = 0 ; ibuf != in.end() ; ++ibuf, ++ifrom) {
    Buffer &inbuf = *(*ibuf);

    unsigned int oIndex(fromData[ifrom].offset);
    for (int i = 0; i < inbuf.size(); i++, oIndex++) {
      if(balance == "TRUE" && ifrom == 1 && Isis::IsSpecial(inbuf[i]) == false ){  
        ot[oIndex] =  coeff * inbuf[i];
      } else {
        ot[oIndex] = inbuf[i];
      }
    }
  }
  return;
}