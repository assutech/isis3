#ifndef QnetCubePointsFilter_h
#define QnetCubePointsFilter_h

#include <QAction>
#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QRadioButton>
#include <QLineEdit>
#include <QList>
#include "QnetFilter.h"


namespace Qisis {
  /**
   * Defines the Points filter for the QnetNavTool's Cubes
   * section.  The user may choose to enter a maximum or minimum
   * number of points. This class is designed to remove cubes from
   * the current filtered list that contain more than the maximum 
   * or less than the minimum number of points. 
   * 
   * @internal 
   *   @history 2009-01-08 Jeannie Walldren - Modified filter()
   *                          method to create a new filtered list
   *                          from images in the existing filtered
   *                          list.
   *                          
   */
  class QnetCubePointsFilter : public QnetFilter {
    Q_OBJECT

    public:
      QnetCubePointsFilter (QWidget *parent=0);
      virtual void filter();

    private:
     QRadioButton *p_lessThanRB; 
     QRadioButton *p_greaterThanRB;
     QLineEdit *p_pointEdit;     
  };
};

#endif
