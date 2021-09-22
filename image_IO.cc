//LIC

#include "image_IO.h"

using namespace std;

//HEA

void _image_IO::read_image(string Name,cv::Mat &Image_in)
{
  #ifdef WINDOWS
  string Aux_string;
  for (int i=0;i<Name.size();i++){
    if (Name[i]=='/'){
      Aux_string.push_back('\\');
    }
    else Aux_string.push_back(Name[i]);
  }
  Name=Aux_string;
  #endif

  Image_in=cv::imread(Name,CV_LOAD_IMAGE_UNCHANGED);
  if( !Image_in.data ){ // check if the image has been loaded properly
    cout << "Error reading the image: " << Name << endl;
    exit(-1);
  }

  if (Image_in.channels()==4){
     cv::cvtColor(Image_in,Image_in,CV_BGRA2BGR);
  }

  if (Image_in.depth()!=CV_8U){
    cout << "Error in the depth of the image" << endl;
    exit(-1);
  }
}

//HEA

void _image_IO::write_image(string Name,cv::Mat &Image_out)
{
#ifdef WINDOWS
	cvSaveImage(Name.c_str(), &(IplImage(Image_out)));
#else
	imwrite(Name, Image_out);
#endif 
}
