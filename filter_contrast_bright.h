//LIC

#ifndef _FILTER_CONTRAST_BRIGHT_H
#define _FILTER_CONTRAST_BRIGHT_H

#include <QDialog>
#include <QGroupBox>
#include <QSlider>
#include <QLabel>
#include "line_edit.h"
#include <string>
#include "filter.h"

#define DEFINED_FILTER_CONTRAST_BRIGHT

namespace _f_contrast_bright_ns
{
  // parameter 1
  const std::string String_group_box_parameter1("Contrast");
  const std::string String_label_parameter1_min("0.1 ");
  const std::string String_label_parameter1_max("5");
  const std::string String_label_parameter1_tooltip("Display the value of contrast value");
  const int Parameter1_min_value=1;
  const int Parameter1_max_value=50;
  const int Parameter1_single_step=1;
  const int Parameter1_page_step=5;
  const int Parameter1_tick_interval=5;
  const bool Parameter1_set_tracking=false;
  const std::string String_parameter1_tooltip("Controls the appearance of contrast");
  // warning: the capture value must be adjusted to a odd value
  const int Parameter1_divisor=10;

  // parameter 2
  const std::string String_group_box_parameter2("Bright");
  const std::string String_label_parameter2_min("-255 ");
  const std::string String_label_parameter2_max("255");
  const std::string String_label_parameter2_tooltip("Display the value of bright");
  const int Parameter2_min_value=-255;
  const int Parameter2_max_value=255;
  const int Parameter2_single_step=10;
  const int Parameter2_page_step=25;
  const int Parameter2_tick_interval=25;
  const bool Parameter2_set_tracking=false;
  const std::string String_parameter2_slider_tooltip("Controls the appearance of bright");

  // Default values
  const int CONTRAST_DEFAULT=1;
  const int BRIGHT_DEFAULT=0;
}

class _gl_widget;
class _qtw_filter_contrast_bright;

//HEA

class _filter_contrast_bright : public _filter
{
public:
    _filter_contrast_bright();
    ~_filter_contrast_bright(){}
    void reset_data();
    bool change_output_image_size(){return Change_output_image_size;};
    bool use_dots(){return Use_dots;};

    void update();
    void contrast(float Contrast1);
    void bright(float Bright1);

    float contrast(){return Contrast;};
    float bright(){return Bright;};

    float Contrast;
    int Bright;
};
#endif
