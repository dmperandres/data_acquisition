//LIC

#include "filter_border_mask.h"

using namespace _f_border_mask_ns;

//HEAD

_filter_border_mask::_filter_border_mask()
{
  Number_of_control_points=BORDER_MASK_CONTROL_POINTS_DEFAULT;
  Max_multiplicity=BORDER_MASK_MAX_MULTIPLICITY_DEFAULT;
  Border_width=BORDER_MASK_BORDER_WIDTH_DEFAULT;
  Border_uniformity=BORDER_MASK_BORDER_UNIFORMITY_DEFAULT;
  Border_uniformity_width=BORDER_MASK_BORDER_UNIFORMITY_WIDTH_DEFAULT;

  Change=true;

  Num_channels_input_image_0=1;
  Num_channels_output_image_0=1;

  Type_filter=_f_filter_ns::FILTER_BORDER_MASK;

  Scaling_factor=1;

  Change_output_image_size=false;
  Use_dots=false;
}

//HEA

void _filter_border_mask::reset_data()
{
  Number_of_control_points=BORDER_MASK_CONTROL_POINTS_DEFAULT;
  Max_multiplicity=BORDER_MASK_MAX_MULTIPLICITY_DEFAULT;
  Border_width=BORDER_MASK_BORDER_WIDTH_DEFAULT;
  Border_uniformity=BORDER_MASK_BORDER_UNIFORMITY_DEFAULT;
  Border_uniformity_width=BORDER_MASK_BORDER_UNIFORMITY_WIDTH_DEFAULT;
}

//HEAD

void _filter_border_mask::draw_control_points(cv::Mat *Aux_output_image)
{
  for (unsigned int i=0;i<Points.size();i++){
    cv::circle(*Aux_output_image,cvPoint(int(roundf(Control_points[i].x)),int(roundf(Control_points[i].y))),3,cvScalar(0,0,0));
  }
}

//HEAD

void _filter_border_mask::draw_points(cv::Mat *Aux_output_image)
{
  for (unsigned int i=0;i<Points.size();i++){
    cv::circle(*Aux_output_image,cvPoint(int(roundf(Points[i].x)),int(roundf(Points[i].y))),3,cvScalar(0,0,0));
  }
}

//HEAD

void _filter_border_mask::draw_triangles(cv::Mat *Aux_output_image)
{
  cv::Point Tri_points[3];
  for (unsigned int i=0;i<Output_vertices.size()/3;i++){
    Tri_points[0].x=int(Output_vertices[i*3].GetX());
    Tri_points[0].y=int(Output_vertices[i*3].GetY());
    Tri_points[1].x=int(Output_vertices[i*3+1].GetX());
    Tri_points[1].y=int(Output_vertices[i*3+1].GetY());
    Tri_points[2].x=int(Output_vertices[i*3+2].GetX());
    Tri_points[2].y=int(Output_vertices[i*3+2].GetY());
    cv::fillConvexPoly(*Aux_output_image,Tri_points,3,cvScalar(0,0,0));
  }
}

//HEAD

void _filter_border_mask::triangulate()
{
  Input_vertices.clear();
  Output_vertices.clear();

  for (unsigned int i=0;i<Points.size();i++){
    Input_vertices.push_back(Vector2d(Points[i].x,Points[i].y));
  }

  Triangulate::Process(Input_vertices,Output_vertices);
}

//HEAD

