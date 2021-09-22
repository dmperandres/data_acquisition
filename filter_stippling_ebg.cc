//LIC

#include "filter_stippling_ebg.h"
#include "glwidget.h"

using namespace _f_stippling_ebg_ns;

//HEA

_filter_stippling_ebg::_filter_stippling_ebg()
{
  Scaling_factor=STIPPLING_EBG_SCALING_FACTOR_DEFAULT;
  Displacement_ramdomness=STIPPLING_EBG_DISPLACEMENT_DEFAULT;
  Num_rows_dot_matrix=STIPPLING_EBG_NUM_ROWS_DOT_MATRIX_DEFAULT;
  Num_cols_dot_matrix=STIPPLING_EBG_NUM_COLS_DOT_MATRIX_DEFAULT;
  Dot_size_min=STIPPLING_EBG_MIN_DOT_SIZE_DEFAULT;
  Dot_size_max=STIPPLING_EBG_MAX_DOT_SIZE_DEFAULT;
  Pixel_density=STIPPLING_EBG_PIXEL_DENSITY_DEFAULT;
  Pixel_density_factor=VEC_PIXEL_DENSITY_FACTOR[Pixel_density];
  Black_level=STIPPLING_EBG_BLACK_LEVEL_DEFAULT;

  Num_channels_input_image_0=1;
  Num_channels_output_image_0=1;

  Type_filter=_f_filter_ns::FILTER_STIPPLING_EBG;

  Change_output_image_size=true;

  Seeds_initialized=false;
  Dots_texture_packet=0;
  Use_dots=true;
}

//HEA

void _filter_stippling_ebg::reset_data()
{
  Scaling_factor=STIPPLING_EBG_SCALING_FACTOR_DEFAULT;
  Displacement_ramdomness=STIPPLING_EBG_DISPLACEMENT_DEFAULT;
  Num_rows_dot_matrix=STIPPLING_EBG_NUM_ROWS_DOT_MATRIX_DEFAULT;
  Num_cols_dot_matrix=STIPPLING_EBG_NUM_COLS_DOT_MATRIX_DEFAULT;
  Dot_size_min=STIPPLING_EBG_MIN_DOT_SIZE_DEFAULT;
  Dot_size_max=STIPPLING_EBG_MAX_DOT_SIZE_DEFAULT;
  Pixel_density=STIPPLING_EBG_PIXEL_DENSITY_DEFAULT;
  Pixel_density_factor=VEC_PIXEL_DENSITY_FACTOR[Pixel_density];
  Black_level=STIPPLING_EBG_BLACK_LEVEL_DEFAULT;
}

//HEA

void _filter_stippling_ebg::set_dots_images(vector<vector<cv::Mat *> > *Dots1)
{
  Dots=Dots1;
}

//HEA

void _filter_stippling_ebg::pixel_density(int Pixel_density1)
{
  Pixel_density=Pixel_density1;
  Pixel_density_factor=VEC_PIXEL_DENSITY_FACTOR[Pixel_density];
}

//HEA

void _filter_stippling_ebg::output_image_size(int &Width1,int &Height1)
{
  // the size of the output image must be changed
  Width1=(int)((float)Input_image_0->cols*Scaling_factor*Pixel_density_factor);
  Height1=(int)((float)Input_image_0->rows*Scaling_factor*Pixel_density_factor);

  adjust_image_sizes(Width1,Height1);
}

//HEA

void _filter_stippling_ebg::put_dot(cv::Mat *Output_image0,float Row1,float Col1,unsigned int Selected_dot_size1,unsigned int Index_row1,unsigned int Index_col1)
{
  int Row,Col;
  int Row_out,Col_out;
  unsigned char Value;
  unsigned int Row_aux=Selected_dot_size1*Index_row1*Pixel_density_factor;
  unsigned int Col_aux=Selected_dot_size1*Index_col1*Pixel_density_factor;

  // All the pixels of the dot are modified
  for (Row=0;Row<(int)(Selected_dot_size1*Pixel_density_factor);Row++){
    for (Col=0;Col<(int)(Selected_dot_size1*Pixel_density_factor);Col++){
      // get the value of the image of dots of size Selected_dot_size1

      Value=(*Dots)[Pixel_density][Selected_dot_size1]->at<unsigned char>(Row+Row_aux,Col+Col_aux);
      // output position
      Row_out=Row1+Row;
      Col_out=Col1+Col;

      if (Row_out<0 || Row_out>=Output_image0->rows || Col_out<0 || Col_out>=Output_image0->cols) continue;
      // the row and col are correct, the color is changed
      Output_image0->at<unsigned char>(Row_out,Col_out)=(unsigned char)((float)Output_image0->at<unsigned char>(Row_out,Col_out)*(float)Value/255.0);
    }
  }
}

//HEA

