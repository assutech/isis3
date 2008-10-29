#ifndef IssNACamera_h
#define IssNACamera_h

#include "Camera.h"

namespace Cassini { 
  /**                                                                       
   * @brief Cassini ISS Narrow Angle Camera class
   *
   * This is the camera class for the IssNACamera
   *
   * @ingroup Cassini
   *
   * @author  2007-07-10 Steven Koechle
   *
   * @internal
   *   @history 2007-07-10 Steven Koechle - Original Version
   *   @history 2007-07-10 Steven Koechle - Removed hardcoding of
   *            NAIF Instrument number
   *   @history 2007-07-11 Steven Koechle - casted NaifIkCode to
   *            int before iString to fix problem on Linux 32bit
   *   @history 2008-08-08 Steven Lambright Now using the new LoadCache(...)
   *           method instead of CreateCache(...).
   */
  class IssNACamera : public Isis::Camera {
    public:
      IssNACamera (Isis::Pvl &lab);
      ~IssNACamera () {};      
  };
};
#endif
