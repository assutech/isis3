#ifndef TableMainWindow_h
#define TableMainWindow_h
#include <QMenu>
#include "Filename.h"
#include "MainWindow.h"


namespace Qisis {
  /**
  * @brief a subclass of the qisis mainwindow, tablemainwindow 
  *        handles all of the table tasks. 
  *
  * @ingroup Visualization Tools
  *
  * @author Stacy Alley
  *
  * @internal 
  *  @history 2008-06-12 Noah Hilt - Changed the save/load
  *            functions to work with blank entries.
  *  @history 2008-06-25 Noah Hilt - Fixed the delete rows method to search the
  *           entire row for values, rather than just the first column.
  */


  class TableMainWindow : public Qisis::MainWindow {
    Q_OBJECT
    public:
      /**
       * Returns the table 
       * 
       * @return QTableWidget* 
       */
      QTableWidget *table() const { return p_table; }; 

      /**
       * 
       * Returns the table window
       * 
       * @return Qisis::MainWindow* 
       */
      Qisis::MainWindow *tableWindow() const { return p_tableWin; }; 

      /**
       * 
       * Returns the item list 
       * 
       * @return QList<QListWidgetItem*> 
       */
      QList<QListWidgetItem*> itemList() const { return p_itemList;}; 

      /**
       * 
       * Returns the list widget 
       * 
       * @return QListWidget* 
       */
      QListWidget *listWidget() const { return p_listWidget; }; 

      /**
       * 
       * Returns the selected rows
       * 
       * @return int 
       */
      int selectedRows() const { return  p_selectedRows; }; 

      /**
       * 
       * Returns the current index
       * 
       * @return int 
       */
      int currentIndex() const { return  p_currentIndex; };

      /**
       * 
       * Returns the current row
       * 
       * @return int 
       */
      int currentRow() const { return  p_currentRow; }; 
      
      TableMainWindow (QString title, QWidget *parent=0);
      void addToTable (bool setOn, const QString &heading,
                       const QString &menuText = "", int insertAt = -1, 
                       Qt::Orientation o = Qt::Horizontal);
      void deleteColumn(int item);
      void setStatusMessage(QString message);

      
    public slots:
      void showTable();
      void syncColumns();
      void syncRows();
      void saveTable ();
      void clearTable ();
      void deleteRows();
      void clearRow(int row);
      void setCurrentRow(int row);
      void setCurrentIndex(int currentIndex);
      void setTrackListItems(bool track = false);
      bool trackListItems();
      void loadTable ();

  signals:
      /**
       * Signal emitted when a file has loaded
       * 
       */
      void fileLoaded();
      
    protected:
      bool eventFilter(QObject *o,QEvent *e);  
      void createTable();      
      void readSettings();
      void readItemSettings(QString heading, QListWidgetItem *item );
      void writeSettings();

    private:
      std::string p_appName; //!< The application name
      QWidget *p_parent; //!< The parent widget
      QString p_title; //!< The title string
      QDockWidget *p_dock; //!< The dock widget

      QTableWidget *p_table; //!< The table
      Qisis::MainWindow *p_tableWin; //!< The main window 
      QList<QListWidgetItem*> p_itemList; //!< List of widget items 
      QListWidget *p_listWidget; //!< List widget
      int p_selectedRows; //!< Number of selected rows
      int p_currentIndex; //!< Current index
      int p_currentRow; //!< Current row
      
      int p_visibleColumns; //!< Number of visible columns
      QList<int>p_startColumn; //!< List of start columns
      QList<int>p_endColumn; //!< List of end columns
      bool p_trackItems; //!< Boolean to track items

      QSettings *p_settings; //!< Settings associated with this object

  };
};

#endif