void _filter_stippling_ebg::stippling(cv::Mat *Input_image0,cv::Mat *Output_image0)
{
  int Row,Col,Selected_dot_size;
  float Pos_row,Pos_col;
  int Selected_row,Selected_col;
  _vertex2f Position;
  unsigned int Counter_of_dots=0;

  if (Seeds_initialized==false){
    Index_row.seed();
    Dot_size.seed();
    Index_col.seed();
    Displacement.seed();
    save_seeds();
    Seeds_initialized=true;
  }
  else load_seeds();

  Index_row.init(0,Num_rows_dot_matrix-1);
  Index_col.init(0,Num_cols_dot_matrix-1);
  Dot_size.init(Dot_size_min,Dot_size_max-1);
  Displacement.init(-Displacement_ramdomness,Displacement_ramdomness);

  for (Row=0;Row<Input_image0->rows;Row++){
    for (Col=0;Col<Input_image0->cols;Col++){
      if (Input_image0->at<unsigned char>(Row,Col)==BLACK){
        // compute a random dot size
        Selected_dot_size=Dot_size.value();

        Counter_of_dots++;
        //
        Selected_row=(int)Index_row.value();
        Selected_col=(int)Index_col.value();

        Pos_row=(float)Row*Scaling_factor*(float)Pixel_density_factor+(Displacement.value()*(float)AVERAGE_DOT_SIZE*(float)Pixel_density_factor)-(float)Selected_dot_size/2;
        Pos_col=(float)Col*Scaling_factor*(float)Pixel_density_factor+(Displacement.value()*(float)AVERAGE_DOT_SIZE*(float)Pixel_density_factor)-(float)Selected_dot_size/2;
        put_dot(Output_image0,Pos_row,Pos_col,Selected_dot_size,Selected_row,Selected_col);
      }
    }
  }

  set_info1(Counter_of_dots);

  if (Output_mode==OUTPUT_MODE_MONO_COLOR){
    for (Row=0;Row<Output_image_0->rows;Row++){
      for (Col=0;Col<Output_image_0->cols;Col++){
        if (Output_image_0->at<unsigned char>(Row,Col)<=Black_level) Output_image_0->at<unsigned char>(Row,Col)=BLACK;
        else Output_image_0->at<unsigned char>(Row,Col)=WHITE;
      }
    }
  }
}

//HEA

void _filter_stippling_ebg::update()
{
  cv::Mat *Aux_input_image=nullptr;
  cv::Mat *Aux_output_image=nullptr;

  int Width1,Height1;

  output_image_size(Width1,Height1);
  // check the output size
  if (Input_image_0->cols!=Width1 || Input_image_0->rows!=Height1){
    Output_image_0->release();
    Output_image_0->create(Height1,Width1,CV_8UC1);
  }

  // Check the number of input channels
  if (Input_image_0->channels()!=Num_channels_input_image_0){// Different number of channels
    if (Input_image_0->channels()==3){
      // conversion
      Aux_input_image=new cv::Mat;
      Aux_input_image->create(Input_image_0->rows,Input_image_0->cols,CV_8UC1);
      cvtColor(*Input_image_0,*Aux_input_image,CV_BGR2GRAY,1);
    }
    else cout << "Error in the number of channels in the input image " << __LINE__ << " " << __FILE__ << endl;
  }
  else{// the same number of channels
    Aux_input_image=Input_image_0;
  }

  // Check the number of output channels
  if (Output_image_0->channels()!=Num_channels_output_image_0){// Different number of channels
    if (Output_image_0->channels()==3){
      // conversion
      Aux_output_image=new cv::Mat;
      Aux_output_image->create(Output_image_0->rows,Output_image_0->cols,CV_8UC1);

      // set the output image to white
      Output_image_0->setTo(cv::Scalar(255,255,255));
      stippling(Aux_input_image,Aux_output_image);
      cvtColor(*Aux_output_image,*Output_image_0,CV_GRAY2RGB,3);
    }
    else cout << "Error in the number of channels in the output image " << __LINE__ << " " << __FILE__ << endl;
  }
  else{// the same number of channels
    // set the output image to white
    Output_image_0->setTo(255);
    stippling(Aux_input_image,Output_image_0);
  }

  if (Aux_input_image!=nullptr && Aux_input_image!=Input_image_0) delete Aux_input_image;
  if (Aux_output_image!=nullptr && Aux_output_image!=Output_image_0) delete Aux_output_image;
}

//HEA

void _filter_stippling_ebg::displacement_ramdomness(float Displacement_ramdomness1)
{
  Displacement_ramdomness=Displacement_ramdomness1;
}

//HEA

void _filter_stippling_ebg::save_seeds()
{
  Index_row.save_seed();
  Index_col.save_seed();
  Displacement.save_seed();
  Dot_size.save_seed();
}

//HEA

void _filter_stippling_ebg::load_seeds()
{
  Index_row.load_seed();
  Index_col.load_seed();
  Displacement.load_seed();
  Dot_size.load_seed();
}

//HEA

void _filter_stippling_ebg::adjust_image_sizes(int &Width1, int &Height1)
{
  float Rest;

  if (Width1%4!=0){
    Rest=round((float)Width1/4.0);
    Width1=(int)Rest*4;

  }
  if (Height1%4!=0){
    Rest=round((float)Height1/4.0);
    Height1=(int)Rest*4;
  }
}
