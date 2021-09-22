//LIC

#ifndef IMAGES_TAB_H
#define IMAGES_TAB_H

#include <QDialog>
#include <QListWidget>
#include <QGroupBox>
#include <QSpinBox>
#include <QVBoxLayout>
#include <string>

#include "line_edit.h"

class _gl_widget;

//HEA
// class for the tab that includes the images

class _images_tab: public QWidget
{
  Q_OBJECT

public:
        _images_tab(_gl_widget *GL_widget, QWidget *Parent = 0);
  void  set_item(std::string Name);
  void  set_selected_item(int Pos);
  void  clear(){Images_list->clear();};

protected slots:
  void item_clicked(QListWidgetItem *);

private:
  QListWidget *Images_list;
  _gl_widget *GL_widget;
};

#endif
