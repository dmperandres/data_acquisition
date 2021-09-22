//LIC

#ifndef IMAGEIO_H
#define IMAGEIO_H

#ifdef X64
  #include <opencv.hpp>
#else
  #include <imgproc/imgproc.hpp>
  #include <highgui/highgui.hpp>
#endif

#include <iostream>
#include <string>

//HEA

class _image_IO
{
public:
  std::string String;

  void read_image(std::string Name,cv::Mat &Image_in);
  void write_image(std::string Name, cv::Mat &Image_out);
};

#endif