void _filter_border_mask::compute_control_points(unsigned int Width1,unsigned int Height1)
{
  _vertex2f Point;
  _vertex2f Point1;
  _vertex2f Point2;
  _vertex2f Center(Width1/2,Height1/2);
  _vertex2f Direction;
  float t;
  vector<_vertex3f> Control_points_aux;
  unsigned int Border_width1;
  unsigned int Border_uniformity_width1;

  if (Width1>Height1) Border_width1=Height1;
  else Border_width1=Width1;

  Border_uniformity_width1=static_cast<unsigned int>(float(Border_width1)*float(Border_uniformity_width)/100.0f);
  Border_width1=static_cast<unsigned int>(float(Border_width1)*float(Border_width)/100.0f);


  _vertex2f Corners[]={_vertex2f(Border_width1,Border_width1),_vertex2f(Width1-Border_width1,Border_width1),_vertex2f(Width1-Border_width1,Height1-Border_width1),_vertex2f(Border_width1,Height1-Border_width1)};


//  float Radio;
  _random_uniform_double Random_radio(-double(100.0-Border_uniformity)/100.0,double(100.0-Border_uniformity)/100.0);
  _random_uniform_int Random_multiplicity(1,int(Max_multiplicity));

//  Random_multiplicity.seed();

  Control_points_aux.resize(Number_of_control_points*4);
  Multiplicity.resize(Number_of_control_points*4);

  for (unsigned int i=0;i<4;i++){
    Point1=Corners[i];
    Point2=Corners[(i+1)%4];
    for (unsigned int j=0;j<Number_of_control_points;j++){
      t=float(j)/float(Number_of_control_points);
      Point=Point1*(1-t)+Point2*t;
      Direction=Point-Center;
      Direction.normalize();
      Point=Point+Direction*Border_uniformity_width1*float(Random_radio.value());
      Control_points_aux[i*Number_of_control_points+j]=Point;
      Multiplicity[i*Number_of_control_points+j]=static_cast<unsigned int>(Random_multiplicity.value());
    }
  }

  // copy the points
  Control_points.clear();
  Num_valid_sections=0;
  for (unsigned int i=0;i<Control_points_aux.size();i++){
    for (unsigned int j=0;j<Multiplicity[i];j++){
      Control_points.push_back(Control_points_aux[i]);
      Num_valid_sections++;
    }
  }

  // repeat the first 3
  for (unsigned int i=0;i<3;i++){
    for (unsigned int j=0;j<Multiplicity[i];j++){
      Control_points.push_back(Control_points_aux[i]);
    }
  }

  // show control points
//  for (unsigned int i=0;i<Control_points.size();i++){
//    cout << "Control point x: " << Control_points[i].x << " y: " << Control_points[i].y << endl;
//  }
}


//HEAD

void _filter_border_mask::compute_points(unsigned int Num_points1)
{
  float u,u2,u3;
  float B0,B1,B2,B3;
  _vertex3f Point,Point1,Point2,Point3;
  unsigned int Num_points_section=Num_points1/Num_valid_sections;
  unsigned int Count=0;

//  cout << "Num sections: " << Num_valid_sections << " Num_dots_section: " << Num_points_section << endl;
  Points.clear();
  Points.resize(Num_points_section*Num_valid_sections);
  for (unsigned int i=0;i<Num_valid_sections;i++){
    for (unsigned int j=0;j<Num_points_section;j++){
      u=(float(j)/float(Num_points_section));
      u2=u*u;
      u3=u2*u;
      // B0 (-u3+3u2-3u+1)/6
      B0=(-u3+3*u2-3*u+1)/6;
//      B0=((1-u)*(1-u)*(1-u))/6;
      // B1 (3u3-6u2+4)/6
      B1=(3*u3-6*u2+4)/6;
      // B2 (-3*u3+3u2+3u+1)/6
      B2=(-3*u3+3*u2+3*u+1)/6;
      // B3 u3/6
      B3=u3/6;
      //
      Point=Control_points[i]*B0;
      Point1=Control_points[i+1]*B1;
      Point2=Control_points[i+2]*B2;
      Point3=Control_points[i+3]*B3;

      Point+=Point1;
      Point+=Point2;
      Point+=Point3;
      Points[Count++]=Point;
    }
  }

}


