#include "HistogramTool.h"
#include "PlotWindow.h"
#include "PolygonTools.h"

namespace Qisis {

  /**
   * Constructor creates a new HistogramTool object.
   * 
   * @param parent 
   */
  HistogramTool::HistogramTool (QWidget *parent) : Qisis::Tool(parent) {
    p_rubberBand = NULL;
    RubberBandTool::allowPoints(1);
    p_parent = parent;
    createWindow();   
    setupPlotCurves();
    p_scaled = false;
    p_action = new QAction(p_histToolWindow);
    p_action->setText("HistogramTool");
    p_action->setIcon(QPixmap(toolIconDir()+"/histogram.png"));
    QObject::connect(p_action,SIGNAL(activated()),this,SLOT(showPlotWindow()));
    QObject::connect(this,SIGNAL(viewportChanged()),this,SLOT(viewportSelected()));
    p_color = 0;
  }


  /**
   * This protected slot is called when user selects a viewport.
   * 
   */
  void HistogramTool::viewportSelected(){
    p_autoScale->setChecked(true);
  }


  /**
   * This method is called when the tool is activated by the 
   *   parent, or when the plot mode is changed. It's used to
   *   activate or change the rubber banding mode to be either
   *   rectangle or line, depending on the current plot type.
   */
  void HistogramTool::enableRubberBandTool() {
    if(p_rubberBand) {
      p_rubberBand->reset();
      p_rubberBand->setEnabled(true);
    }
  }

  /** 
   * This method adds the histogram tool to the tool pad.
   * 
   * @param toolpad
   * 
   * @return QAction*
   */
  QAction *HistogramTool::toolPadAction(ToolPad *toolpad) {
    QAction *action = new QAction(toolpad);
    action->setIcon(QPixmap(toolIconDir()+"/histogram.png"));
    action->setToolTip("Histogram (H)");
    action->setShortcut(Qt::Key_H);
    QObject::connect(action,SIGNAL(activated()),this,SLOT(showPlotWindow()));

    QString text  =
      "<b>Function:</b>  Plot histogram in active viewport \
      <p><b>Shortcut:</b> H</p> ";
    action->setWhatsThis(text);
    return action;
  }


  /**
   * This method creates the widgets for the tool bar.
   * 
   * 
   * @param parent 
   * 
   * @return QWidget* 
   */
  QWidget *HistogramTool::createToolBarWidget (QStackedWidget *parent) {
    QWidget *hbox = new QWidget(parent);

    p_rubberBand = new RubberBandComboBox(
      RubberBandComboBox::Rectangle |
      RubberBandComboBox::Polygon |
      RubberBandComboBox::Circle |
      RubberBandComboBox::Line,
      RubberBandComboBox::Rectangle
    );
    
    QToolButton *newWindowButton = new QToolButton();
    newWindowButton->setText("New");
    newWindowButton->setToolTip("Opens a new blank plot window");
    QString windowText =
    "<b>Function:</b> This button will bring up a blank plot window that allows \
     the user to copy and paste curves from the active plot window to other windows \
    <p><b>Shortcut:</b>  CTRL+W</p>";
    newWindowButton->setWhatsThis(windowText);
    newWindowButton->setShortcut(Qt::CTRL+Qt::Key_W);
    connect(newWindowButton,SIGNAL(clicked()),this,SLOT(newPlotWindow()));

    QToolButton *plotButton = new QToolButton();
    plotButton->setText("Show");
    plotButton->setToolTip("Shows the active the plot window");
    QString text =
    "<b>Function:</b> This button will bring up the plot window that allows \
     the user to view the min, max, and average values of each band in a  \
     selected range of the image. <p><b>Shortcut:</b>  CTRL+L</p>";
    plotButton->setWhatsThis(text);
    plotButton->setShortcut(Qt::CTRL+Qt::Key_L);
    connect(plotButton,SIGNAL(clicked()),this,SLOT(showPlotWindow()));

    QHBoxLayout *layout = new QHBoxLayout(hbox);
    layout->setMargin(0);
    layout->addWidget(p_rubberBand);
    layout->addWidget(newWindowButton);
    layout->addWidget(plotButton);
    layout->addStretch(1);
    hbox->setLayout(layout);
    return hbox;
  }


  /**
   * This method adds the histogram tool to the menu
   * 
   * 
   * @param menu 
   */
  void HistogramTool::addTo (QMenu *menu) {
    menu->addAction(p_action);
  }


