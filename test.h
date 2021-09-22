#ifndef TEST_H
#define TEST_H

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>
#include <ctime>


static std::vector<std::string> Test_names={
  {"tip_050_paper_cp"},
  {"tip_050_paper_mp"},
  {"tip_050_paper_hp"},
  {"tip_020_paper_cp"},
  {"tip_020_paper_mp"},
  {"tip_020_paper_hp"},
  {"tip_013_paper_cp"},
  {"tip_013_paper_mp"},
  {"tip_013_paper_hp"}
};



struct _personal_data {
  unsigned int Test_id=0;
  unsigned int Age=0;
  unsigned int Sex=0;
  unsigned int Activity=0;
  unsigned int Stippling_relationship=0;
  unsigned int Creating_stippling=0;

};

class _data
{
public:

  _data();

  int Test_id;
  int Tone;
  int Shape;
  int Size;
};


class _test
{
public:
  _test();
  void save_data(int Test, int Latin_square[][9], int Slider_order[][11]);

  _personal_data Personal_data;
  std::vector<_data> Test_data[2];
};

#endif // TEST_H
