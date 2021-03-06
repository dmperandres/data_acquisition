//LIC

#include "images_tab.h"
#include "glwidget.h"

//HEA

_images_tab::_images_tab(_gl_widget *GL_widget1, QWidget *Parent)
: QWidget(Parent)
{
  QString Str;

  GL_widget=GL_widget1;

  // images
  QGroupBox *Group_box_images=new QGroupBox(tr("Images"));
  Group_box_images->setAlignment(Qt::AlignCenter);

  Images_list = new QListWidget;
  Images_list->setToolTip(tr("Selects which images is shown or saved"));

  // for adjusting the size of the widget depending on the height of the font
  QFontMetrics Font_metrics(Images_list->fontMetrics());
  Images_list->setMaximumHeight(Font_metrics.height()*15);

  QVBoxLayout *Vertical_images = new QVBoxLayout;

  Vertical_images->addWidget(Images_list,0,Qt::AlignTop);
  Group_box_images->setLayout(Vertical_images);

  connect(Images_list,SIGNAL(itemClicked(QListWidgetItem *)), this,SLOT(item_clicked(QListWidgetItem *)));

  // general
  QVBoxLayout *Vertical_parameters=new QVBoxLayout;

  Vertical_parameters->addWidget(Group_box_images);

  setLayout(Vertical_parameters);
}

//HEA

void _images_tab::set_item(string Name)
{
  QListWidgetItem *Item=new QListWidgetItem;
  Item->setText(tr(Name.c_str()));
  Images_list->insertItem(Images_list->count(),Item);
  Images_list->updateGeometry();
}

//HEA

void _images_tab::set_selected_item(int Pos)
{
  Images_list->setCurrentRow(Pos);
}

//HEA

void _images_tab::item_clicked(QListWidgetItem *Item)
{
  Q_UNUSED(Item)
// GL_widget->set_image((Item->text()).toStdString());
}
