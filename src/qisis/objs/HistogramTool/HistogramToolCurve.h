
#ifndef HistogramToolCurve_h
#define HistogramToolCurve_h

#include "PlotCurve.h"
#include "CubeViewport.h"


/**                                                                       
 * @file                                                                  
 * $Revision: 1.1 $                                                             
 * $Date: 2008/08/04 17:58:05 $
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


namespace Qisis {
  class HistogramToolCurve : public PlotCurve{
 
    public:
      HistogramToolCurve();
      void copyCurveProperties(HistogramToolCurve *pc);  
      QList <QPointF > getVertices() const;
      void setVertices(const QList <QPoint> &points);
      CubeViewport* getViewPort() const;
      void setViewPort(CubeViewport *cvp);

  private:
      QList <QPointF> p_pointList;//!< List of data points
      CubeViewport *p_cvp;//!< Viewport the data is from
     
   };
};
#endif
