#ifndef _FILTER_DISTANCE_FIELD_H
#define _FILTER_DISTANCE_FIELD_H

#include <string>
#include <vector>
#include <iostream>
#include "filter.h"

#define DEFINED_FILTER_DISTANCE_FIELD

namespace _f_distance_field_ns
{
  // Default values
  const int DISTANCE_FIELD_LINE_WIDTH_DEFAULT=3;

  class _point
  {
  public:
    float Distance=-1;
    int x=-1;
    int y=-1;
  };
}

class _gl_widget;
class _qtw_filter_distance_field;

//HEA

class _filter_distance_field : public _filter
{
public:
    _filter_distance_field();
    void reset_data();
    bool change_output_image_size(){return Change_output_image_size;}
    bool use_dots(){return Use_dots;}

    void jump_flooding(cv::Mat *Input_image,cv::Mat *Output_image);
    void update();
    void line_width(int Line_width1){Line_width=Line_width1;}

    int line_width(){return Line_width;}

    int Line_width;
};
#endif
