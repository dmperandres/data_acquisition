//LIC

#include <QApplication>
#include <QWizardPage>
#include <QLabel>
#include <QVBoxLayout>
#include <QFontDatabase>
#include <QSurfaceFormat>
#include <QMessageBox>
#include <fstream>

#include "window.h"

#ifdef ENGLISH
const QString String_start("Start");
const QString Intro("Most parts of this program and its associated data have been developed by Domingo Martín (dmartin@ugr.es)\n\nThe program can be used in non-commercial applications\n\nThe images have copyright.");

#else
const QString String_start("Comenzar");
const QString Intro("La mayoría de partes de este programa han sido desarrollados por Domingo Martín (dmartin@ugr.es)\n\nEl programa puede ser usado en aplicaciones no comerciales\n\nLas imágenes tienen copyright.");
#endif

std::ofstream Output_stream;
bool DEBUG=false;

//HEA

QWizardPage *createIntroPage()
{
  QWizardPage *page = new QWizardPage;
  page->setTitle("General information");

//  page->setPixmap(QWizard::WatermarkPixmap, QPixmap("./splash.png"));

  QLabel *label = new QLabel(Intro);
  label->setWordWrap(true);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(label);
  page->setLayout(layout);

  return page;
}

//HEA

int main(int argc, char *argv[])
{
  string File_name;

  if (argc==2){
    if (strcmp(argv[1],"DEBUG")==0) DEBUG=true;
  }

//  cout << DEBUG << endl;

  QApplication Application(argc, argv);
  _window Window;
  //Window.read_settings();

  if (DEBUG){
    Output_stream.open("log.txt",std::ofstream::out);
    cout.rdbuf(Output_stream.rdbuf());
    cout << "Debugging" << endl;
  }

  setlocale(LC_NUMERIC, "C");

  QSurfaceFormat Format;
  Format.setDepthBufferSize(24);
  Format.setStencilBufferSize(8);
  QSurfaceFormat::setDefaultFormat(Format);

  QFont Font("Arial");
  Font.setPointSize(10);
  Application.setFont(Font);

  //QFontDatabase Database;
  //int Result = Database.addApplicationFont("fonts/Ubuntu-R.ttf");
  //QFont Font=QFont("Ubuntu",10);
  //Application.setFont(Font);


  Window.setWindowState(Qt::WindowMaximized);
  Window.show();

  QWizard wizard;
  wizard.setOptions(QWizard::NoCancelButton | QWizard::NoBackButtonOnLastPage);
  wizard.addPage(createIntroPage());
  wizard.setWindowTitle("License");
  wizard.raise();
  wizard.exec();

  QMessageBox Messagebox;
  Font.setPointSize(30);
  Messagebox.setFont(Font);
  Messagebox.addButton(String_start,QMessageBox::YesRole);
  Messagebox.exec();

  Window.start_slot();

  return Application.exec();
}
