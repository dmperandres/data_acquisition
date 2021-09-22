//LIC

#include "window.h"
#include "glwidget.h"

//HEA

_window::_window()
{
  QSizePolicy Q(QSizePolicy::Expanding,QSizePolicy::Expanding);

  QWidget *Central_widget = new QWidget(this);

  QGLFormat Format;
  Format.setDoubleBuffer(true);
  Format.setRgba(true);
  Format.setAlpha(true);
  QGLFormat::setDefaultFormat(Format);

  // frame widget for OpenGL window
//  Framed_widget= new QFrame;
//  Framed_widget->setSizePolicy(Q);
//  Framed_widget->setFrameStyle(QFrame::Panel);
//  Framed_widget->setLineWidth(3);

  // OpenGL windows inside the frame widget
  GL_widget=new _gl_widget(this);//,Framed_widget);// Central_widget
  GL_widget->setSizePolicy(Q);
  GL_widget->setMinimumSize(400,400);

  //
//  QHBoxLayout *Horizontal_frame = new QHBoxLayout;
//  Horizontal_frame->setContentsMargins(1,1,1,1);

//  Horizontal_frame->addWidget(GL_widget);
//  Framed_widget->setLayout(Horizontal_frame);

  // This is the tab
  Tab_widget = new QTabWidget(Central_widget);

  // and the widgets to the tab
//  Tab_widget->addTab(GL_widget->Filters_tab, tr("Parameters"));
  Tab_widget->addTab(GL_widget->Widget_parameters, tr("Parameters"));
  Tab_widget->hide();


  QFontMetrics Font_metric(Tab_widget->font());
  int Width_text = Font_metric.width("M");
  Tab_widget->setMaximumWidth(Width_text*_window_ns::WIDTH_TAB_CHARS);

  // the layout of the central widget
  QHBoxLayout *Horizontal_main = new QHBoxLayout(Central_widget);
  Horizontal_main->setSpacing(10);
  Horizontal_main->setMargin(3);
  Horizontal_main->addWidget(GL_widget);
  Horizontal_main->addWidget(Tab_widget);

  Central_widget->setLayout(Horizontal_main);

  setCentralWidget(Central_widget);

  // actions for file menu
  Open_file = new QAction(QIcon("./icons/fileopen.png"), tr("&Open input image"), this);
  Open_file->setShortcuts(QKeySequence::Open);
  Open_file->setToolTip(tr("Open a file"));
  connect(Open_file, SIGNAL(triggered()), this, SLOT(open_file_slot()));
//  Open_file->setEnabled(false);

  Save_file = new QAction(QIcon("./icons/filesave.png"), tr("&Save resulting stipple image "), this);
  Save_file->setShortcuts(QKeySequence::Save);
  Save_file->setToolTip(tr("Save a file"));
  connect(Save_file, SIGNAL(triggered()), this, SLOT(save_file_slot()));
//  Save_file->setEnabled(false);

  QAction *Exit = new QAction(QIcon("./icons/exit.png"), tr("&Exit..."), this);
  Exit->setShortcut(tr("Ctrl+Q"));
  Exit->setToolTip(tr("Exit the application"));
  connect(Exit, SIGNAL(triggered()), this, SLOT(close()));

  // menus
//  QMenu *File_menu=menuBar()->addMenu(tr("&File"));
//  File_menu->addAction(Open_file);
//  File_menu->addAction(Save_file);
//  File_menu->addSeparator();
//  File_menu->addAction(Exit);
//  File_menu->setAttribute(Qt::WA_AlwaysShowToolTips);

  setWindowTitle(tr("Contrast editor"));

  QRect Screen_size = QApplication::primaryScreen()->geometry();
  resize(int(float(Screen_size.width())*_window_ns::SCREEN_WIDTH_FACTOR),int(float(Screen_size.height())*_window_ns::SCREEN_HEIGHT_FACTOR));
}

//HEA

void _window::open_file_slot()
{
  QString File_name1 = QFileDialog::getOpenFileName(this, tr("Open File"),"./images",tr("Images (*.png *.jpg *.pgm)"));

  if (!File_name1.isEmpty()){
    QFile File(File_name1);
    if (!File.open(QFile::ReadOnly | QFile::Text)) {
      QMessageBox::warning(this, tr("Application"),tr("Cannot read file %1:\n%2.").arg(File_name1).arg(File.errorString()));
      return;
    }
    Tab_widget->setEnabled(true);
    Tab_widget->show();
    GL_widget->read_image(File_name1.toStdString());
    // the name is saved
    File_name=File_name1.toStdString();
    Save_file->setEnabled(true);
  }
}

//HEA

bool _window::save_file_slot()
{
  QString File_name,File_type1;
  QFileDialog File_dialog;
  int File_type=-1;

  do {
    File_name = File_dialog.getSaveFileName(this, tr("Save File"),"./images",tr("Images (*.png *.jpg)"));
    if (File_name.isEmpty()) return false;
    if (File_name.endsWith(".png")) File_type=0;
    if (File_name.endsWith(".jpg")) File_type=1;
    if (File_type==-1){
      QMessageBox::warning(this, tr("Warning"),tr("The file must be of type .png or .jpg"));
    }
  } while (File_type==-1);

  GL_widget->save_image(File_name.toStdString());

  return true;
}

//HEA

void _window::closeEvent(QCloseEvent *event)
{
  event->accept();
}

//HEA

void _window::start_slot()
{
  // the personal data

  #ifdef DEBUG
  cout << "en start_slot" << endl;
  #endif

  Tab_widget->setEnabled(true);
  Tab_widget->show();
  GL_widget->next_image();
}


