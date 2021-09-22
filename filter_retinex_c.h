//LIC

#ifndef _FILTER_RETINEX_C_H
#define _FILTER_RETINEX_C_H

#include <GL/glew.h>
#include "shaders.h"

#include <string>
#include <limits>
#include "filter.h"

#define DEFINED_FILTER_RETINEX_C

class _gl_widget;

namespace _f_retinex_c_ns
{
  typedef enum {MODE_UNIFORM,MODE_LOW,MODE_HIGH} _filter_mode;

  // max-min values
  const int RETINEX_MAX_NUM_KERNELS=3;
  const float RETINEX_MAX_COLOR_RESTORATION_VARIANCE=2.0f; // check 2.0


  // default values

  const int RETINEX_NUM_KERNELS_DEFAULT=3;
  const int RETINEX_SMALL_GAUSSIAN_KERNEL_SIZE_DEFAULT=3;
  const int RETINEX_MEDIUM_GAUSSIAN_KERNEL_SIZE_DEFAULT=127;
  const int RETINEX_BIG_GAUSSIAN_KERNEL_SIZE_DEFAULT=255;
  const float RETINEX_COLOR_RESTORATION_VARIANCE_DEFAULT=1.0f;

  const float CONTRAST_DEFAULT=1.0f;
  const float MAX_CONTRAST=5.0f;
  const int BRIGHT_DEFAULT=0;

//  const int Contrast_divisor=10;
}

class _qtw_filter_retinex_c;

//HEA

class _filter_retinex_c : public _filter
{
public:
  _filter_retinex_c(_gl_widget *GL_widget1);
  ~_filter_retinex_c();
  void reset_data();
  bool change_output_image_size(){return Change_output_image_size;}
  bool use_dots(){return Use_dots;}
  void create_buffers();

  void update();

  void color_restoration_variance(float Color_restoration_variance1);
  float color_restoration_variance(){return Color_restoration_variance;}

  void small_gaussian_size(int Small_gaussian_size1);
  int  small_gaussian_size(){return Vec_kernel_size[0];}

  void medium_gaussian_size(int Medium_gaussian_size1);
  int  medium_gaussian_size(){return Vec_kernel_size[1];}

  void big_gaussian_size(int Big_gaussian_size1);
  int  big_gaussian_size(){return Vec_kernel_size[2];}

  void  contrast(float Contrast1){Contrast=Contrast1;}
  float contrast(){return Contrast;}

  void bright(int Bright1){Bright=Bright1;}
  int  bright(){return Bright;}

  void kernel_size(unsigned int Pos,int Value);
  int kernel_size(unsigned int Pos){
      if (int(Pos)<Num_kernels) return Vec_kernel_size[Pos];
      else return(3);
  }

  int num_kernels(){return Num_kernels;}

  void compute_gaussians_distribution();

  int odd_adjust(int Value){if (Value%2==0) return(Value+1);else return Value;}

protected:
  int Num_kernels;
  _f_retinex_c_ns::_filter_mode Mode;
  float Color_restoration_variance;
  int Vec_kernel_size[_f_retinex_c_ns::RETINEX_MAX_NUM_KERNELS];
  float Contrast;
  int Bright;

  GLuint Program0;
  GLuint Program1a;
  GLuint Program1b;
  GLuint Program2;
  GLuint Program3;
  GLuint Program4;
  GLuint Program5;

  GLuint VAO1;

  GLuint Fbo_retinex;

  GLuint Tex_orig;
  GLuint Tex_orig_float;
  GLuint Tex_gaussian[6]; // for 2 pass

  GLuint Buffer_gaussians_coeffs;
  GLuint Buffer_mean;
  GLuint Buffer_square;

//  bool Created_buffers;
  int Size_buffer;

  _gl_widget *GL_widget;
};
#endif
