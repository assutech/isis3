#include "Isis.h"
#include "ProcessByLine.h"
#include "LineManager.h"
#include "ProjectionFactory.h"
#include "Projection.h"
#include "iException.h"
#include "Application.h"
#include "SpecialPixel.h"

using namespace std; 
using namespace Isis;

void trim(Buffer &in,Buffer &out);
void getSize(Buffer &in);

double slat, elat, slon, elon;
int smallestLine, biggestLine, smallestSample, biggestSample;
Projection *proj;

void IsisMain() {
  // Get the projection
  UserInterface &ui = Application::GetUserInterface ();
  Pvl pvl(ui.GetFilename("FROM"));
  proj = ProjectionFactory::CreateFromCube(pvl);
 
  // Determine ground range to crop and/or trim
  if (ui.WasEntered ("SLAT")) {
    slat = ui.GetDouble ("SLAT");
    elat = ui.GetDouble ("ELAT");
    slon = ui.GetDouble ("SLON");
    elon = ui.GetDouble ("ELON");
  }
  else if (proj->HasGroundRange()) {
    slat = proj->MinimumLatitude();
    elat = proj->MaximumLatitude();
    slon = proj->MinimumLongitude();
    elon = proj->MaximumLongitude();
  }
  else {
	  string msg = "Latitude and longitude range not defined in projection";
		throw iException::Message(iException::User,msg,_FILEINFO_);
	}

  // Crop image if necessary
  string mode = ui.GetString("MODE");
  if(mode != "TRIM") {
    smallestLine = smallestSample = INT_MAX; 
    biggestLine = biggestSample = -INT_MAX; 

    ProcessByLine p;
    p.SetInputCube("FROM");
    p.StartProcess(getSize);
    p.EndProcess();
    
    int samples = biggestSample - smallestSample + 1;
    int lines = biggestLine - smallestLine + 1;

    // Run external crop
    string cropParams = "";
    cropParams += "from= " + ui.GetFilename("FROM");
    if(mode == "CROP") {
      cropParams += " to= " + ui.GetAsString("TO") + " ";
    }
    else {
      cropParams += " to= TEMPORARYcropped.cub ";
    }
    
    cropParams += " sample= "   + iString(smallestSample);
    cropParams += " nsamples= " + iString(samples);
    cropParams += " line= "     + iString(smallestLine);
    cropParams += " nlines= "   + iString(lines);
    
    try {
      Isis::iApp->Exec("crop", cropParams);
    }
    catch(iException &e) {
      string msg = "Could not execute crop with params: [" + cropParams + "]";
      throw Isis::iException::Message(Isis::iException::Programmer, msg,
                                      _FILEINFO_);
    }
    if(mode == "BOTH") {
      delete proj;
      Pvl pvl("TEMPORARYcropped.cub");
      proj = ProjectionFactory::CreateFromCube(pvl);
    }
  }

  // Trim image if necessasry
  if(mode != "CROP") {
    ProcessByLine p;
    CubeAttributeInput att;
    if(mode == "BOTH") {
      p.SetInputCube("TEMPORARYcropped.cub", att); 
    }
    else {
      p.SetInputCube("FROM");
    }
    p.SetOutputCube("TO"); 
    p.StartProcess(trim);
    p.EndProcess();
    if(mode == "BOTH") remove("TEMPORARYcropped.cub");
  }
  delete proj;
}

// Line processing routine
void trim(Buffer &in, Buffer &out) {
  // Loop for each pixel in the line.  Find lat/lon of pixel, if outside
  // of range, set to NULL.
  double lat,lon;
  for (int i = 0; i < in.size(); i++) {
    proj->SetWorld ((double)in.Sample(i),(double)in.Line(i));
    lat = proj->Latitude();
    lon = proj->Longitude();
    if (lat < slat || lat > elat || lon < slon || lon > elon) {
      out[i] = NULL8;
    }
    else {
      out[i] = in[i];
    }
  }
}

// Size up the cropped area in terms of lines and samples
void getSize(Buffer &in) {
  double lat, lon;
  for (int i = 0; i < in.size(); i++) {
    proj->SetWorld ((double)in.Sample(i),(double)in.Line(i));
    lat = proj->Latitude();
    lon = proj->Longitude();
    
    // Skip past pixels outside of lat/lon range
    if (lat < slat || lat > elat || lon < slon || lon > elon) {
      continue; 
    }
    else {
      if(in.Line(i) < smallestLine) {
        smallestLine = in.Line(i);
      }
      if(in.Line(i) > biggestLine) {
        biggestLine = in.Line(i);
      }
      if(in.Sample(i) < smallestSample) {
        smallestSample = in.Sample(i);
      }
      if(in.Sample(i) > biggestSample) {
        biggestSample = in.Sample(i);
      }
    }
  }
}
