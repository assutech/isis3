#define GUIHELPERS

#include "Isis.h"
#include "ControlNet.h"
#include "ID.h"

using namespace std;
using namespace Isis;

void PrintMap ();

map <string,void*> GuiHelpers(){
  map <string,void*> helper;
  helper ["PrintMap"] = (void*) PrintMap;
  return helper;
}

void IsisMain() {

  // Get the map projection file provided by the user
  UserInterface &ui = Application::GetUserInterface();
  Pvl userMap;
  userMap.Read(ui.GetFilename("PROJECTION"));
  PvlGroup &mapGroup = userMap.FindGroup("Mapping",Pvl::Traverse);

  // Construct a Projection for converting between Lon/Lat and X/Y
  // Note: Should this be an option to include this in the program?
  string target = mapGroup.FindKeyword("TargetName")[0];
  PvlGroup radii = Projection::TargetRadii(target);

  mapGroup.AddKeyword(PvlKeyword("EquatorialRadius", (string) radii["EquatorialRadius"]));
  mapGroup.AddKeyword(PvlKeyword("PolarRadius", (string) radii["PolarRadius"]));
  mapGroup.AddKeyword(PvlKeyword("MinimumLatitude", ui.GetDouble("MINLAT")),Pvl::Replace);
  mapGroup.AddKeyword(PvlKeyword("MaximumLatitude", ui.GetDouble("MAXLAT")),Pvl::Replace);
  mapGroup.AddKeyword(PvlKeyword("MinimumLongitude", ui.GetDouble("MINLON")),Pvl::Replace);
  mapGroup.AddKeyword(PvlKeyword("MaximumLongitude", ui.GetDouble("MAXLON")),Pvl::Replace);

  Projection *proj = Isis::ProjectionFactory::Create(userMap);
                       
  //Convert the Lat/Lon range to an X/Y range.
  double minX;
  double minY;
  double maxX;
  double maxY;
  bool foundRange = proj->XYRange(minX, maxX, minY, maxY);
  if (!foundRange) {
    string msg = "Cannot convert Lat/Long range to an X/Y range.";
    throw Isis::iException::Message(Isis::iException::User,msg,_FILEINFO_);
  }

  double xStepSize = ui.GetDouble("XSTEP");
  double yStepSize = ui.GetDouble("YSTEP");

  // Create the control net to store the points in.
  ControlNet cnet;
  cnet.SetType (ControlNet::ImageToGround);
  cnet.SetTarget (target);
  string networkId = ui.GetString("NETWORKID");
  cnet.SetNetworkId(networkId);
  cnet.SetUserName(Isis::Application::UserName());
  cnet.SetDescription(ui.GetString("DESCRIPTION"));

  // Set up an automatic id generator for the point ids
  ID pointId = ID(ui.GetString("POINTID"));

  double equatorialRadius = radii["EquatorialRadius"];

  double x = minX;
  double y = minY;
  while(x < maxX) {
    while(y < maxY) {
      proj->SetCoordinate(x,y);
      if (!proj->IsSky()) {
        ControlPoint control;
        control.SetId (pointId.Next());
        control.SetIgnore (true);
        control.SetUniversalGround(proj->Latitude(), proj->Longitude(), equatorialRadius);
        cnet.Add(control);
      }
      y += yStepSize;
    }
    x += xStepSize;
    y = minY;
  }

  PvlGroup results ("Results");
  results += PvlKeyword("EquatorialRadius", equatorialRadius);
  results += PvlKeyword("NumberControlPoints", cnet.Size());
  Application::Log(results);

  cnet.Write(ui.GetFilename("TO"));
}

// Helper function to print out mapfile to session log
void PrintMap() {
  UserInterface &ui = Application::GetUserInterface();

  // Get mapping group from map file
  Pvl userMap;
  userMap.Read(ui.GetFilename("PROJECTION"));
  PvlGroup &userGrp = userMap.FindGroup("Mapping",Pvl::Traverse);

  //Write map file out to the log
  Isis::Application::GuiLog(userGrp);
}