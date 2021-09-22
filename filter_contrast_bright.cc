//LIC

#include "filter_contrast_bright.h"
#include "glwidget.h"

using namespace _f_contrast_bright_ns;

//HEA

_filter_contrast_bright::_filter_contrast_bright()
{
  Contrast=CANNY_CONTRAST_DEFAULT;
  Bright=CANNY_BRIGHT_DEFAULT;

  Num_channels_input_image_0=1;
  Num_channels_output_image_0=1;

  Type_filter=_f_filter_ns::FILTER_CONTRAST_BRIGHT;

  Scaling_factor=1;
  Change_output_image_size=false;
  Use_dots=false;
}

//HEA

void _filter_contrast_bright::reset_data()
{
  Contrast=CANNY_CONTRAST_DEFAULT;
  Bright=CANNY_BRIGHT_DEFAULT;
}

//HEA

void _filter_contrast_bright::update()
{
  cv::Mat *Aux_input_image=nullptr;
  cv::Mat *Aux_output_image=nullptr;
  cv::Mat Aux_float_image;

  // check the output size
  if (Input_image_0->cols!=Output_image_0->cols || Input_image_0->rows!=Output_image_0->rows){
    Output_image_0->release();
    Output_image_0->create(Input_image_0->rows,Input_image_0->cols,CV_8UC1);
  }

  // Check the number of input channels
  if (Input_image_0->channels()!=Num_channels_input_image_0){// Different number of channels
    if (Input_image_0->channels()==3){
      // conversion

      Aux_input_image=new cv::Mat;
      Aux_input_image->create(Input_image_0->rows,Input_image_0->cols,CV_8UC1);
      cvtColor(*Input_image_0,*Aux_input_image,CV_BGR2GRAY,1);

      Aux_input_image->convertTo(Aux_float_image,CV_32F);
    }
    else cout << "Error in the number of channels in the input image " << __LINE__ << " " << __FILE__ << endl;
  }
  else{// the same number of channels
    Input_image_0->convertTo(Aux_float_image,CV_32F);
  }

  Aux_float_image.convertTo(Aux_float_image,-1,Contrast,Bright);

  // Check the number of output channels
  if (Output_image_0->channels()!=Num_channels_output_image_0){// Different number of channels
    if (Output_image_0->channels()==3){
      // conversion
      Aux_output_image=new cv::Mat;
      Aux_output_image->create(Output_image_0->rows,Output_image_0->cols,CV_8UC1);

      Aux_float_image.convertTo(*Aux_output_image,CV_8UC1);

      cvtColor(*Aux_output_image,*Output_image_0,CV_GRAY2RGB,3);
    }
    else cout << "Error in the number of channels in the output image " << __LINE__ << " " << __FILE__ << endl;
  }
  else{// the same number of channels
    Aux_float_image.convertTo(*Output_image_0,CV_8UC1);
  }

  if (Aux_input_image!=nullptr && Aux_input_image!=Input_image_0) delete Aux_input_image;
  if (Aux_output_image!=nullptr && Aux_output_image!=Output_image_0) delete Aux_output_image;
}

//HEA

void _filter_contrast_bright::contrast(float Contrast1)
{
  Contrast=Contrast1;
}

//HEA

void _filter_contrast_bright::bright(float Bright1)
{
  Bright=Bright1;
}
