#if !defined(CamTools_h)
#define CamTools_h
/**                                                                       
 * @file                                                                  
 * $Revision: 1.6 $
 * $Date: 2009/06/23 16:51:24 $
 * 
 *   Unless noted otherwise, the portions of Isis written by the USGS are 
 *   public domain. See individual third-party library and package descriptions 
 *   for intellectual property information, user agreements, and related  
 *   information.                                                         
 *                                                                        
 *   Although Isis has been used by the USGS, no warranty, expressed or   
 *   implied, is made by the USGS as to the accuracy and functioning of such 
 *   software and related material nor shall the fact of distribution     
 *   constitute any such warranty, and no responsibility is assumed by the
 *   USGS in connection therewith.                                        
 *                                                                        
 *   For additional information, launch                                   
 *   $ISISROOT/doc//documents/Disclaimers/Disclaimers.html                
 *   in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *   http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *   http://www.usgs.gov/privacy.html.                                    
 */                                                                       

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "Pvl.h"
#include "PvlKeyword.h"
#include "SpecialPixel.h"
#include "ImagePolygon.h"
#include "iException.h"

namespace Isis {

/**
 * @brief Checks value of key, produces appropriate value
 *  
 * This function checks the value of the keyword for specialness 
 * and will create the appropriate keyword if it is special. 
 * 
 * @param keyname Name of keyword to create
 * @param value   Keyword value
 * @param unit    Optional unit qualifer with value
 * 
 * @return PvlKeyword Returns newly created keyword/value 
 */
inline PvlKeyword ValidateKey(const std::string keyname, 
                                     const double &value, 
                                     const std::string &unit = "") { 
  if (IsSpecial(value)) {
    return (PvlKeyword(keyname,"NULL"));
  }
  else {
    return (PvlKeyword(keyname,value,unit));
  }
}

/**
 * @brief Checks proper value of a NULLed keyword
 *  
 * If the keyword is a NULL keyword, ensure it has proper value. 
 * 
 * @param keyname Name of keyword to create
 * @param key     Keyword/value set
 * @param unit    Optional unit qualifer with value
 * 
 * @return PvlKeyword Returns newly created keyword/value 
 */

inline PvlKeyword ValidateKey(const std::string keyname, PvlKeyword &key,
                              const std::string &unit = "") { 
  if (key.IsNull()) {
    return (PvlKeyword(keyname,"NULL"));
  }
  else {
    return (ValidateKey(keyname,(double) key,unit));
  }
}


/** Returns degree to radian conversion factor */
inline double DegToRad(const double ang) { return (ang * rpd_c()); }
/** Returns radians to degrees conversion factor */
inline double RadToDeg(const double ang) { return (ang * dpr_c()); }

/**  A very useful, typesafe way to delete pointers in STL container
 *   Courtesy Scott Meyers, "Effective STL", Item 7, pg 37-40 */
struct DeleteObject {  
  template <typename T> 
    void operator()(const T* ptr) const {
      delete ptr;
    }
};

  /**
   * @brief Collect Band geometry 
   *  
   * This produces the geometry and polygon information from an image cube.  It 
   * has some special processing that accounts for band independant geometrical 
   * image cubes.  It processes each band and creates unique geometry and 
   * ploygon values consider this situation.  The resulting polygon is a union 
   * of all bands, which is a (usually) slightly better footprint of the acutal 
   * footprint for the product. 
   *  
   * The other major advantage to this class is the corner latitude/longitude 
   * points are based upon the extents of each independant band data as the 
   * furthest point from the center of the polygon location (if requested by the 
   * user). 
   *  
   * @ingroup Utility
   * @author 2008-09-10 Kris Becker 
   * @history 2009-02-26 Kris Becker - Removed unconditional computation of 
   *          polygon even when the user did not request it.  Reorganized some
   *          keywords to their relevant group locations.
   * @history 2009-05-29 Kris Becker - Added _pixinx parameter 
   * @history 2009-06-22 Kris Becker - Added hasLimb() method to check for the 
   *          presence of a planet limb; Added getProjGeometry() method.
   */
class BandGeometry {

  public:
    BandGeometry() : _nLines(0), _nSamps(0), _nBands(0), _pixinc(1),
                     _radius(1.0), _isBandIndependent(true), 
                     _hasCenterGeom(false), _gBandList(), _polys(), 
                     _combined(0), _mapping() {  }
    ~BandGeometry() { destruct(); }

