//LIC

#ifndef GLWIDGET
#define GLWIDGET

#include <GL/glew.h>
#include <QOpenGLWidget>
#include <QGLFormat>
#include <QMatrix4x4>

#ifdef X64
  #include <opencv.hpp>
#else
  #include <core/core.hpp>
  #include <imgproc/imgproc.hpp>
  #include <highgui/highgui.hpp>
#endif

#include <math.h>
#include <string>
#include <map>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <vector>
#include <sys/timeb.h>
#include <QMessageBox>
//#include <QSvgGenerator>
//#include <QPainter>

#include <sys/stat.h>

#include <ctime>
//#include <ratio>
#include <chrono>

#include "window.h"
//#include "images_tab.h"
//#include "filters_tab.h"
#include "shaders.h"
#include "image_IO.h"

// filters
#include "filter.h"
#include "filter_halftoning_ost.h"
#include "filter_stippling_ebg.h"
#include "filter_retinex_c.h"
#include "filter_border_mask.h"

//#include "test.h"

#include "upload.h"

#define TEST


#ifdef ENGLISH
const QString String_start("Start");
const QString String_first_name("First name:");
const QString String_last_name("Last name:");
const QString String_test_id("ID:");
const QString String_age("Age:");
const QString String_sex("Sex:");
const QString String_activity("Activity:");
const QString String_stippling_relationship("Relation with stippling:");
const QString String_creating_stippling("Relation with creating stippling:");
const QString String_fault("Please, input data in all the fields");

const QString String_bad_id("The ID does not correspond to the image package. Please, ask the supervisor");

//const vector<QString> Strings_test_id={"None","0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","31","32","33","34","35","36","37","38","39","40","41","42","43","44","45","46","47","48","49","50","51","52","53","54","55","56","57","58","59","60","61","62","63","64","65","66","67","68","69","70","71","72","73","74","75","76","77","78","79"};

const vector<QString> Strings_age={"None","18","19","20","21","22","23","24","25","26","27","28","29","30","31","32","33","34","35","36","37","38","39","40","41","42","43","44","45","46","47","48","49","50","51","52","53","54","55","56","57","58","59","60","61","62","63","64","65","66","67","68","69","70"};
const vector<QString> Strings_sex={"None","Female","Male"};
const vector<QString> Strings_activity={"None","Student","Teacher","Health","Lawyer","Transport","Other"};
const vector<QString> Strings_stippling_relationship={"----","I have never seen stippling before","I have seen stippling a few times before in my life","I look at stippling images about 1-3 times per year","I look at stippling images about 1-3 times per month","I look at stippling images about 1-3 times per week","I look at stippling images daily"};

const vector<QString> Strings_creating_stippling={"----","I have never created stipple art before","I have created stipple art a few times before in my life","I create stipple art about 1-3 times per year","I create stipple art about 1-3 times per month","I create stipple art about 1-3 times per week","I am a stipple artist"};

const QString String_warning("<b>Warning!</b>");
const QString String_message_end("<p>The exercise is finished</p><p>The data will be saved<p><p><b>Thank you for your help!</b></p>");

const QString String_message_next_test("<p>Passing to the next test</p>");

//const QString String_up_window={"Stippled field with scanned dots"};

#else
const QString String_warning("<b>¡Atención!</b>");
const QString String_message_end("<p>El ejercicio ha terminado</p><p>Los datos serán guardados<p><p><b>¡Muchas gracias por su colaboración!</b></p>");
const QString String_message_next_test("<p>Se va a pasar al siguiente test</p>");

const QString String_start("Iniciar test");
const QString String_test_id("ID:");
const QString String_age("Edad:");
const QString String_sex("Sexo:");
const QString String_activity("Actividad:");
const QString String_stippling_relationship("Relacion con el punteado:");
const QString String_creating_stippling("Relacion con la creación de punteado:");
const QString String_fault("Por favor, introduzca los datos en todos los campos");

const QString String_bad_id("El ID no corresponde al paquete de imágenes. Por favor, consúltelo con el supervisor");

//const vector<QString> Strings_test_id={"None","0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","31","32","33","34","35","36","37","38","39","40","41","42","43","44","45","46","47","48","49","50","51","52","53","54","55","56","57","58","59","60","61","62","63","64","65","66","67","68","69","70","71","72","73","74","75","76","77","78","79"};