  /**
   * This method updates the histogram tool.
   * 
   */
  void HistogramTool::updateTool() {
    //If there is no viewport, disable the action
    if (cubeViewport() == NULL) {
      p_action->setEnabled(false);
    }
    //Else enable it and set the window's viewport to 
    //the current viewport
    else {
      p_action->setEnabled(true);

      CubeViewport *cvp = cubeViewport();
      p_histToolWindow->setViewport(cvp);
    }
  }


  /**
   * This method creates the default histogram plot window. 
   * 
   */
  void HistogramTool::createWindow() {
    p_histToolWindow = new HistogramToolWindow("Active Plot Window", p_parent);
    p_histToolWindow->setDestroyOnClose(false);
    p_histToolWindow->setDeletable(false);
    p_histToolWindow->setPasteable(false);
    connect(p_histToolWindow, SIGNAL(curveCopied(Qisis::PlotCurve *)), this, 
            SLOT(copyCurve(Qisis::PlotCurve *)));

    QList<QMenu *> menu;
    QList<QAction *> actionButtons;

    /* menu is the QMenu's at the top of the plot window, while 
     actionButtons are the buttons directly below.*/
    p_histToolWindow->getDefaultMenus(menu, actionButtons);

    for(int i = 0; i < menu.size(); i++) {
      if(menu[i]->title() == "&Options") {
        p_autoScale = new QAction(p_histToolWindow);
        p_autoScale->setText("AutoScale");
        p_autoScale->setCheckable(true);
        p_autoScale->setChecked(true);
        QString text  =
        "<b>Function:</b>  Turn on/off the auto scale option on the plot.";
        p_autoScale->setWhatsThis(text);
        menu[i]->addAction(p_autoScale);
        actionButtons.push_back(p_autoScale);    
      }
    }

    p_histToolWindow->setCustomMenu(menu, actionButtons);
    p_histToolWindow->clearPlotCurves(false);
    p_histToolWindow->hideAllSymbols();
    p_histToolWindow->hideAllCurves();
  }


  /**
   * displays the plot window
   * 
   */
  void HistogramTool::showPlotWindow(){
    p_histToolWindow->showWindow();
  }


  /**
   * Called when the user has finished drawing with the rubber
   * band.  ChangePlot is called to plot the data within the
   * rubber band.
   * 
   */
  void HistogramTool::rubberBandComplete() {
    p_histToolWindow->raise();
    if (RubberBandTool::isValid()) {
      changePlot();
    }
    else {
      QMessageBox::information(p_histToolWindow,"Error",
                               "The selected Area contains no valid pixels",
                               QMessageBox::Ok);
    }
  }


  /**
   * This method creates and displays a blank plot window in which
   * users can paste curves to and copy curves from.
   */
  void HistogramTool::newPlotWindow() {
    HistogramToolWindow *blankWindow = new HistogramToolWindow("Plot Window",p_parent);
    blankWindow->setDestroyOnClose(true);
    connect(blankWindow, SIGNAL(curvePaste(Qisis::PlotWindow *)), this, 
            SLOT(pasteCurve(Qisis::HistogramToolWindow *)));
    connect(blankWindow, SIGNAL(curvePasteSpecial(Qisis::PlotWindow *)), this, 
            SLOT(pasteCurveSpecial(Qisis::HistogramToolWindow *)));
    connect(blankWindow, SIGNAL(curveCopied(Qisis::PlotCurve *)), this, 
            SLOT(copyCurve(Qisis::PlotCurve *)));
    connect(blankWindow, SIGNAL(destroyed(QObject *)), this, 
            SLOT(removeWindow(QObject *)));
    connect(blankWindow, SIGNAL(plotChanged()), this, SLOT(updateViewPort()));
    blankWindow->setScale(QwtPlot::xBottom, p_histToolWindow->p_xMin, 
                          p_histToolWindow->p_xMax);
    blankWindow->setScale(QwtPlot::yLeft, p_histToolWindow->p_yMin, 
                          p_histToolWindow->p_yMax);
    blankWindow->setPlotTitle(p_histToolWindow->getPlotTitle().text());
    blankWindow->setDeletable(true);
    blankWindow->setPasteable(true);
    blankWindow->setCopyEnable(false);
    blankWindow->setupDefaultMenu();
    blankWindow->hideAllSymbols();
    blankWindow->hideAllCurves();
    blankWindow->showWindow();

    p_plotWindows.push_back(blankWindow);
  }