    void setPixInc(const int pixinc) { _pixinc = pixinc; }
    int getPixInc() const { return (_pixinc); }
    int size() const { return (_gBandList.size()); }
    bool isPointValid(const double &sample, const double &line, const Camera *camera = 0) const; 
    bool isBandIndependent() const { return (_isBandIndependent); }
    bool hasCenterGeometry() const;
    bool hasLimb() const;
    void collect(Camera &camera, Cube &cube, bool doGeometry, bool doPolygon);
    void generateGeometryKeys(PvlObject &pband);
    void generatePolygonKeys(PvlObject &pband);

  private:
    // Internal structure to contain geometric properties
    struct GProperties {
      GProperties() : lines(0), samples(0), bands(0), 
          band(0),  realBand(0), target(""),
          centerLine(0.0), centerSamp(0.0), 
          centerLatitude(Null), centerLongitude(Null), radius(Null),
          rightAscension(Null), declination(Null),
          centroidLatitude(Null), centroidLongitude(Null), 
          centroidLine(Null), centroidSample(Null), centroidRadius(Null),
          surfaceArea(Null), phase(Null), emi(Null), inc(Null),
          sampRes(Null), lineRes(Null), grRes(Null),
          solarLongitude(Null), northAzimuth(Null), offNader(Null), 
          subSolarAzimuth(Null), subSolarGroundAzimuth(Null), 
          subSpacecraftAzimuth(Null), subSpacecraftGroundAzimuth(Null),
          localSolartime (Null), targetCenterDistance(Null), slantDistance(Null),
          subSolarLatitude(Null), subSolarLongitude(Null),
          subSpacecraftLatitude(Null), subSpacecraftLongitude(Null),
          startTime(""), endTime(""),
          parallaxx(Null), parallaxy(Null),
          shadowx(Null), shadowy(Null), 
          upperLeftLongitude(Null), upperLeftLatitude(Null), 
          lowerLeftLongitude(Null), lowerLeftLatitude(Null), 
          lowerRightLongitude(Null), lowerRightLatitude(Null), 
          upperRightLongitude(Null),upperRightLatitude(Null),
          hasLongitudeBoundary(false), hasNorthPole(false), hasSouthPole(false) { }
      ~GProperties() { }

      int lines, samples, bands;
      int band, realBand;
      std::string target;
      double centerLine, centerSamp;
      double centerLatitude, centerLongitude;
      double radius;
      double rightAscension, declination;
      double centroidLatitude, centroidLongitude;
      double centroidLine, centroidSample;
      double centroidRadius, surfaceArea;
      double phase, emi, inc;
      double sampRes, lineRes, grRes;
      double solarLongitude, northAzimuth, offNader;
      double subSolarAzimuth, subSolarGroundAzimuth;
      double subSpacecraftAzimuth, subSpacecraftGroundAzimuth;
      double localSolartime, targetCenterDistance, slantDistance;
      double subSolarLatitude, subSolarLongitude;
      double subSpacecraftLatitude, subSpacecraftLongitude;
      std::string startTime, endTime;
      double parallaxx, parallaxy, shadowx, shadowy;
      double upperLeftLongitude, upperLeftLatitude;
      double lowerLeftLongitude, lowerLeftLatitude;
      double lowerRightLongitude, lowerRightLatitude;
      double upperRightLongitude, upperRightLatitude;
      bool hasLongitudeBoundary, hasNorthPole, hasSouthPole;
    };

    typedef std::vector<GProperties> BandPropertiesList;   
    typedef BandPropertiesList::iterator BandPropertiesListIter;
    typedef BandPropertiesList::const_iterator BandPropertiesListConstIter;

    // Internal structure to contain geometric properties
    typedef std::vector<geos::geom::Geometry *> BandPolygonList;
    typedef BandPolygonList::iterator BandPolygonListIter;
    typedef BandPolygonList::const_iterator BandPolygonListConstIter;


    int _nLines;
    int _nSamps;
    int _nBands;
    int _pixinc;
    double _radius;
    bool _isBandIndependent;
    bool _hasCenterGeom;
    BandPropertiesList _gBandList;
    BandPolygonList _polys;
    geos::geom::Geometry *_combined;
    GProperties _summary;
    Pvl _mapping;

    void destruct();
    GProperties getGeometrySummary() const;
    Pvl getProjGeometry(Camera &camera,  geos::geom::MultiPolygon *footprint,
                         GProperties &g);
    double getRadius() const;
    double getPixelResolution() const;
    double getPixelsPerDegree(double pixres, double radius) const; 
    bool isDistShorter(double bestDist, double lat1, double lon1, 
                       double lat2, double lon2, double radius, 
                       double &thisDist) const;
    geos::geom::MultiPolygon *makeMultiPolygon(geos::geom::Geometry *g) const;
                                               

};

} // Namespace Isis

#endif