const vector<QString> Strings_age={"Nulo","18","19","20","21","22","23","24","25","26","27","28","29","30","31","32","33","34","35","36","37","38","39","40","41","42","43","44","45","46","47","48","49","50","51","52","53","54","55","56","57","58","59","60","61","62","63","64","65","66","67","68","69","70"};
const vector<QString> Strings_sex={"Nulo","Mujer","Hombre"};
const vector<QString> Strings_activity={"Nulo","Estudiante","Profesor","Medico","Abogado","Transportista","Otro"};
const vector<QString> Strings_stippling_relationship={"----","Nunca había visto imágenes de punteado","Habia visto imágenes de punteado unas pocas veces en mi vida","Veo imágenes de punteado 1-3 veces al año","Veo imágenes de punteado 1-3 veces al mes","Veo imágenes de punteado 1-3 veces a la semana","Veo imágenes de punteado diariamente"};

const vector<QString> Strings_creating_stippling={"----","Nunca he creado imágenes de punteado","He creado algunas imágenes de punteado en mi vida","Creo entre 1 y 3 imágenes de punteado al año","Creo entre 1 y 3 imágenes de punteado al mes","Creo entre 1 y 3 imágenes de punteado a la semana","Soy un artista del punteado"};

#endif





namespace _f_gl_widget_ns
{

  const int MAX_IMAGES=10;

  const int NUM_PARTICIPANTS=160;

  struct _personal_data {
    unsigned int Test_id=0;
    unsigned int Age=0;
    unsigned int Sex=0;
    unsigned int Activity=0;
    unsigned int Stippling_relationship=0;
    unsigned int Creating_stippling=0;

  };


  struct _data_filters{
    float Color_restoration=1.0;
    int Small_gaussian=3;
    int Medium_gaussian=127;
    int Big_gaussian=255;
    float Contrast=1.0;
    int Bright=0;
  };

  const string Dir_data="data/";
  const string Dir_results="results/";
  const string Url="http://calipso.ugr.es/ig/load.php";

  typedef enum {TYPE_FILTER,INPUT_IMAGE_0,OUTPUT_IMAGE_0,INPUT_IMAGE_1} _info_filter;

//  typedef enum {FILTER_RETINEX_C,FILTER_BORDER_MASK,FILTER_HALFTONING_OSTROMOUKHOV,FILTER_STIPPLING_EBG,FILTER_LAST} _filter_name;
  typedef enum {ORIGINAL,FILTER_RETINEX_C,FILTER_HALFTONING_OSTROMOUKHOV,FILTER_STIPPLING_EBG,FILTER_LAST} _filter_name;

  const int NUM_TABS=4;

  const string FILE_NAME_PAPER={"./papers/default.png"};

  typedef enum {RESOLUTION_300PPI,RESOLUTION_600PPI,RESOLUTION_1200PPI,LAST_RESOLUTION} _resolution;

  const int VEC_RESOLUTION[3]={300,600,1200};
  const int VEC_RESOLUTION_FACTOR[3]={1,2,4};

  const unsigned int MIN_DOT_SIZE=4;
  const unsigned int MAX_DOT_SIZE=8;

  //parameter1
  const std::string String_group_box_parameter1("Color restoration variance");
#ifdef TEST
  const std::string String_label_parameter1_min("Min");
  const std::string String_label_parameter1_max("Max");
#else
  const std::string String_label_parameter1_min("0");
  const std::string String_label_parameter1_max("1");
#endif
  const std::string String_label_parameter1_tooltip("Display the value of the number of kernels");
  const int Parameter1_min_value=0;
  const int Parameter1_max_value=255;
  const int Parameter1_single_step=1;
  const int Parameter1_page_step=5;
  const int Parameter1_tick_interval=15;
  const bool Parameter1_set_tracking=false;
  const std::string String_parameter1_slider_tooltip("Controls the color restoration variation");

  typedef enum {FILTER_UNIFORM,FILTER_LOW,FILTER_HIGH} _filter_mode;

  //parameter2
  // size of the small gaussian
  const std::string String_group_box_parameter2("Size of small gaussian");
#ifdef TEST
  const std::string String_label_parameter2_min("Min");
  const std::string String_label_parameter2_max("Max");
#else
  const std::string String_label_parameter2_min("3  ");
  const std::string String_label_parameter2_max("251");
#endif
  const std::string String_label_parameter2_tooltip("Display the size of the small gaussian");
  const int Parameter2_min_value=3;
  const int Parameter2_max_value=251;
  const int Parameter2_single_step=1;
  const int Parameter2_page_step=10;
  const int Parameter2_tick_interval=15;
  const bool Parameter2_set_tracking=false;
  const std::string String_parameter2_slider_tooltip("Controls the size of the small gaussian");

