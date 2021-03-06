#include <QGridLayout>
#include <QMessageBox>
#include "QnetPointErrorFilter.h"
#include "QnetNavTool.h"
#include "ControlNet.h"
#include "SerialNumberList.h"

#include "qnet.h"

using namespace Qisis::Qnet;

namespace Qisis {
   /**
   * Contructor for the Point Error filter.  It
   * creates the Error filter window found in the
   * navtool
   * 
   * @param parent The parent widget for the point
   *               error filter
   *  
   * @internal
   * @history  2008-08-06 Tracie Sucharski - Added functionality of filtering 
   *                         range of errors. 
   */
  QnetPointErrorFilter::QnetPointErrorFilter (QWidget *parent) : QnetFilter(parent) {
    // Create the components for the filter window
    QLabel *label = new QLabel("Filter bundle-adjustment error");
    p_lessThanCB = new QCheckBox("Less than (undercontrolled)");
    p_lessErrorEdit = new QLineEdit();
    p_greaterThanCB = new QCheckBox("Greater than (overcontrolled)");
    p_greaterErrorEdit = new QLineEdit();
    QLabel *pixels = new QLabel ("pixels");
    QLabel *pad = new QLabel();
    p_greaterThanCB->setChecked(true);

    connect(p_lessThanCB,SIGNAL(clicked()),this,SLOT(clearEdit()));
    connect(p_greaterThanCB,SIGNAL(clicked()),this,SLOT(clearEdit()));

    // Create the layout and add the components to it
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(label,0,0,1,2);
    gridLayout->addWidget(p_lessThanCB,1,0,1,2);
    gridLayout->addWidget(p_lessErrorEdit,2,0);
    gridLayout->addWidget(pixels,2,1);
    gridLayout->addWidget(p_greaterThanCB,3,0,1,2);
    gridLayout->addWidget(p_greaterErrorEdit,4,0);
    gridLayout->addWidget(pixels,4,1);
    gridLayout->addWidget(pad,5,0);
    gridLayout->setRowStretch(5,50);
    this->setLayout(gridLayout);
  }

  /**
   * Filters a list of points for points that have less than or greater
   * than the entered bundle adjust error values.  The filtered list will
   * appear in the navtools point list display.
   * 
   * @internal
   *   @history  2007-06-05 Tracie Sucharski - Look at ControlPoint::MaximumError
   *                           instead of ControlPoint::AverageError
   *   @history  2008-08-06 Tracie Sucharski - Added functionality of filtering 
   *                           range of errors.
   *   @history  2009-01-08 Jeannie Walldren - Modified to remove
   *                           new filter points from the existing
   *                           filtered list. Previously, a new
   *                           filtered list was created from the
   *                           entire control net each time.
   */
  void QnetPointErrorFilter::filter() {
    // Make sure we have a list of control points to filter
    if (g_controlNetwork == NULL) {
      QMessageBox::information((QWidget *)parent(),
                               "Error","No points to filter");
      return;
    }

    // Make sure the user entered a value to use in the filtering
    double lessNum = -1.;
    if (p_lessThanCB->isChecked() && p_lessErrorEdit->text() == "") {
      QMessageBox::information((QWidget *)parent(),
                               "Error","Error value must be entered");
      return;
    }
    double greaterNum = -1.;
    if (p_greaterThanCB->isChecked() && p_greaterErrorEdit->text() == "") {
      QMessageBox::information((QWidget *)parent(),
                               "Error","Error value must be entered");
      return;
    }

    // Get the user entered filtering value
    lessNum = p_lessErrorEdit->text().toDouble();
    greaterNum = p_greaterErrorEdit->text().toDouble();

    // Loop through each value of the filtered points list comparing the error of its 
    // corresponding point with error with the user entered value and remove it from
    // the filtered list if it is outside the filtering range 
    // Loop in reverse order since removal list of elements affects index number
    for (int i = g_filteredPoints.size()-1; i >= 0; i--) {
      Isis::ControlPoint cp = (*g_controlNetwork)[g_filteredPoints[i]];
      if (p_lessThanCB->isChecked() && p_greaterThanCB->isChecked()) {
        if ( (cp.MaximumError() < lessNum) &&
             (cp.MaximumError() > greaterNum) ) {
          continue;
        }
        else g_filteredPoints.removeAt(i);
      }
      else if (p_lessThanCB->isChecked()) {
        if (cp.MaximumError() < lessNum) {
          continue;
        }
        else g_filteredPoints.removeAt(i);
      }
      else if (p_greaterThanCB->isChecked()) {
        if (cp.MaximumError() > greaterNum) {
          continue;
        }
        else g_filteredPoints.removeAt(i);
      }
    }
    // Tell the navtool that a list has been filtered and it needs to update
    emit filteredListModified();
    return;
  }


  /**
   * Clear line edit if checkBox "unchecked"
   * 
   * @internal
   * @history  2008-08-06 Tracie Sucharski - New method for added functionality 
   *                         filtering range of errors.
   */
  void QnetPointErrorFilter::clearEdit() {

    if (!p_lessThanCB->isChecked()) p_lessErrorEdit->clear();
    if (!p_greaterThanCB->isChecked()) p_greaterErrorEdit->clear();
  }
}
