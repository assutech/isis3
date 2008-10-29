#include <iostream>
#include <iomanip>
#include "LoMediumDistortionMap.h"
#include "CameraFocalPlaneMap.h"

using namespace std;
namespace Isis {
  namespace Lo {
    /** Constructor for LunarOrbiterMediumDistortionMap class
     * 
     * Define the distortion model coefficients for a Lunar Orbiter
     * Medium Resolution camera.
     * 
     * @param parent    A pointer to the parent camera object
     *                  
     * @internal
     * 
     * @history 2007-07-31 Debbie A. Cook Original version
     * 
     */
    LoMediumDistortionMap::LoMediumDistortionMap (Camera *parent) :
      CameraDistortionMap (parent,-1) {
    }


   /** Load LO Medium Resolution Camera perspective & distortion coefficients
    * 
    * This method loads the distortion centers, and coefficients from the 
    * instrument kernel.     * The distortion center coordinates (in mm) are 
    * expected to be in the form of:
    * 
    * @code
    * INSxxxxxxx_DISTORTION_CENTER = ( xcenter, ycenter)
    * 
    * where xxxxxxx is the instrument code (always a negative number).
    * @endcode
    *
    * The center, (x0,y0), will be used to calculate the radius of distortion,
    * r, in the equations below.
    * 
    *    distX = x - x0
    *    distY = y - y0, where (x,y) are the distorted focal plane coordinates.
    *    r^2 = DistX^2 + DistY^2 
    * 
    * The distortion coefficients in the NAIF instrument
    * kernel are expected to be in the form of:
    * 
    * @code
    * INSxxxxxxx_OD_K = ( coef1, coef2, ..., coefN)
    * 
    * where xxxxxxx is as is described above for the distortion center.
    * @endcode
    * 
    * These coefficient will be used to convert from focal plane x,y to 
    * to undistorted x,y as follows:
    * 
    *  dr/r = k0 + k1*r^2 + k2*r^4 
    *  ux = x - DistX*dr/r, similarly for uy
    * 
    * @param naifIkCode    Code to search for in instrument kernel
    * */
    
    void LoMediumDistortionMap::SetDistortion(const int naifIkCode) {
      // Get the distortion center (point of symmetry of distortion)
      p_camera->FocalPlaneMap()->SetFocalPlane(0., 0.);
      double boreS = p_camera->FocalPlaneMap()->DetectorSample();
      double boreL = p_camera->FocalPlaneMap()->DetectorLine();

      std::string centkey = "INS" + Isis::iString(naifIkCode) + "_POINT_OF_SYMMETRY";
      p_sample0 = boreS - p_camera->Spice::GetDouble(centkey, 0);
      p_line0 = boreL + p_camera->Spice::GetDouble(centkey, 1);

      // Get the distortion coefficients
      CameraDistortionMap::SetDistortion (naifIkCode);
    }


