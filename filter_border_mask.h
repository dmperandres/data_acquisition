//LIC

#ifndef _FILTER_BORDER_MASK_H
#define _FILTER_BORDER_MASK_H


#ifdef X64
  #include <opencv.hpp>
#else
  #include <core/core.hpp>
  #include <highgui/highgui.hpp>
#endif

#include <string>
#include "filter.h"
#include "triangulate.h"
#include "vertex.h"

#include "random.h"

#include "filter_distance_field.h"

#define DEFINED_FILTER_CANNY

namespace _f_border_mask_ns
{
  // Default values
  const int BORDER_MASK_CONTROL_POINTS_DEFAULT=10;
  const int BORDER_MASK_MAX_MULTIPLICITY_DEFAULT=1;
  const int BORDER_MASK_BORDER_WIDTH_DEFAULT=10;
  const int BORDER_MASK_BORDER_UNIFORMITY_DEFAULT=100;
  const int BORDER_MASK_BORDER_UNIFORMITY_WIDTH_DEFAULT=20;

  #ifdef WINDOWS
  const double M_PI = 3.14159265358979323846;
  #endif
}

class _gl_widget;
class _qtw_filter_border_mask;

class _filter_border_mask : public _filter
{
public:
  _filter_border_mask();
  void reset_data();
  bool change_output_image_size(){return Change_output_image_size;}
  bool use_dots(){return Use_dots;}
  void update();

  // aux functions
//  void set_paper(cv::Mat *Paper1){Paper=Paper1;}
//  void clear_window();
//  void change_mvp();
  void draw_control_points(cv::Mat *Aux_output_image);
  void draw_points(cv::Mat *Aux_output_image);
  void draw_triangles(cv::Mat *Aux_output_image);
  void triangulate();
  void compute_control_points(unsigned int Width1, unsigned int Height1);
  void compute_points(unsigned int Num_points1);
  void jump_flooding(cv::Mat *Input_image,cv::Mat *Output_image);
  void border_mask(cv::Mat *Aux_input_image, cv::Mat *Aux_output_image);

  //
  void number_of_control_points(unsigned int Number_of_contro_points1){Number_of_control_points=Number_of_contro_points1;}
  unsigned int number_of_control_points(){return Number_of_control_points;}

  void max_multiplicity(unsigned int Max_multiplicity1){Max_multiplicity=Max_multiplicity1;}
  unsigned int max_multiplicity(){return Max_multiplicity;}

//  void modulate_dot_size(bool Modulate_dot_size1){Modulate_dot_size=Modulate_dot_size1;}
//  bool modulate_dot_size(){return Modulate_dot_size;}

  void border_width(unsigned int Border_width1){Border_width=Border_width1;}
  unsigned int border_width(){return Border_width;}

  void border_uniformity(unsigned int Border_uniformity1){Border_uniformity=Border_uniformity1;}
  unsigned int border_uniformity(){return Border_uniformity;}

  void border_uniformity_width(unsigned int Border_uniformity_width1){Border_uniformity_width=Border_uniformity_width1;}
  unsigned int border_uniformity_width(){return Border_uniformity_width;}

//  void antialias(bool Antialias1){Antialias=Antialias1;}
//  bool antialias(){return Antialias;}

//  void gaussian_kernel_size(int Gaussian_kernel_size1){Gaussian_kernel_size=Gaussian_kernel_size1;}
//  int gaussian_kernel_size(){return Gaussian_kernel_size;}

  void new_dot();
  void set_change(bool Change1){Change=Change1;}

//  void gl_widget(_gl_widget *GL_widget1){GL_widget=GL_widget1;}

  unsigned int Number_of_control_points;
  //    int Number_of_dots;
  unsigned int Max_multiplicity;
//  bool Modulate_dot_size;
  unsigned int Border_width;
  unsigned int Border_uniformity;
  unsigned int Border_uniformity_width;
//  bool Antialias;
//  unsigned int Gaussian_kernel_size;
  bool Change;


//  unsigned int Framebuffer; // id for the framebuffer
//  unsigned int Renderbuffers[_f_border_mask_ns::Num_renderbuffers]; // id for the renderbuffers

  // geometry
  std::vector<_vertex3f> Vertices; // coordinates of vertices
  std::vector<_vertex3i> Triangles; // index of vertices for each triangle

  // the original positions
  std::vector<_vertex3f> Control_points;
  std::vector<unsigned int> Multiplicity;
  std::vector<_vertex3f> Points;

  //
  Vector2dVector Input_vertices;
  Vector2dVector Output_vertices;
  // for reading the image
  std::vector<cv::Vec3b> Pixels;

  // for drawing
  int Window_width;
  int Window_height;

  int Num_divisions;
  int Num_vertices;
  int Num_faces;
  int Radius;
  int Height;
  int Moved_points;

  //
  unsigned int Num_valid_sections;  
};

#endif