  //parameter 3
  // size of the medium gaussian
  const std::string String_group_box_parameter3("Size of medium gaussian");
#ifdef TEST
  const std::string String_label_parameter3_min("Min");
  const std::string String_label_parameter3_max("Max");
#else
  const std::string String_label_parameter3_min("5  ");
  const std::string String_label_parameter3_max("253");
#endif
  const std::string String_label_parameter3_tooltip("Display the size of the medium gaussian");
  const int Parameter3_min_value=5;
  const int Parameter3_max_value=253;
  const int Parameter3_single_step=1;
  const int Parameter3_page_step=10;
  const int Parameter3_tick_interval=15;
  const bool Parameter3_set_tracking=false;
  const std::string String_parameter3_slider_tooltip("Controls the size of the medium gaussian");

  //parameter 4
  // size of the big gaussian
  const std::string String_group_box_parameter4("Size of big gaussian");
#ifdef TEST
  const std::string String_label_parameter4_min("Min");
  const std::string String_label_parameter4_max("Max");
#else
  const std::string String_label_parameter4_min("7  ");
  const std::string String_label_parameter4_max("255");
#endif
  const std::string String_label_parameter4_tooltip("Display the size of the big gaussian");
  const int Parameter4_min_value=7;
  const int Parameter4_max_value=255;
  const int Parameter4_single_step=1;
  const int Parameter4_page_step=10;
  const int Parameter4_tick_interval=15;
  const bool Parameter4_set_tracking=false;
  const std::string String_parameter4_slider_tooltip("Controls the size of the big gaussian");

  // parameter 5
  const std::string String_group_box_parameter5("Contrast");
#ifdef TEST
  const std::string String_label_parameter5_min("Min");
  const std::string String_label_parameter5_max("Max");
#else
  const std::string String_label_parameter5_min("0.1");
  const std::string String_label_parameter5_max("5  ");
#endif
  const std::string String_label_parameter5_tooltip("Display the value of contrast value");
  const int Parameter5_min_value=1;
  const int Parameter5_max_value=250;
  const int Parameter5_single_step=1;
  const int Parameter5_page_step=5;
  const int Parameter5_tick_interval=10;
  const bool Parameter5_set_tracking=false;
  const std::string String_parameter5_tooltip("Controls the appearance of contrast");
  // warning: the capture value must be adjusted to a odd value
  const int Parameter5_divisor=50;

  // parameter 6
  const std::string String_group_box_parameter6("Bright");
#ifdef TEST
  const std::string String_label_parameter6_min("Min");
  const std::string String_label_parameter6_max("Max");
#else
  const std::string String_label_parameter6_min("-255 ");
  const std::string String_label_parameter6_max("255  ");
#endif
  const std::string String_label_parameter6_tooltip("Display the value of bright");
  const int Parameter6_min_value=-255;
  const int Parameter6_max_value=255;
  const int Parameter6_single_step=10;
  const int Parameter6_page_step=25;
  const int Parameter6_tick_interval=25;
  const bool Parameter6_set_tracking=false;
  const std::string String_parameter6_slider_tooltip("Controls the appearance of bright");

  // parameter 7
#ifdef ENGLISH
  const std::string String_group_box_parameter7("Images");
  const QString String_button_original("Original");
  const QString String_button_stippling("Stippling");
#else
  const std::string String_group_box_parameter7("Imágenes");
  const QString String_button_original("Original");
  const QString String_button_stippling("Punteado");
#endif

#ifdef ENGLISH
  const QString String_next_step("Next image");
  const QString String_previous_step("Previous image");
  const QString String_warning_box("<b>Warning!</b>");
  const QString String_message("Are you sure that the task is finished?");
#else
  const QString String_next_step("Siguiente imagen");
  const QString String_previous_step("Imagen previa");
  const QString String_warning_box("<b>¡Atención!</b>");
  const QString String_message("¿Está seguro de que ha terminado?");
#endif

  // Default values
//  const int CONTRAST_DEFAULT=1;
//  const int BRIGHT_DEFAULT=0;
  const int RETINEX_MAX_COLOR_RESTORATION_VARIANCE=1;

}

//HEA

class _gl_widget : public QOpenGLWidget
{
Q_OBJECT

public:

  shared_ptr<cv::Mat> Input_image;
  vector<shared_ptr<cv::Mat>> Vec_images;
  vector<shared_ptr<_filter>> Vec_filters;


        _gl_widget(_window *Window1,QWidget *Parent = nullptr);
        ~_gl_widget() override;
  void  clear_data();
  void  update_images();
  void  read_image(std::string File_name);
  void  save_image(std::string File_name);

  void  read_dots();
  void  refresh_image();
  void  refresh_image(cv::Mat *Image);

  // change tab
  void change_tab(int Index1);

  // activate the dots packets
//  void selected_dots_index(int Selected_dots_index1){Selected_dots_index=Selected_dots_index1;}

  void create_widget_parameters();

