#include "NirCamera.h"
#include "CameraDetectorMap.h"
#include "CameraFocalPlaneMap.h"
#include "RadialDistortionMap.h"
#include "CameraGroundMap.h"
#include "CameraSkyMap.h"
#include "iString.h"

using namespace std;
using namespace Isis;
namespace Clementine {
  NirCamera::NirCamera (Pvl &lab) : Camera(lab) {

    // Get the camera characteristics
    iString filter = (string)(lab.FindGroup("BandBin", Pvl::Traverse))["FilterName"];
    filter = filter.UpCase();

    if(filter.compare("A") == 0) {
      SetFocalLength (2548.2642*0.038);
    }
    else if(filter.compare("B") == 0) {
      SetFocalLength (2530.8958*0.038);
    }
    else if(filter.compare("C") == 0) {
      SetFocalLength (2512.6589*0.038);
    }
    else if(filter.compare("D") == 0) {
      SetFocalLength (2509.0536*0.038);
    }
    else if(filter.compare("E") == 0) {
      SetFocalLength (2490.7378*0.038);
    }
    else if(filter.compare("F") == 0) {
      SetFocalLength (2487.8694*0.038);
    }

    SetPixelPitch ();

    // Get the start time in et
    PvlGroup inst = lab.FindGroup ("Instrument",Pvl::Traverse);
    string stime = inst["StartTime"];

    double time; 
    str2et_c(stime.c_str(),&time);

    // Do not correct time for center of the exposure duration. This is because the kernels were built to accept the
    // start times of the images. 
    //time += ((double)inst["ExposureDuration"] / 1000.0) / 2.0; // Add half exposure duration in milliseconds

    // Setup detector map
    new CameraDetectorMap(this);

    // Setup focal plane map
    CameraFocalPlaneMap *focalMap = new CameraFocalPlaneMap(this,NaifIkCode());

    focalMap->SetDetectorOrigin (Spice::GetDouble("INS" + (iString)(int)NaifIkCode() + "_BORESIGHT_SAMPLE"), 
                                 Spice::GetDouble("INS" + (iString)(int)NaifIkCode() + "_BORESIGHT_LINE"));

    // Setup distortion map 
    new RadialDistortionMap(this, -0.0006364);

    // Setup the ground and sky map
    new CameraGroundMap(this);
    new CameraSkyMap(this);

    SetEphemerisTime(time);
    LoadCache();
  }
}

extern "C" Camera *NirCameraPlugin(Pvl &lab) {
  return new Clementine::NirCamera(lab);
}