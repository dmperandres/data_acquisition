//LIC

#ifndef WINDOW
#define WINDOW

#include <QWidget>
#include <QSizePolicy>
#include <QSettings>
#include <QFileInfo>
#include <QDateTime>
#include <QMainWindow>
#include <QFrame>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QTimer>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollBar>
#include <QRect>
#include <QScreen>
#include <QComboBox>
#include <QLabel>

#include <QApplication>

#include <string>
#include "vertex.h"
#include "filters_tab.h"


class _gl_widget;

typedef enum{SELECTION_MODE_RUBBER_BOX,SELECTION_MODE_BOX,SELECTION_MODE_CIRCLE} _selection_mode;
typedef enum{SELECTION_STATE_START,SELECTION_STATE_STOP,SELECTION_STATE_NOTHING} _selection_state;
typedef enum{MODE_MOUSE_SELECTION,MODE_MOUSE_DELETE,MODE_MOUSE_NOTHING} _mode_mouse;
typedef enum{MODE_CURSOR_ARROW,MODE_CURSOR_CROSS,MODE_CURSOR_MOVE,MODE_CURSOR_LAST} _mode_cursor;

const int MIN_WIN_WIDTH=1100;
const int MIN_WIN_HEIGHT=900;

namespace _window_ns {
  const float SCREEN_WIDTH_FACTOR=0.5f;
  const float SCREEN_HEIGHT_FACTOR=0.9f;

#ifdef LINUX
  const int WIDTH_TAB_CHARS=20;
#else
  const int WIDTH_TAB_CHARS=30;
#endif
}

//HEA

class _window : public QMainWindow
{
Q_OBJECT

public:

  typedef enum {SAVE_RESOLUTION_300PPI,SAVE_RESOLUTION_1200PPI, LAST_SAVE_RESOLUTION} _save_resolution;

  _window();

  void read_settings();
  void write_settings();

//  void next_step();
//  void previous_step();

protected:
  void closeEvent(QCloseEvent *event);

public slots:
  void start_slot();

private slots:
  void open_file_slot();
  bool save_file_slot();

private:
  _gl_widget *GL_widget;
  QTabWidget *Main_tab_widget;
  QTabWidget *Tab_widget;

//  QHBoxLayout *Edit_horizontal_frame;

//  QFrame *Framed_widget;

  std::string File_name;
  std::string Effect_name;
  bool Link_file;

  QAction *Open_file;
  QAction *Save_file;


};

#endif