   /** Compute undistorted focal plane x/y for Lo Medium Resolution Camera
    * 
    * Compute undistorted focal plane x/y given a distorted focal plane x/y
    * for the Lunar Orbiter Medium Resolution Camera. The polynomial used is
    * described in the SetDistortion documentation. After calling this method, 
    * the  undistorted x/y can be obtained via the UndistortedFocalPlaneX and
    * UndistortedFocalPlaneY methods of the parent class.
    * 
    * @param dx distorted focal plane x in millimeters
    * @param dy distorted focal plane y in millimeters
    * 
    * @return if the conversion was successful
    * @see SetDistortion
    */
    bool LoMediumDistortionMap::SetFocalPlane(const double dx, 
                                                  const double dy) {
      p_focalPlaneX = dx;
      p_focalPlaneY = dy;
      p_camera->FocalPlaneMap()->SetFocalPlane(dx, dy);

      double ds = p_camera->FocalPlaneMap()->DetectorSample();
      double dl = p_camera->FocalPlaneMap()->DetectorLine();
      // Translate the focal plane x/y coordinate to be relative to the
      // distortion point of symmetry
      double dists  =  ds - p_sample0;
      double distl  =  dl - p_line0;

      // Get the distance from the focal plane center and if we are close
      // skip the distortion
      double origr2 = dists * dists + distl * distl;
      double r2 = origr2;
      if (r2 <= 1.0E-6) {
        p_undistortedFocalPlaneX = dx;
        p_undistortedFocalPlaneY = dy;
        return true;
      }

      // Otherwise remove distortion
      double rRef2 = 5000. * 5000.;
      double drOverR  =  (p_odk[0] + r2*(p_odk[1] + r2*p_odk[2]/rRef2)/rRef2)/5000.;
      // Get improved value from estimate
      r2  =  r2 * (1. - drOverR) * (1. - drOverR);
      drOverR  =  (p_odk[0] + r2*(p_odk[1] + r2*p_odk[2]/rRef2)/rRef2)/5000.;
      drOverR  = drOverR*sqrt(r2)/sqrt(origr2);
      double undistortedSample = dists * (1. - drOverR) + p_sample0;
      double undistortedLine = distl * (1. - drOverR) + p_line0;
      p_camera->FocalPlaneMap()->SetDetector(undistortedSample, undistortedLine);
      p_undistortedFocalPlaneX = p_camera->FocalPlaneMap()->FocalPlaneX();
      p_undistortedFocalPlaneY = p_camera->FocalPlaneMap()->FocalPlaneY();
      return true;
    }


   /** Compute distorted focal plane x/y for Lo Medium Resolution Camera
    *
    * Compute distorted focal plane x/y given an undistorted focal plane x/y for
    * the Lunar Orbiter Medium Resolution Camera.  This method applies a
    * distortion error based on a polynomial defined in the SetDistortion
    * method.  After calling this method the distorted x/y can be obtained via 
    * the FocalPlaneX and FocalPlaneY methods.
    * 
    * @param ux undistorted focal plane x in millimeters
    * @param uy undistorted focal plane y in millimeters
    * 
    * @return if the conversion was successful
    * @see SetDistortion
    */
    bool LoMediumDistortionMap::SetUndistortedFocalPlane(const double ux, 
                                                             const double uy) {
      // Adjust for Z direction
      double signFactor = fabs(p_zDirection)/p_zDirection;

      p_undistortedFocalPlaneX = ux*signFactor;
      p_undistortedFocalPlaneY = uy*signFactor;

      p_camera->FocalPlaneMap()->SetFocalPlane(p_undistortedFocalPlaneX, p_undistortedFocalPlaneY);
      double us = p_camera->FocalPlaneMap()->DetectorSample();
      double ul = p_camera->FocalPlaneMap()->DetectorLine();

      // Translate the distorted x/y coordinate to be relative to the
      // distortion point of symmetry
      double distus  =  us - p_sample0;
      double distul  =  ul - p_line0;

      // Compute the distance from the focal plane center and if we are
      // close to the center then no distortion is required
      double rp2 = distus * distus + distul * distul;

      if (rp2 < 1.0E-6) {
        p_focalPlaneX = p_undistortedFocalPlaneX;
        p_focalPlaneY = p_undistortedFocalPlaneY;
        return true;
      }

      // Add distortion.  First compute fractional distortion at rp (r-prime)
      double rRef2 = 5000. * 5000.;
      double drOverR  =  (p_odk[0]  +  rp2*(p_odk[1] + rp2*p_odk[2]/rRef2)/rRef2)/5000.;

      // Compute the focal plane distorted s/l
      double ds  =  p_sample0 + (distus * (1. + drOverR) );  
      double dl  =  p_line0 + (distul * (1. + drOverR) );

      p_camera->FocalPlaneMap()->SetDetector(ds, dl);
      p_focalPlaneX = p_camera->FocalPlaneMap()->FocalPlaneX();
      p_focalPlaneY = p_camera->FocalPlaneMap()->FocalPlaneY();
      return true;
    }
  }

}