  /**
   * This method plots the selected data in a histogram window.
   */
  void HistogramTool::changePlot() {
    CubeViewport *cvp = cubeViewport();
    /* Delete any current curves*/
    p_histToolWindow->clearPlotCurves();

    QList<QPoint> vertices; 

    if(RubberBandTool::getMode() == RubberBandTool::Circle) {
      geos::geom::Geometry *p = RubberBandTool::geometry();
      geos::geom::CoordinateSequence *c = p->getCoordinates();
      for(int i = 0; i < (int)c->getSize(); i++) {
        QPoint point((int)(c->getX(i) + 0.5), (int)(c->getY(i) + 0.5));
        vertices.append(point);
      }
      delete p;
    }
    else {
      vertices = RubberBandTool::getVertices();
    }

    p_histCurve->setViewPort(cvp);
    p_histCurve->setVertices(vertices);
  
    p_cdfCurve->setViewPort(cvp);
    p_cdfCurve->setVertices(vertices);

    if (vertices.size() < 1) return;

    Isis::Cube *cube = cvp->cube();
    int band = cvp->grayBand();
    Isis::Histogram hist(*cube, 1);

    //If the rubber band is a line
    if(RubberBandTool::getMode() == RubberBandTool::Line) {
      double ssamp, sline, esamp, eline;
      cvp->viewportToCube(vertices[0].rx(),vertices[0].ry(),
                         ssamp,sline);

      cvp->viewportToCube(vertices[1].rx(),vertices[1].ry(),
                         esamp,eline);

      QLine line((int)ssamp,(int)sline,(int)esamp,(int)eline);

      double slope;
      int i;
      int x,y,xinc,yinc;
      int xsize,ysize;

      //Get all of the points out of the line
      QList<QPoint *> *linePts = new QList<QPoint *>;
  
      int sx = line.p1().x();
      int ex = line.p2().x();
      int sy = line.p1().y();
      int ey = line.p2().y();
      if (sx > ex) {
        xsize = sx - ex + 1;
        xinc = -1;
      } else {
        xsize = ex - sx + 1;
        xinc = 1;
      }
  
      if (sy > ey) {
        ysize = sy - ey + 1;
        yinc = -1;
      } else {
        ysize = ey - sy + 1;
        yinc = 1;
      }
  
      if (ysize > xsize) {
        slope = (double) (ex - sx) / (double) (ey - sy);   
        y = sy;
        for (i=0; i<ysize; i++) {
          x = (int) (slope * (double) (y - sy) + (double) sx + 0.5);
  
          QPoint *pt = new QPoint;
          pt->setX(x);
          pt->setY(y);
          linePts->push_back(pt);
          y += yinc;
        }
      } else if (xsize == 1) {
        QPoint *pt = new QPoint;
        pt->setX(sx);
        pt->setY(sy);
        linePts->push_back(pt);
      } else {
        slope = (double) (ey - sy) / (double) (ex - sx);   
        x = sx;
        for (i=0; i<xsize; i++) {
          y = (int) (slope * (double) (x - sx) + (double) sy + 0.5);
  
          QPoint *pt = new QPoint;
          pt->setX(x);
          pt->setY(y);
          linePts->push_back(pt);
          x += xinc;
        }
      }

      if (linePts->empty()) {
        QMessageBox::information((QWidget *)parent(),
                                 "Error","No points in edit line");
        return;
      }

      Isis::Brick *brick = new Isis::Brick(*cube,1,1,1);

      //For each point read that value from the cube and add it to the histogram
      for (int i=0; linePts && i<(int)linePts->size(); i++) {
        QPoint *pt = (*linePts)[i];
        int is = pt->x();
        int il = pt->y();
        brick->SetBasePosition(is, il, band);
        cube->Read(*brick);
        hist.AddData(brick->DoubleBuffer(), 1);
      }
      delete brick;

      delete linePts;
 
    }
    //If rubber band is a rectangle
    else if(RubberBandTool::getMode() == RubberBandTool::Rectangle) {
      double ssamp,sline,esamp,eline;
  
      // Convert them to line sample values
      cvp->viewportToCube(vertices[0].x(),vertices[0].y(),ssamp,sline);
      cvp->viewportToCube(vertices[2].x(),vertices[2].y(),esamp,eline);
  
      ssamp = ssamp + 0.5;
      sline = sline + 0.5;
      esamp = esamp + 0.5;
      eline = eline + 0.5;
  
      int nsamps = (int)(esamp - ssamp + 1);
      if (nsamps < 1) nsamps = -nsamps;

      Isis::Brick *brick = new Isis::Brick(*cube,nsamps,1,1);

      //For each line read nsamps and add it to the histogram
      for (int line=(int)std::min(sline,eline); line<=(int)std::max(sline,eline); line++) {
        brick->SetBasePosition((int)ssamp,line,band);
        cube->Read(*brick);
        hist.AddData(brick->DoubleBuffer(),nsamps);
      } 
      delete brick;
    }
    //If rubber band is a polygon or circle
    else {
      geos::geom::Geometry *polygon = RubberBandTool::geometry();

      std::vector <int> x_contained, y_contained;
      if(polygon != NULL) {
        const geos::geom::Envelope *envelope = polygon->getEnvelopeInternal();
        double ssamp, esamp, sline, eline;
        cvp->viewportToCube((int)floor(envelope->getMinX()), (int)floor(envelope->getMinY()), ssamp, sline);
        cvp->viewportToCube((int)ceil(envelope->getMaxX()), (int)ceil(envelope->getMaxY()), esamp, eline);


        for (int y = (int)sline; y <= (int)eline; y++) {
          for (int x = (int)ssamp; x <= (int)esamp; x++) {
            int x1, y1;
            cvp->cubeToViewport(x, y, x1, y1);
            geos::geom::Coordinate c(x1, y1);
            geos::geom::Point *p = Isis::globalFactory.createPoint(c);
            bool contains = p->within(polygon);
            delete p;
  
            if(contains) {
              x_contained.push_back(x);
              y_contained.push_back(y);
            }             
          } 
        }

        delete polygon;

        Isis::Brick *brick = new Isis::Brick(*cube,1,1,1);

        //Read each point from the cube and add it to the histogram
        for (unsigned int j = 0; j < x_contained.size(); j++) {   
          brick->SetBasePosition(x_contained[j], y_contained[j], band);
          cube->Read(*brick);
          hist.AddData(brick->DoubleBuffer(), 1);
        }
        delete brick;
      }
    }


    //Transfer data from histogram to the plotcurve
    std::vector<double> xarray,yarray,y2array;
    double cumpct = 0.0;
    for (int i=0; i<hist.Bins(); i++) {
      if (hist.BinCount(i) > 0) {
        xarray.push_back(hist.BinMiddle(i));
        yarray.push_back(hist.BinCount(i));

        double pct = (double)hist.BinCount(i) / hist.ValidPixels() * 100.;
        cumpct += pct;
        y2array.push_back(cumpct);
      }
    }

    p_histCurve->setData(&xarray[0],&yarray[0],xarray.size());
    p_cdfCurve->setData(&xarray[0],&y2array[0],xarray.size());

    p_histToolWindow->add(p_histCurve);
    p_histToolWindow->add(p_cdfCurve);
    p_histToolWindow->fillTable();

    if(p_autoScale->isChecked()) {
      p_histToolWindow->setScale(QwtPlot::xBottom,p_histCurve->minXValue(),p_histCurve->maxXValue());
      p_histToolWindow->setScale(QwtPlot::yLeft,p_histCurve->minYValue(),p_histCurve->maxYValue());      
    }

    p_histToolWindow->showWindow();
    updateTool();
  }