void _filter_border_mask::jump_flooding(cv::Mat *Input_image,cv::Mat *Output_image)
{
  vector<_f_distance_field_ns::_point> Image1;
  vector<_f_distance_field_ns::_point> Image2;
  vector<_f_distance_field_ns::_point> *Read=nullptr;
  vector<_f_distance_field_ns::_point> *Write=nullptr;
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
        Image1[static_cast<std::size_t>(i*Input_image->cols+j)]=Point;
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
        Point=(*Read)[static_cast<std::size_t>(Pos)];
        // Go ahead and write our current closest seed, if any. If we don't do this
        // we might lose this information if we don't update our seed this round.
        (*Write)[static_cast<std::size_t>(Pos)]=Point;

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
              Point_aux=(*Read)[static_cast<std::size_t>(Pos1)];

              // If the neighbor doesn't have a closest seed yet, skip it
              if(Point_aux.x==-1 || Point_aux.y==-1) continue;

              // Calculate the distance from us to the neighbor's closest seed
              Distance_aux=sqrtf((Point_aux.x-j)*(Point_aux.x-j) + (Point_aux.y-i)*(Point_aux.y-i));

              // If dist is -1, it means we have no closest seed, so we might as well take this one
              // Otherwise, only adopt this new seed if it's closer than our current closest seed
              if (Distance==-1 || Distance_aux<Distance) {
                Point_aux.Distance=Distance_aux;
                (*Write)[static_cast<std::size_t>(Pos)]=Point_aux;
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

  unsigned int Border_width1;

  if (Input_image->cols>Input_image->rows) Border_width1=static_cast<unsigned int>(Input_image->rows);
  else Border_width1=static_cast<unsigned int>(Input_image->cols);

  Border_width1=static_cast<unsigned int>(float(Border_width1)*float(Border_width)/100.0f);

  float Value;
  for (unsigned int i=0;i<Input_image->total();i++){
    Value=(*Write)[static_cast<std::size_t>(i)].Distance;
    Value/=Border_width1;
    Value=Value*255;
    if (Value>255) Output_image->at<unsigned char>(int(i))=255;
    else Output_image->at<unsigned char>(int(i))=static_cast<unsigned char>(Value);
  }

}



void _filter_border_mask::border_mask(cv::Mat *Aux_input_image,cv::Mat *Aux_output_image)
{
  Aux_output_image->setTo(255);

  compute_control_points(static_cast<unsigned int>(Aux_input_image->cols),static_cast<unsigned int>(Aux_input_image->rows));
  compute_points(Num_valid_sections*10);
  triangulate();

  cv::Mat Aux_image1;
  cv::Mat Aux_image2;

  Aux_image1.create(Aux_input_image->rows,Aux_input_image->cols,CV_8UC1);
  Aux_image1.setTo(255);

  Aux_image2.create(Aux_input_image->rows,Aux_input_image->cols,CV_8UC1);
  Aux_image2.setTo(255);

  draw_triangles(&Aux_image1);

  jump_flooding(&Aux_image1,&Aux_image2);

  unsigned int Value;
  for (unsigned int i=0;i<Aux_image2.total();i++){
    if (Aux_image2.at<unsigned char>(int(i))!=0){
      Value=Aux_input_image->at<unsigned char>(int(i))+Aux_image2.at<unsigned char>(int(i));
      if (Value>255) Aux_output_image->at<unsigned char>(int(i))=255;
      else Aux_output_image->at<unsigned char>(int(i))=Aux_input_image->at<unsigned char>(int(i))+Aux_image2.at<unsigned char>(int(i));
    }
    else Aux_output_image->at<unsigned char>(int(i))=Aux_input_image->at<unsigned char>(int(i));
  }

//    draw_points(Aux_output_image);
}

//HEA

void _filter_border_mask::update()
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
      Aux_output_image=new cv::Mat;
      Aux_output_image->create(Output_image_0->rows,Output_image_0->cols,CV_8UC1);

      border_mask(Aux_input_image,Aux_output_image);
      cvtColor(*Aux_output_image,*Output_image_0,CV_GRAY2RGB,3);
    }
    else cout << "Error in the number of channels in the output image " << __LINE__ << " " << __FILE__ << endl;
  }
  else{// the same number of channels
    border_mask(Aux_input_image,Output_image_0);
  }

  if (Aux_input_image!=nullptr && Aux_input_image!=Input_image_0) delete Aux_input_image;
  if (Aux_output_image!=nullptr && Aux_output_image!=Output_image_0) delete Aux_output_image;
}

