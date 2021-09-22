//LIC

#ifndef FILTERS_TAB_H
#define FILTERS_TAB_H

#include <QDialog>
#include <QListWidget>
#include <QGroupBox>
#include <QSpinBox>
#include <QVBoxLayout>
#include <string>

#include "line_edit.h"

class _gl_widget;

//HEA

class _filters_tab: public QWidget
{
Q_OBJECT

public:
  _filters_tab(_gl_widget *GL_widget, QWidget *Parent = nullptr);
  void  set_item(std::string Name);
  void  insert_widgets();
  void  remove_widgets();
  void  set_selected_item(int Pos);
  void  clear(){Filters_list->clear();}

protected slots:
  void item_clicked(QListWidgetItem *);

private:
  QGroupBox *Group_box_filters;
  QListWidget *Filters_list;
  QVBoxLayout *Vertical_parameters;
  _gl_widget *GL_widget;
};

#endif