  /**
   * This method creates a new HistogramToolCurve and copies the properties of the
   * curve the user clicked on into the new curve. The plotWindow class emits a 
   * signal when a curve has been requested to be copied. 
   *  
   * @param pc
   */
  void HistogramTool::copyCurve(Qisis::PlotCurve *pc){
    p_copyCurve = new HistogramToolCurve();
    p_copyCurve->copyCurveProperties((HistogramToolCurve*)pc);
  }


  /**
   * This method pastes the copied curve into the given plot
   * window.  The plotWindow class emits a signal when a paste
   * command has taken place inside the window.
   * @param pw
   */
  void HistogramTool::pasteCurve(Qisis::HistogramToolWindow *pw){
    p_cvp = cubeViewport();
    pw->add(p_copyCurve);
    updateViewPort(p_cvp);

  }


  /**
   * This method does the same as the above method but gives the
   * curve a different color than the copied curve.
   * @param pw
   */
  void HistogramTool::pasteCurveSpecial(Qisis::HistogramToolWindow *pw){
    p_cvp = cubeViewport();
    if (p_color < p_colors.size()) {
      p_copyCurve->setColor(p_colors[p_color]);
    }
    else {
      QColor c = QColorDialog::getColor(Qt::white, p_histToolWindow);
      if (c.isValid()) {
        p_copyCurve->setColor(c);
      }
    }

    pw->add(p_copyCurve);
    updateViewPort(p_cvp);
    p_color++;

  }


