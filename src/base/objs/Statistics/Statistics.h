#ifndef Statistics_h
#define Statistics_h
/**
 * @file
 * $Date: 2008/05/06 17:19:08 $
 * $Revision: 1.3 $
 *
 *  Unless noted otherwise, the portions of Isis written by the USGS are public domain. See
 *  individual third-party library and package descriptions for intellectual property information,
 *  user agreements, and related information.
 *
 *  Although Isis has been used by the USGS, no warranty, expressed or implied, is made by the
 *  USGS as to the accuracy and functioning of such software and related material nor shall the
 *  fact of distribution constitute any such warranty, and no responsibility is assumed by the
 *  USGS in connection therewith.
 *
 *  For additional information, launch $ISISROOT/doc//documents/Disclaimers/Disclaimers.html
 *  in a browser or see the Privacy &amp; Disclaimers page on the Isis website,
 *  http://isis.astrogeology.usgs.gov, and the USGS privacy and disclaimers on
 *  http://www.usgs.gov/privacy.html.
 */


#include "SpecialPixel.h"
#include "Constants.h"

namespace Isis {
    /**
    * @brief This class is used to accumulate statistics on double arrays.
    *
    * This class is used to accumulate statistics on double arrays. In
    * particular, it is highly useful for obtaining statistics on cube data.
    * Parameters which can be computed are 1) @b average, 2) @b standard
    * @b deviation, 3) @b variance, 4) @b minimum, 5) @b maximum and 6)
    * @b various @b counts of valid and/or special pixels.
    *
    * The following example shows a simple set up and usage of the Statistics
    * class to calculate the average of a set of values:
    *
    * @code
    *   Statistics myStats ;
    *   double myData [] = { 1.0, 3.0, 2.4, 7.5 } ;
    *
    *   myStats.AddData (myData, 4) ;
    *   double myAverage = myStats.Average () ;
    *   cout << "The average of the data is " << myAverage << endl ;
    * @endcode
    *
    * For an example of how the Statistics object is used in %Isis, see the
    * Histogram object (inherits from Statistics) and the stats application,
    * stats.cpp (uses the Statistics child class Histogram).
    *
    * @ingroup Statistics
    *
    * @author Jeff Anderson - 2002-05-06
    *
    * @internal
    * @history 2002-05-08 Jeff Anderson - Added Chebyshev and Best 
    * minimum/maximum methods.
    * @history 2004-05-11 Jeff Anderson - Moved Reset, AddData and RemoveData 
    * methods into public space.
    * @history 2004-06-28 Jeff Anderson - Added Sum and SumSquare methods.
    * @history 2005-02-17 Deborah Lee Soltesz - Modified file to support Doxygen
    * documentation.
    * @history 2005-05-23 Jeff Anderson - Changed to support 2GB+ files
    * @history 2006-02-15 Jacob Danton - Added Valid Range options/methods
    * @history 2006-03-10 Jacob Danton - Added Z-score method
    * @history 2007-01-18 Robert Sucharski - Added AddData method
    *                       for a single double value
    * @history 2008-05-06 Steven Lambright - Added AboveRange, BelowRange methods
    *
    * @todo 2005-02-07 Deborah Lee Soltesz - add example using cube data to the 
    * class documentation
    *
    */
  class Statistics {
    public:
      Statistics ();
      ~Statistics ();

      void Reset ();
      void AddData (const double *data, const unsigned int count);
      void AddData (const double data);
      void RemoveData (const double *data, const unsigned int count);
      void RemoveData (const double data);
      void SetValidRange(const double minimum=Isis::ValidMinimum, const double maximum=Isis::ValidMaximum);

      double ValidMinimum() const {return p_validMinimum;}
      double ValidMaximum() const {return p_validMaximum;}
      bool InRange(const double value) {return (value >= p_validMinimum && value <= p_validMaximum);}
      bool AboveRange(const double value) {return (value > p_validMaximum);}
      bool BelowRange(const double value) {return (value < p_validMinimum);}

      double Average () const;
      double StandardDeviation () const;
      double Variance () const;

      double Minimum () const;
      double Maximum () const;
      double ChebyshevMinimum (const double percent=99.5) const;
      double ChebyshevMaximum (const double percent=99.5) const;
      double BestMinimum (const double percent=99.5) const;
      double BestMaximum (const double percent=99.5) const;
      double ZScore (const double value) const;

      BigInt TotalPixels () const;
      BigInt ValidPixels () const;
      BigInt OverRangePixels () const;
      BigInt UnderRangePixels () const;
      BigInt NullPixels () const;
      BigInt LisPixels () const;
      BigInt LrsPixels () const;
      BigInt HisPixels () const;
      BigInt HrsPixels () const;
      BigInt OutOfRangePixels () const;


      /**
       * Returns the sum of all the data
       *
       * @return The sum of the data
       */
      double Sum() const { return p_sum; };

      /**
       * Returns the sum of all the squared data
       *
       * @return The sum of the squared data
       */
      double SumSquare () const { return p_sumsum; };

    private:
      double p_sum;           //!< Sum accumulator.
      double p_sumsum;        //!< Sum-squared accumulator.
      double p_minimum;       //!< Minimum double value encountered.
      double p_maximum;       //!< Maximum double value encountered.
      double p_validMinimum;  //!< Minimum valid pixel value
      double p_validMaximum;  //!< Maximum valid pixel value
      BigInt p_totalPixels;   //!< Count of total pixels processed.
      BigInt p_validPixels;   //!< Count of valid pixels (non-special) processed.
      BigInt p_nullPixels;    //!< Count of null pixels processed.
      BigInt p_lrsPixels;     //!< Count of low instrument saturation pixels processed.
      BigInt p_lisPixels;     //!< Count of low representation saturation pixels processed.
      BigInt p_hrsPixels;     //!< Count of high instrument saturation pixels processed.
      BigInt p_hisPixels;     //!< Count of high instrument representation pixels processed.
      BigInt p_underRangePixels; //!< Count of pixels less than the valid range
      BigInt p_overRangePixels; //!< Count of pixels greater than the valid range
      bool   p_removedData;   /**< Indicates the RemoveData method was called which implies
                                   p_minimum and p_maximum are invalid. */
  };
} // end namespace isis

#endif

