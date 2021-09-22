#include "test.h"
#include "window.h"

using namespace std;

/**
* 170105
*
*@param
*@returns nothing
*/

_data::_data()
{
  Test_id=-1;
  Tone=-1;
  Shape=-1;
  Size=-1;
}


/**
* 170105
*
*@param
*@returns nothing
*/

_test::_test()
{
  Test_data[0].resize(9);
  Test_data[1].resize(9);
}


/**
* 170105
*
*@param
*@returns nothing
*/


bool compare(_data &Data1,_data &Data2){
  if (Data1.Test_id<Data2.Test_id) return true;
  return false;
}


void _test::save_data(int Test, int Latin_square[][9],int Slider_order[][11])
{
  std::ofstream File;
  time_t Now = time(0);
  string File_name;
  string String_now;
  string Parameters[]={"Tone", "Shape", "Size"};


  String_now=to_string(Now);

  // now the statistics
  File_name="results/statistics_"+QString("%1").arg(Personal_data.Test_id,2,10,QChar('0')).toStdString()+"_"+String_now+".txt";
  File.open (File_name);
  File << "Latin-square" << ";" << endl;
  for (unsigned int i=0;i<18;i++){
    for (unsigned int j=0;j<9;j++){
      File << std::to_string(Latin_square[i][j]) << "; ";
    }
    File << std::endl;
  }
  File << "Slider order" << ";" << endl;
  for (unsigned int i=0;i<54;i++){
    for (unsigned int j=0;j<11;j++){
      File << std::to_string(Slider_order[i][j]) << "; ";
    }
    File << std::endl;
  }

  File << "--------------------------------" << endl;
  File << "Test number:" << Personal_data.Test_id << ";" << endl;
  File << "Test Order" << endl;
  for (unsigned int i=0;i<9;i++){
    File << i << " " << Test_names[Latin_square[Personal_data.Test_id%18][i]] << endl;
  }
  File << "--------------------------------" << endl;
  File << "Tests" << endl;
  for (unsigned int i=0;i<54;i++){
    if (i==0) File << "************ Individual tests ************" << endl;
    if (i==27) File << "************ Group tests ************" << endl;

    File << "--" +Test_names[Latin_square[Personal_data.Test_id%18][(i/3)%9]] << endl;

    File << Parameters[i%3] << endl;

    File << "Slider values" << endl;
    for (unsigned int j=0;j<11;j++){
      File << std::to_string(Slider_order[i][j]) << "; ";
    }
    File << endl;
    File << "Selected value: ";
    switch (i%3){
    case 0: // tone
      File << std::to_string(Test_data[i/27][(i/3)%9].Tone) << ";" << endl;
      break;
    case 1: // shape
      File << std::to_string(Test_data[i/27][(i/3)%9].Shape) << ";" << endl;
      break;
    case 2: // size
      File << std::to_string(Test_data[i/27][(i/3)%9].Size) << ";" << endl;
      break;
    }
    File << endl;
  }
  File.close();


  // now the data is ordered

  std::sort(Test_data[0].begin(),Test_data[0].end(),compare);
  std::sort(Test_data[1].begin(),Test_data[1].end(),compare);

  File_name="results/results_"+QString("%1").arg(Personal_data.Test_id,2,10,QChar('0')).toStdString()+"_"+String_now+".txt";
  File.open (File_name);
//  File << Personal_data.First_name << ";" << endl;
//  File << Strings_test_id[Personal_data.Test_id].toStdString() << ";" << endl;
//  File << Strings_age[Personal_data.Age].toStdString() << ";" << endl;
//  File << Strings_sex[Personal_data.Sex].toStdString() << ";" << endl;
//  File << Strings_activity[Personal_data.Activity].toStdString() << ";" << endl;
//  File << Strings_stippling_relationship[Personal_data.Stippling_relationship].toStdString() << ";" << endl;

  File << std::to_string(Personal_data.Test_id) << ";" << endl;
  File << std::to_string(Personal_data.Age+18) << ";" << endl;
  File << std::to_string(Personal_data.Sex) << ";" << endl;
  File << std::to_string(Personal_data.Activity) << ";" << endl;
  File << std::to_string(Personal_data.Stippling_relationship) << ";" << endl;
  File << std::to_string(Personal_data.Creating_stippling) << ";" << endl;
  for (unsigned int i=0;i<2;i++){
    for (unsigned int j=0;j<Test_data[i].size();j++){
      File << std::to_string(Test_data[i][j].Test_id) << "; ";
      File << std::to_string(Test_data[i][j].Tone) << "; ";
      File << std::to_string(Test_data[i][j].Shape) << "; ";
      File << std::to_string(Test_data[i][j].Size) << "; ";
      File << std::endl;
    }
  }
  File.close();
}
