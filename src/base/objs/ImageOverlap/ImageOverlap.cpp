#include "PolygonTools.h"
#include "iException.h"
#include "Filename.h"
#include "ImageOverlap.h"

namespace Isis {

  /**                                                                       
   * Construct an empty ImageOverlap object
   * 
   */ 
  ImageOverlap::ImageOverlap () {
    Init();
  }


  /**                                                                       
   * Construct an ImageOverlap object and initialize it with the arguments
   * 
   * @param serialNumber The initial serial number assiciated with the polygon.
   * 
   * @param polygon The polygon that defines the overlap area.
   *
   */ 
  ImageOverlap::ImageOverlap (std::string serialNumber,
                              geos::geom::MultiPolygon &polygon) {
    Init();
    SetPolygon (polygon);
    Add (serialNumber);
  }


  /**                                                                       
   * Destroy this ImageOverlap object
   * 
   */ 
  ImageOverlap::~ImageOverlap() {
    delete p_polygon;
  };


  /**                                                                       
   * Initialize this object to a known state.
   * 
   */ 
  void ImageOverlap::Init () {
    p_serialNumbers.clear();
    p_polygon = NULL;
  }


  /**                                                                       
   * This method will replace the existing polygon that defines the overlap with
   * a new one.
   * 
   * @param polygon The new polygon.
   *
   */ 
  void ImageOverlap::SetPolygon (const geos::geom::MultiPolygon &polygon) {
    if (p_polygon != NULL) {
      delete p_polygon;
      p_polygon = NULL;
    }

    p_polygon = PolygonTools::CopyMultiPolygon(polygon);
  }


  /**                                                                       
   * This method will replace the existing polygon that defines the overlap with
   * a new one.
   * 
   * @param polygon The new polygon.
   *
   */
  void ImageOverlap::SetPolygon (const geos::geom::MultiPolygon *polygon) {
    if (p_polygon != NULL) {
      delete p_polygon;
      p_polygon = NULL;
    }

    p_polygon = PolygonTools::CopyMultiPolygon(polygon);
  }


  /**                                                                       
   * This method will add a new serial number to the list of serial numbers
   * alread associated with the overlap.
   * 
   * @param sn The serial number to be added to the list.
   *
   */ 
  void ImageOverlap::Add (std::string &sn) {
    for (unsigned int s=0; s<p_serialNumbers.size(); ++s) {
      if (sn == p_serialNumbers[s]) {
        std::string msg = "Duplicate SN added to [" +  p_polygon->toString() + "]";
        throw iException::Message(iException::Programmer,msg,_FILEINFO_);
      }
    }

    p_serialNumbers.push_back(sn);
    return;
  }


  /**                                                                       
   * This method will return the area of the polygon. This member does not 
   * assume any particular units of measure for the verticies of the polygon.
   * It is provided as a convience.
   *                                                                        
   */ 
  double ImageOverlap::Area() {
    return p_polygon->getArea();
  }

  /**                                                                       
   * This method will return true if any serial number from this ImageOverlap is
   * also in the other ImageOverlap
   */ 
  bool ImageOverlap::HasAnySameSerialNumber(ImageOverlap &other) {
    for (int thisSn=0; thisSn<this->Size(); ++thisSn) {
      for (int otherSn=0; otherSn<other.Size(); ++otherSn) {
        if (p_serialNumbers[thisSn] == other.p_serialNumbers[otherSn]) {
          return true;
        }
      }
    }
    return false;
  }



  /**                                                                       
   * This method will return true if input serial number exists in the
   * ImageOverlap.
   * 
   * @param[in] sn    (std::string &)  Serial Number to search for
   * 
   * @return bool  Returns true if the serial number exists in the
   *               ImageOverlap.
   */ 
  bool ImageOverlap::HasSerialNumber(std::string &sn) {
    for (int thisSn=0; thisSn<this->Size(); ++thisSn) {
      if (p_serialNumbers[thisSn] == sn) {
        return true;
      }
    }
    return false;
  }


}