  /**
   * This class keeps a list of how many plot windows it has
   * created (in a QList).  When a user closes a window, we want
   * to remove that window from our QList.  The PlotWindow class
   * emits a signal when the window has been destroyed so we can
   * call this slot when that signal has been sent.
   * @param window
   */
  void HistogramTool::removeWindow(QObject *window){
    for (int i = 0; i < p_plotWindows.size(); i++) {

      if (p_plotWindows[i] == window) {
        p_plotWindows.removeAt(i);

      }
    }
    updateViewPort();

  }


  /**
   * This method sets up the names, line style, and color  of the
   * all the HistogramToolCurves that will be used in this class. This method also
   * fills the p_colors QList with the colors that will be used when the user 
   * copies and pastes (special) into another plot window. 
   */
  void HistogramTool::setupPlotCurves() {
    p_histCurve = new HistogramToolCurve();
    p_histCurve->setStyle(QwtPlotCurve::Lines);
    p_histCurve->setTitle("Frequency");

    p_cdfCurve = new HistogramToolCurve();
    p_cdfCurve->setStyle(QwtPlotCurve::Lines);
    p_cdfCurve->setTitle("Percentage");

    QPen *pen = new QPen(Qt::red);
    pen->setWidth(2);
    p_histCurve->setYAxis(QwtPlot::yLeft);
    p_histCurve->setPen(*pen);
    pen->setColor(Qt::blue);
    p_cdfCurve->setYAxis(QwtPlot::yRight);
    p_cdfCurve->setPen(*pen);

    p_colors.push_back(Qt::cyan);
    p_colors.push_back(Qt::magenta);
    p_colors.push_back(Qt::yellow);
    p_colors.push_back(QColor(255,170,255));
    p_colors.push_back(Qt::green);
    p_colors.push_back(Qt::white);
    p_colors.push_back(Qt::blue);
    p_colors.push_back(Qt::red);
    p_colors.push_back(QColor(134,66,176));
    p_colors.push_back(QColor(255,152,0));   

  }

  /** 
   * This method paints the polygons of the copied curves 
   * onto the cubeviewport
   * 
   * @param vp
   * @param painter
   */
  void HistogramTool::paintViewport(CubeViewport *vp, QPainter *painter) {
    int sample1, line1, sample2, line2;

    // loop thru the window list
    for (int i = 0; i < p_plotWindows.size(); i++) {  

      for (int c = 0; c < p_plotWindows[i]->getNumCurves(); c++) {
        /*get all curves in current window*/
        HistogramToolCurve *curve = (HistogramToolCurve *)p_plotWindows[i]->getPlotCurve(c);  

        if (curve->getViewPort() == vp) {
          QPen pen(curve->pen().color());
          pen.setWidth(curve->pen().width());
          pen.setStyle(curve->pen().style());
          painter->setPen(pen);
          QList <QPointF> points = curve->getVertices();
         
          for(int p = 1; p < points.size(); p++) {
            vp->cubeToViewport(points[p-1].x(),points[p-1].y(), sample1,line1);
            vp->cubeToViewport(points[p].x(),points[p].y(), sample2,line2);
            painter->drawLine(QPoint(sample1,line1), QPoint(sample2,  line2));              
          }

          vp->cubeToViewport(points[points.size()-1].x(),
                             points[points.size()-1].y(), sample1,line1);
          vp->cubeToViewport(points[0].x(),points[0].y(), sample2,line2);
          painter->drawLine(QPoint(sample1,line1), QPoint(sample2,  line2));

        }
      }
    }

  }


  /**
   * This method causes the view port corresponding with the given PlotToolCurve 
   * to be repainted with all of the area's of interest associated with the 
   * PlotToolCurve's plotwindow. The paintViewport() method is called. 
   * 
   * @param cvp 
   */
  void HistogramTool::updateViewPort(Qisis::CubeViewport *cvp){
    cvp->repaint();
  }

  /**
   * This overloaded method is called to repaint the current view
   * port.  The paintViewport() method is called.
   */
  void HistogramTool::updateViewPort(){
    p_cvp->viewport()->repaint();

  }
}