  void set_parameter1(float Value);
  void set_parameter2(int Value);
  void set_parameter3(int Value);
  void set_parameter4(int Value);
  void set_parameter5(float Value);
  void set_parameter6(int Value);

  void update_filters(unsigned int Pos);

  void next_image();
  void previous_image();

  void input_data();
  void message_fault();

  void save_data(string File_name, string File_dir);

  // Tabs
//  _filters_tab *Filters_tab;

  QWidget *Widget_parameters;

protected slots:
  void set_parameter1_slot(int Value);
  void set_parameter2_slot(int Value);
  void set_parameter3_slot(int Value);
  void set_parameter4_slot(int Value);
  void set_parameter5_slot(int Value1);
  void set_parameter6_slot(int Value);
  void button_next_step_slot();
  void button_previous_step_slot();

  void button_original_slot();
  void button_stippling_slot();

  void button_ok_slot();


protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int width, int height) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
//  void wheelEvent(QWheelEvent *event) override;

  void draw_objects();
  void refresh();

  void adjust_image_sizes(int &Width1, int &Height1);

  bool check_file(int Pos);

private:
  _window *Window;

  GLuint Program1;
  GLuint Texture1;
  GLuint VAO1;
  GLuint VBO1_1;
  GLuint VBO1_2;

//  vector<QImage> Vec_images;

  std::vector<_vertex3f> Vertices;
  std::vector<_vertex2f> Tex_coordinates;

  float Window_width;
  float Window_height;

  float Scale_drawing_mode;
  _vertex2f Translation;

  // translation of the window
  bool Change_pos;
  QTime Click_time;

  int Initial_position_X;
  int Initial_position_Y;

  bool Ready;
  bool Effect_ready;
  bool Image_loaded;

  cv::Mat Frame_buffer;

  //
//  std::string Selected_image;
//  std::string Selected_filter;
//  int Selected_filter_index;

  // index of the image
  int Selected_image_index; // for images
//  int Selected_dots_index; // for dots
//  int Selected_dot_design_index; // for dot design

//  int Selected_tab;
//  int Selected_index; // the general index for mouse
  //
  int Max_texture_size;

//  shared_ptr<cv::Mat> Image_cv_gray;
//  shared_ptr<cv::Mat> Image_cv_gray1;
//  shared_ptr<cv::Mat> Image_cv_color;
//  shared_ptr<cv::Mat> Frame_buffer;
//  shared_ptr<cv::Mat> Original_image;

  // Images of dots for stippling (no ppi level)
  vector<vector<cv::Mat *>> Dots;

//  bool Painting;
//  bool Paper_loaded;
//  bool Active_dots_tabs;
//  bool Drawn;
//  bool Image_changed;

  QGroupBox *Group_box_parameter1;
  QGroupBox *Group_box_parameter2;
  QGroupBox *Group_box_parameter3;
  QGroupBox *Group_box_parameter4;
  QGroupBox *Group_box_parameter5;
  QGroupBox *Group_box_parameter6;
  QGroupBox *Group_box_parameter7;

  // Color_restoration_variance
  QSlider *Slider_parameter1;
  QLineEdit *Line_edit_parameter1;

  // small gaussian size
  QSlider *Slider_parameter2;
  QLineEdit *Line_edit_parameter2;

  // medium gaussian size
  QSlider *Slider_parameter3;
  QLineEdit *Line_edit_parameter3;

  // big gaussian size
  QSlider *Slider_parameter4;
  QLineEdit *Line_edit_parameter4;

  // contrast
  QSlider *Slider_parameter5;
  QLineEdit *Line_edit_parameter5;

  // bright
  QSlider *Slider_parameter6;
  QLineEdit *Line_edit_parameter6;

  QPushButton *Button_next_step;
  QPushButton *Button_previous_step;

  QPushButton *Button_original;
  QPushButton *Button_stippling;


  int Image_number=-1;

  std::vector<_f_gl_widget_ns::_data_filters> Vec_data_filters;

  //
  //
  QDialog *Dialogbox;
  int Return_dialogbox;

  QComboBox *Combo_test_id;
  QComboBox *Combo_age;
  QComboBox *Combo_sex;
  QComboBox *Combo_activity;
  QComboBox *Combo_stippling_relationship;
  QComboBox *Combo_creating_stippling;

  QLabel *Label1;
  QLabel *Label2;

  _f_gl_widget_ns::_personal_data Personal_data;

  float Factor_size_image=1;
  float Factor_size_image_original=1;

  //
  std::chrono::steady_clock::time_point Start_time;
  std::chrono::steady_clock::time_point Stop_time;

  vector<float> Vec_elapsed_time;
  vector<vector<int>> Vec_count_events;
  vector<string> Vec_file_names;

  vector<QString> Strings_test_id;

};


#endif
