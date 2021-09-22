//LIC

#include "filter_distance_field.h"

using namespace std;
using namespace _f_distance_field_ns;

//HEA

_filter_distance_field::_filter_distance_field()
{
  Line_width=DISTANCE_FIELD_LINE_WIDTH_DEFAULT;

  Num_channels_input_image_0=1;
  Num_channels_output_image_0=1;

  Type_filter=_f_filter_ns::FILTER_DISTANCE_FIELD;

  Scaling_factor=1;
  Change_output_image_size=false;
  Use_dots=false;
}

//HEA

void _filter_distance_field::reset_data()
{
  Line_width=DISTANCE_FIELD_LINE_WIDTH_DEFAULT;
}

//HEA
// jump-flooding method

void _filter_distance_field::jump_flooding(cv::Mat *Input_image,cv::Mat *Output_image)
{
  std::vector<_f_distance_field_ns::_point> Image1;
  std::vector<_f_distance_field_ns::_point> Image2;
  std::vector<_f_distance_field_ns::_point> *Read,*Write;
  _f_distance_field_ns::_point Point;
  _f_distance_field_ns::_point Point_aux;
  int Step;
  bool Read_image1;
  int Pos;
  int Pos1;
  float Distance;
  float Distance_aux;

  Image1.resize(Input_image->total());
  Image2.resize(Input_image->total());

  // asigning the seeds (distance=0,x0,y0)
  for (int i=0;i<Input_image->rows;i++){
    for (int j=0;j<Input_image->cols;j++){
      if (Input_image->at<unsigned char>(i,j)==0){
        // this is a seed
        Point.Distance=0;
        Point.x=j;
        Point.y=i;
        Image1[i*Input_image->cols+j]=Point;
      }
    }

  }

  if (Input_image->cols>=Input_image->rows) Step=Input_image->cols/2;
  else Step=Input_image->rows/2;

  Read_image1=true;

  while (Step>=1){
    // select the reading buffer
    if (Read_image1==true){
      Read=&Image1;
      Write=&Image2;
    }
    else{
      Write=&Image1;
      Read=&Image2;
    }

    // iterate all the pixels
    for (int i=0;i<Input_image->rows;i++){
      for (int j=0;j<Input_image->cols;j++){
        Pos=i*Input_image->cols+j;

        // Closest seed (if any)
        Point=(*Read)[Pos];
        // Go ahead and write our current closest seed, if any. If we don't do this
        // we might lose this information if we don't update our seed this round.
        (*Write)[Pos]=Point;

        // this is a seed, skip it
        if (Point.x == j && Point.y ==i) continue;

        if(Point.x == -1 || Point.y== -1 ){
          // No closest seed has been found yet
          Distance = -1;
        }
        else{
         // Current closest seed's distance
         Distance = sqrtf((Point.x-j)*(Point.x-j) + (Point.y-i)*(Point.y-i));
        }

        int Pos_y;
        int Pos_x;
        for( int k = -1; k <= 1; k++ ){
          for( int l = -1; l <= 1; l++ ){
            if (k!=0 || l!=0){
              // not to check with itself

              // Calculate neighbor's row and column
              Pos_y = i + k * Step;
              Pos_x = j + l * Step;

              // If the neighbor is outside the bounds of the buffer, skip it
              if( Pos_x < 0 || Pos_x >= Input_image->cols || Pos_y < 0 || Pos_y >= Input_image->rows ) continue;

              // Calculate neighbor's absolute index
              Pos1 =(Pos_y*Input_image->cols)+Pos_x;

              // Retrieve the neighbor
              Point_aux=(*Read)[Pos1];

              // If the neighbor doesn't have a closest seed yet, skip it
              if(Point_aux.x==-1 || Point_aux.y==-1) continue;

              // Calculate the distance from us to the neighbor's closest seed
              Distance_aux=sqrtf((Point_aux.x-j)*(Point_aux.x-j) + (Point_aux.y-i)*(Point_aux.y-i));

              // If dist is -1, it means we have no closest seed, so we might as well take this one
              // Otherwise, only adopt this new seed if it's closer than our current closest seed
              if (Distance==-1 || Distance_aux<Distance) {
                Point_aux.Distance=Distance_aux;
                (*Write)[Pos]=Point_aux;
                Distance=Distance_aux;
              }
            }
          }
        }
      }
    }

    // Half the step.
    Step /= 2;
    // Swap the buffers for the next round
    Read_image1=!Read_image1;
  }

//  float Value;
//  for (unsigned int i=0;i<Input_image->total();i++){
//    Value=(*Write)[i].Distance;
//    Value=Value/(float) Line_width;
//    if ((int)Value%2==0) Value=255;
//    else Value=0;
//    Output_image->at<unsigned char>(i)=(int)Value;
//  }

  float Max_distance=-1e6;
  for (unsigned int i=0;i<(*Write).size();i++){
    if ((*Write)[i].Distance>Max_distance) Max_distance=(*Write)[i].Distance;
  }

  float Value;
  for (unsigned int i=0;i<Input_image->total();i++){
    Value=(*Write)[i].Distance;
    Value/=Max_distance;
    Value=Value*255;
    Output_image->at<unsigned char>(i)=(int)Value;
  }

}

//HEA

void _filter_distance_field::update()
{
  cv::Mat *Aux_input_image=nullptr;
  cv::Mat *Aux_output_image=nullptr;

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

      jump_flooding(Aux_input_image,Aux_output_image);
      cvtColor(*Aux_output_image,*Output_image_0,CV_GRAY2RGB,3);
    }
    else cout << "Error in the number of channels in the output image " << __LINE__ << " " << __FILE__ << endl;
  }
  else{// the same number of channels
    jump_flooding(Aux_input_image,Output_image_0);
  }

  if (Aux_input_image!=nullptr && Aux_input_image!=Input_image_0) delete Aux_input_image;
  if (Aux_output_image!=nullptr && Aux_output_image!=Output_image_0) delete Aux_output_image;
}

