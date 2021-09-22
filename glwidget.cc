//LIC

#include "glwidget.h"

using namespace _f_gl_widget_ns;

extern std::ofstream Output_stream;
extern bool DEBUG;
//HEA

//int _gl_widget::Counter_draw=0;

_gl_widget::_gl_widget(_window *Window1, QWidget *Parent)
: QOpenGLWidget(Parent)
{
  Window=Window1;

  QGLFormat GLformat;
  GLformat.setSwapInterval(0);
  QGLFormat::setDefaultFormat(GLformat);

  // controls if an image is loaded
  Image_loaded=false;

  // scale and tranlation values for each image
  // it must be initialized
  Scale_drawing_mode=1.0f;
  Translation=_vertex2f(0,0);

  Selected_image_index=4;

  // it is necessary the ppi level!!!!
  // for the 5 textures packets of dots
  Dots.resize(3); // 3 ppi level
  for (unsigned int i=0;i<3;i++){
    Dots[i].resize(9);
    for (unsigned int j=4;j<=8;j++){
      Dots[i][j]=new cv::Mat;
    }
  }

  #ifdef DEBUG
  cout << "creadas las matrices de puntos" << endl;
  #endif

  read_dots();

  #ifdef DEBUG
  cout << "leidos los puntos" << endl;
  #endif

  Vec_images.resize(FILTER_LAST);
  Vec_filters.resize(FILTER_LAST);

  Input_image=make_shared<cv::Mat>();
  for (unsigned int i=0;i<Vec_images.size();i++) Vec_images[i]=make_shared<cv::Mat>();

  Vec_filters[FILTER_RETINEX_C]=make_shared<_filter_retinex_c>(this);
//  Vec_filters[FILTER_BORDER_MASK]=make_shared<_filter_border_mask>();
  Vec_filters[FILTER_HALFTONING_OSTROMOUKHOV]=make_shared<_filter_halftoning_ost>();
  Vec_filters[FILTER_STIPPLING_EBG]=make_shared<_filter_stippling_ebg>();

//  auto Ptr=dynamic_cast<_filter_stippling_ebg *>(Vec_filters[3].get());
//  Ptr->set_dots_images(&Dots);

  dynamic_cast<_filter_stippling_ebg *>(Vec_filters[FILTER_STIPPLING_EBG].get())->set_dots_images(&Dots);

//  Vec_filters[FILTER_RETINEX_C]->set_images(Input_image.get(),Vec_images[FILTER_RETINEX_C].get(),nullptr);
  for (unsigned int i=1;i<Vec_filters.size();i++){
    Vec_filters[i]->set_images(Vec_images[i-1].get(),Vec_images[i].get(),nullptr);
  }

  Vec_images[FILTER_STIPPLING_EBG]->create(256,256,CV_8U);
  Vec_images[FILTER_STIPPLING_EBG]->setTo(255);


  Vec_data_filters.resize(_f_gl_widget_ns::MAX_IMAGES);

  create_widget_parameters();

  Vec_elapsed_time.resize(FILTER_LAST);
  for (unsigned int i=0;i<Vec_elapsed_time.size();i++) Vec_elapsed_time[i]=0;

  Vec_count_events.resize(MAX_IMAGES);
  for (unsigned int i=0;i<Vec_count_events.size();i++){
    Vec_count_events[i].resize(6);
    for (unsigned int j=0;j<6;j++) Vec_count_events[i][j]=0;
  }

  Vec_file_names={
    "images/repeated/image000.png",
    "images/repeated/image001.png",
    "images/repeated/image002.png",
    "images/repeated/image003.png",
    "images/repeated/image004.png",
    "images/",
    "images/",
    "images/",
    "images/",
    "images/",
  };

  Strings_test_id.resize(_f_gl_widget_ns::NUM_PARTICIPANTS+1);
#ifdef ENGLISH
  Strings_test_id[0]="None";
#else
  Strings_test_id[0]="Nulo";
#endif
  char Aux[10];

  for (unsigned int i=0;i<_f_gl_widget_ns::NUM_PARTICIPANTS;i++){
    sprintf(Aux,"%03d",i);
    Strings_test_id[i+1]=Aux;
  }

}

//HEA

_gl_widget::~_gl_widget()
{

  #ifdef DEFINE_FILTER_EXAMPLE_BASED_STIPPLING
  // delete images of dots
  #ifdef FULL
  for (int i=0;i<_f_stippling_ebg::LAST_RESOLUTION;i++){
  #else
  for (int i=_f_stippling_ebg::RESOLUTION_300PPI;i<_f_stippling_ebg::RESOLUTION_600PPI;i++){
  #endif
    for (int j=_f_stippling_ebg::MIN_DOT_SIZE;j<=_f_stippling_ebg::MAX_DOT_SIZE;j++){
      delete Dots[i][j];
    }
  }
  #endif

  clear_data();
}

//HEA

void _gl_widget::clear_data()
{
//  Filters.clear();
//  Images.clear();
//  Graph.clear();
//  Pipeline.clear();
}

//HEA

void  _gl_widget::read_image(string File_name)
{

  if (Image_loaded) Vec_images[ORIGINAL]->release();

  // the original image is read
  _image_IO Image_io;

  // hide the widget meanwhile there are changes
//  hide();

  Image_io.read_image(File_name,*Vec_images[ORIGINAL]);

  if (DEBUG){
    cout << "Size of loaded image: width=" << Vec_images[ORIGINAL]->cols << " height=" << Vec_images[ORIGINAL]->rows << endl;
  }

  if (Vec_images[ORIGINAL]->channels()==1){
    QMessageBox MsgBox;
    MsgBox.setText("The input image must be in color");
    MsgBox.exec();
    exit(-1);
  }

  int Width_aux=Vec_images[ORIGINAL]->cols;
  int Height_aux=Vec_images[ORIGINAL]->rows;

  adjust_image_sizes(Width_aux,Height_aux);

  if (Width_aux!=Vec_images[ORIGINAL]->cols || Height_aux!=Vec_images[ORIGINAL]->rows){
      QMessageBox MsgBox;
      MsgBox.setText("The size of the input image is not even. Adjusted to the nearest value (cubic interpolation)");
      MsgBox.exec();
    cv::resize(*Vec_images[ORIGINAL],*Vec_images[ORIGINAL],cv::Size(Width_aux,Height_aux),0,0,CV_INTER_CUBIC);
  }

  // flip the original image to adapt it to OpenGL
  cv::flip(*Vec_images[ORIGINAL],*Vec_images[ORIGINAL],0);

//  show();

  // adjust the size of the images depending on the input image
  for (unsigned int i=1;i<Vec_images.size()-1;i++){
    Vec_images[i]->release();
    Vec_images[i]->create(Input_image->rows,Input_image->cols,CV_8U);
    Vec_images[i]->setTo(255);
  }

  // the last image depends on the scaling factor
  float Scaling_factor=Vec_filters[FILTER_STIPPLING_EBG]->scaling_factor();
  Width_aux=int(float(Vec_images[ORIGINAL]->cols)*Scaling_factor);
  Height_aux=int(float(Vec_images[ORIGINAL]->rows)*Scaling_factor);

  adjust_image_sizes(Width_aux,Height_aux);
  Vec_images[FILTER_STIPPLING_EBG]->release();
  Vec_images[FILTER_STIPPLING_EBG]->create(Height_aux,Width_aux,CV_8U);

  for (unsigned int i=1;i<Vec_filters.size();i++) Vec_filters[i]->input_changed(true);

  Image_loaded=true;

  Selected_image_index=FILTER_STIPPLING_EBG;

  float Aux=float(Vec_images[ORIGINAL]->cols*Window_height)/float(Vec_images[ORIGINAL]->rows);
  Factor_size_image=float(Width_aux)/Aux;

  Factor_size_image_original=float(Vec_images[ORIGINAL]->rows)/float(Window_height);
//  update_filters(FILTER_RETINEX_C);
}

//HEA

void  _gl_widget::save_image(string File_name)
{
  _image_IO Image_io;
  cv::Mat Aux_image;

//  Aux_image=Images.get_data(Selected_image)->clone();

  cv::flip(Aux_image,Aux_image,0);
  Image_io.write_image(File_name,Aux_image);
}

//HEA

void _gl_widget::read_dots()
{
  string File_name;
  _image_IO Image_io;
  char Name[200];

  QDir Directory;
  File_name=Directory.currentPath().toStdString();

//  printf("Reading dots files\n");
  for (unsigned int i=_f_gl_widget_ns::RESOLUTION_300PPI;i<_f_gl_widget_ns::LAST_RESOLUTION;i++){
    for (unsigned int j=_f_gl_widget_ns::MIN_DOT_SIZE;j<=_f_gl_widget_ns::MAX_DOT_SIZE;j++){
      // to resolve the problem with absolute paths
      sprintf(Name,"/dots/dots_%d_%d.pgm",j,_f_gl_widget_ns::VEC_RESOLUTION[i]);
      string File_name_aux=File_name+string(Name);
      Image_io.read_image(File_name_aux,*Dots[i][j]);
      cv::flip(*Dots[i][j],*Dots[i][j],0);
      if (Dots[i][j]->channels()!=1){
        cout << "Error: number of channels incorrect in dots files" << endl;
        exit(-1);
      }
    }
  }
}

//HEA

void _gl_widget::mousePressEvent(QMouseEvent *event)
{
  if (event->buttons() & Qt::LeftButton) {
    Change_pos=true;
    Initial_position_X=event->x();
    Initial_position_Y=event->y();
  }
}

//HEA

void _gl_widget::mouseReleaseEvent(QMouseEvent *event)
{
  Q_UNUSED(event)
  Change_pos=false;
}

//HEA

void _gl_widget::mouseMoveEvent(QMouseEvent *event)
{
  int Last_position_X=event->x();
  int Last_position_Y=event->y();

  if (Change_pos==true){
    Translation.x+=Last_position_X-Initial_position_X;
    Translation.y+=Initial_position_Y-Last_position_Y;
    Initial_position_X=Last_position_X;
    Initial_position_Y=Last_position_Y;
    }

//  Drawn=false;

  update();
}

//HEA

//void _gl_widget::wheelEvent(QWheelEvent *event)
//{
//  int Step=event->delta()/120;

//  if (Step<0) Scale_drawing_mode/=1.2f;
//  else Scale_drawing_mode*=1.2f;

//  update();
//}

//HEA

void _gl_widget::mouseDoubleClickEvent(QMouseEvent *Event)
{
  if (Event->buttons() & Qt::LeftButton) {
    Translation.x=0;
    Translation.y=0;
    update();
  }
  if (Event->buttons() & Qt::RightButton) {
    Scale_drawing_mode=1.0;
    update();
  }
}

//HEA

void _gl_widget::initializeGL()
{
  const GLubyte* strm;
  GLenum err;

  if (DEBUG) cout << "initializeGL" << endl;

  //Glew init:
  err = glewInit();
  if (GLEW_OK != err){
    // Problem: glewInit failed, something is seriously wrong.
    if (DEBUG) cout << "glewInit failed" << endl;
    if (DEBUG) Output_stream.flush();

    std::cerr << "Error: " << glewGetErrorString(err) << "\n";
    QMessageBox::warning(this, tr("Warning"),tr("glewInit failed"));
    exit (-1);
  }

  std::cerr << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << "\n";
  if (!glewGetExtension("GL_ARB_texture_non_power_of_two")){
    if (DEBUG) cout << "texture_non_power_of_two may not be supported" << endl;
    if (DEBUG) Output_stream.flush();

    std::cerr << "Warning: ARB_texture_non_power_of_two may not be supported" << "\n";
    QMessageBox::warning(this, tr("Warning"),tr("ARB_texture_non_power_of_two may not be supported"));
    exit (-1);
  }

  #ifdef DEBUG
  cout << "se ha cargado glew" << endl;
  #endif

  if (DEBUG) cout << "GLEW loaded" << endl;

  strm = glGetString(GL_VENDOR);
  std::cerr << "Vendor: " << strm << "\n";
  if (DEBUG) cout << "Vendor:" << strm << endl;

  strm = glGetString(GL_RENDERER);
  std::cerr << "Renderer: " << strm << "\n";
  if (DEBUG) cout << "Renderer:" << strm << endl;

  strm = glGetString(GL_VERSION);
  std::cerr << "OpenGL version: " << strm << "\n";
  if (DEBUG) cout << "OpenGL version:" << strm << endl;

  strm = glGetString(GL_SHADING_LANGUAGE_VERSION);
  std::cerr << "GLSL version: " << strm << "\n";
  if (DEBUG) cout << "GLSL version:" << strm << endl;

  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &Max_texture_size);

  if (DEBUG) cout << "Max texture size:" << Max_texture_size << endl;

  if (DEBUG) Output_stream.flush();

  Change_pos=false;

  _shaders Shader;

  // initial image: a gray square
  int Width1=512;
  int Height1=512;

  QImage Image1(Width1,Height1,QImage::Format_RGB888);
  Image1.fill(QColor(240,0,0));


  static const GLchar *P0_vs[]={
    "#version 450 core                                               \n"
    "layout (location=0) uniform mat4 matrix;                        \n"
    "layout (location=1) in vec3 Vertex;                             \n"
    "layout (location=2) in vec2 tex_coord;                          \n"
    "                                                                \n"
    "out VS_OUT                                                      \n"
    "{                                                               \n"
    "  vec2 tex_coord;                                               \n"
    "} vs_out;                                                       \n"
    "                                                                \n"
    "void main(void)                                                 \n"
    "{                                                               \n"
    "   vs_out.tex_coord=tex_coord;                                  \n"
    "   gl_Position = matrix*vec4(Vertex,1.0);                       \n"
    "}                                                               \n"
  };

  static const GLchar * P0_fs[]={
    "#version 450 core                                              \n"
    "uniform sampler2D texture_image;                               \n"
    "                                                               \n"
    "in VS_OUT                                                      \n"
    "{                                                              \n"
    "  vec2 tex_coord;                                              \n"
    "} fs_out;                                                      \n"
    "                                                               \n"
    "out vec4 frag_color;                                           \n"
    "                                                               \n"
    "void main(void)                                                \n"
    "{                                                              \n"
    "  frag_color = texture(texture_image,fs_out.tex_coord);        \n"
    "}                                                              \n"
  };



  Program1=Shader.load_shaders(P0_vs,P0_fs);
  if (Program1==0){
    exit(-1);
  }
  // adjust the sizes

  glCreateTextures(GL_TEXTURE_2D,1,&Texture1);
  glTextureStorage2D(Texture1,1,GL_RGB8,Width1,Height1);

  glBindTexture(GL_TEXTURE_2D,Texture1);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // fill with data
  glTextureSubImage2D(Texture1,0,0,0,Width1,Height1,GL_RGB,GL_UNSIGNED_BYTE,Image1.constBits());

  Vertices.resize(6);
  Tex_coordinates.resize(6);

  Vertices[0]=_vertex3f(-Width1/2, -Height1/2, 0.);
  Vertices[1]=_vertex3f(Width1/2, -Height1/2, 0.);
  Vertices[2]=_vertex3f(-Width1/2, Height1/2, 0.);
  Vertices[3]=_vertex3f(Width1/2, -Height1/2, 0.);
  Vertices[4]=_vertex3f(Width1/2, Height1/2, 0.);
  Vertices[5]=_vertex3f(-Width1/2, Height1/2, 0.);

  Tex_coordinates[0]=_vertex2f(0.0,0.0);
  Tex_coordinates[1]=_vertex2f(1.0,0.0);
  Tex_coordinates[2]=_vertex2f(0.0,1.0);
  Tex_coordinates[3]=_vertex2f(1.0,0.0);
  Tex_coordinates[4]=_vertex2f(1.0,1.0);
  Tex_coordinates[5]=_vertex2f(0.0,1.0);

  glCreateVertexArrays(1,&VAO1);
  glBindVertexArray(VAO1);

  // vertices coordinates
  // create
  glCreateBuffers(1,&VBO1_1);
  // reserve espace (and initialice; if use NULL there is not initialization)
  glNamedBufferStorage(VBO1_1,Vertices.size()*3*sizeof(float),&Vertices[0],GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
  // bind to VAO
  glVertexArrayVertexBuffer(VAO1,0,VBO1_1,0,3*sizeof(float));
  // state the format of the data
  glVertexArrayAttribFormat(VAO1,1,3,GL_FLOAT,GL_FALSE,0);
  // state what vertex buffer binding ot use for this attribute
  glVertexArrayAttribBinding(VAO1,1,0);
  // enable the attribute
  glEnableVertexAttribArray(1);

  // texture coordinates
  glCreateBuffers(1,&VBO1_2);
  // reserve espace (and initialice; if use NULL there is not initialization)
  glNamedBufferStorage(VBO1_2,Tex_coordinates.size()*2*sizeof(float),&Tex_coordinates[0],GL_DYNAMIC_STORAGE_BIT);
  // bind to VAO
  glVertexArrayVertexBuffer(VAO1,1,VBO1_2,0,2*sizeof(float));
  // state the format of the data
  glVertexArrayAttribFormat(VAO1,2,2,GL_FLOAT,GL_FALSE,0);
  // state what vertex buffer binding ot use for this attribute
  glVertexArrayAttribBinding(VAO1,2,1);
  // enable the attribute
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);

//  refresh_image();
}

//HEA

void _gl_widget::paintGL()
{
  glViewport(0,0,width(),height());
  draw_objects();
}

//HEA

void _gl_widget::resizeGL(int width, int height)
{
  Window_width=width;
  Window_height=height;

  glViewport(0,0,width,height);
}

//HEA

void _gl_widget::draw_objects()
{

  QMatrix4x4 Model_view;

  if (Selected_image_index==FILTER_STIPPLING_EBG){
    Model_view.ortho(-Factor_size_image*Window_width*Scale_drawing_mode/2,Factor_size_image*Window_width*Scale_drawing_mode/2,Factor_size_image*-Window_height*Scale_drawing_mode/2,Factor_size_image*Window_height*Scale_drawing_mode/2,-100,100);
  }
  else{
    Model_view.ortho(-Factor_size_image_original*Window_width*Scale_drawing_mode/2,Factor_size_image_original*Window_width*Scale_drawing_mode/2,Factor_size_image_original*-Window_height*Scale_drawing_mode/2,Factor_size_image_original*Window_height*Scale_drawing_mode/2,-100,100);
  }


  glViewport(0,0,int(Window_width),int(Window_height));
  glClearColor(0.8f,0.8f,0.8f,1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  //  glPointSize(10);
  glUseProgram(Program1);
  glBindVertexArray(VAO1);

  glBindTexture(GL_TEXTURE_2D,Texture1);

  glUniformMatrix4fv(0,1,GL_FALSE,Model_view.data());

  glDrawArrays(GL_TRIANGLES,0,6);

  glBindVertexArray(0);
  glUseProgram(0);
}

//HEA

void _gl_widget::refresh_image()
{
  if (Image_loaded){
    if (Vec_images[Selected_image_index]->channels()==1){
      cv::cvtColor(*Vec_images[Selected_image_index].get(),Frame_buffer,CV_GRAY2BGR,3);
    }
    else{
      Vec_images[Selected_image_index]->copyTo(Frame_buffer);
    }

    int Width1=Frame_buffer.cols;
    int Height1=Frame_buffer.rows;

    glDeleteTextures(1,&Texture1);

    glCreateTextures(GL_TEXTURE_2D,1,&Texture1);
    glTextureStorage2D(Texture1,1,GL_RGB8,Width1,Height1);

    glBindTexture(GL_TEXTURE_2D,Texture1);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // fill with data
    glTextureSubImage2D(Texture1,0,0,0,Width1,Height1,GL_BGR,GL_UNSIGNED_BYTE,Frame_buffer.data);

    Vertices[0]=_vertex3f(-Width1/2, -Height1/2, 0.);
    Vertices[1]=_vertex3f(Width1/2, -Height1/2, 0.);
    Vertices[2]=_vertex3f(-Width1/2, Height1/2, 0.);
    Vertices[3]=_vertex3f(Width1/2, -Height1/2, 0.);
    Vertices[4]=_vertex3f(Width1/2, Height1/2, 0.);
    Vertices[5]=_vertex3f(-Width1/2, Height1/2, 0.);

    glBindVertexArray(VAO1);
    // uptdate vertices coordinates
    glNamedBufferSubData(VBO1_1,0,Vertices.size()*3*sizeof(float),&Vertices[0]);

    glBindVertexArray(0);

    update();
  }
}

//HEA

void _gl_widget::refresh_image(cv::Mat *Image)
{
  if (Image->channels()==1){
    cv::cvtColor(*Image,Frame_buffer,CV_GRAY2BGR,3);
  }
  else{
    Image->copyTo(Frame_buffer);
  }

  glTexImage2D(GL_TEXTURE_2D,0,3,Frame_buffer.cols,Frame_buffer.rows,0,GL_BGR,GL_UNSIGNED_BYTE,Frame_buffer.data);

//  Drawn=false;

  update();
}


//HEA

void _gl_widget::adjust_image_sizes(int &Width1, int &Height1)
{
  float Rest;

  if (Width1%4!=0){
    Rest=roundf(float(Width1)/4.0f);
    Width1=int(Rest*4);
  }
  if (Height1%4!=0){
    Rest=roundf(float(Height1)/4.0f);
    Height1=int(Rest*4);
  }
}

//

void _gl_widget::create_widget_parameters()
{
  QString Str;
  char Aux[200];

  Widget_parameters=new QWidget;

  QVBoxLayout *Verticalbox_layers=new QVBoxLayout;

  // parameter 1
  Group_box_parameter1=new QGroupBox(tr(String_group_box_parameter1.c_str()));
  Group_box_parameter1->setAlignment(Qt::AlignCenter);

  QGridLayout *Grid_parameter1 = new QGridLayout;

  QLabel *Label_parameter1_min= new QLabel(tr(String_label_parameter1_min.c_str()));
  QLabel *Label_parameter1_max= new QLabel(tr(String_label_parameter1_max.c_str()));

  Line_edit_parameter1=new QLineEdit();
  Line_edit_parameter1->setAlignment(Qt::AlignRight);
  Line_edit_parameter1->setReadOnly(true);
  Line_edit_parameter1->setEnabled(false);
  Line_edit_parameter1->setToolTip(tr(String_label_parameter1_tooltip.c_str()));
  Str=Str.number(dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->color_restoration_variance());
  Line_edit_parameter1->setText(Str);

  Slider_parameter1 = new QSlider(Qt::Horizontal);
  Slider_parameter1->setRange(Parameter1_min_value,Parameter1_max_value);
  Slider_parameter1->setSingleStep(Parameter1_single_step);
  Slider_parameter1->setPageStep(Parameter1_page_step);
  Slider_parameter1->setTickInterval(Parameter1_tick_interval);
  Slider_parameter1->setTickPosition(QSlider::TicksRight);
  Slider_parameter1->setTracking(Parameter1_set_tracking);
  Slider_parameter1->setToolTip(tr(String_parameter1_slider_tooltip.c_str()));
  set_parameter1(dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->color_restoration_variance());

#ifndef TEST
  Grid_parameter1->addWidget(Line_edit_parameter1,0,1,Qt::AlignCenter);
#endif
  Grid_parameter1->addWidget(Label_parameter1_min,1,0,Qt::AlignRight);
  Grid_parameter1->addWidget(Slider_parameter1,1,1);
  Grid_parameter1->addWidget(Label_parameter1_max,1,2,Qt::AlignLeft);

  Group_box_parameter1->setLayout(Grid_parameter1);

//  Vertical_box_main->addWidget(Group_box_parameter1);

//  Group_box_main->setLayout(Vertical_box_main);

  connect(Slider_parameter1, SIGNAL(valueChanged(int)),this,SLOT(set_parameter1_slot(int)));


  // parameter 2
  Group_box_parameter2=new QGroupBox(tr(String_group_box_parameter2.c_str()));
  Group_box_parameter2->setAlignment(Qt::AlignCenter);

  QGridLayout *Grid_parameter2 = new QGridLayout;

  QLabel *Label_parameter2_min= new QLabel(tr(String_label_parameter2_min.c_str()));
  QLabel *Label_parameter2_max= new QLabel(tr(String_label_parameter2_max.c_str()));

  Line_edit_parameter2=new QLineEdit();
  Line_edit_parameter2->setAlignment(Qt::AlignRight);
  Line_edit_parameter2->setReadOnly(true);
  Line_edit_parameter2->setEnabled(false);
  Line_edit_parameter2->setToolTip(tr(String_label_parameter2_tooltip.c_str()));
  Str=Str.number(dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->small_gaussian_size());
  Line_edit_parameter2->setText(Str);

  Slider_parameter2 = new QSlider(Qt::Horizontal);
  Slider_parameter2->setRange(Parameter2_min_value,Parameter2_max_value);
  Slider_parameter2->setSingleStep(Parameter2_single_step);
  Slider_parameter2->setPageStep(Parameter2_page_step);
  Slider_parameter2->setTickInterval(Parameter2_tick_interval);
  Slider_parameter2->setTickPosition(QSlider::TicksRight);
  Slider_parameter2->setTracking(Parameter2_set_tracking);
  Slider_parameter2->setToolTip(tr(String_parameter2_slider_tooltip.c_str()));
  set_parameter2(dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->small_gaussian_size());

#ifndef TEST
  Grid_parameter2->addWidget(Line_edit_parameter2,0,1,Qt::AlignCenter);
#endif
  Grid_parameter2->addWidget(Label_parameter2_min,1,0,Qt::AlignRight);
  Grid_parameter2->addWidget(Slider_parameter2,1,1);
  Grid_parameter2->addWidget(Label_parameter2_max,1,2,Qt::AlignLeft);

  Group_box_parameter2->setLayout(Grid_parameter2);

  connect(Slider_parameter2, SIGNAL(valueChanged(int)),this,SLOT(set_parameter2_slot(int)));

  // parameter 3
  Group_box_parameter3=new QGroupBox(tr(String_group_box_parameter3.c_str()));
  Group_box_parameter3->setAlignment(Qt::AlignCenter);

  QGridLayout *Grid_parameter3 = new QGridLayout;

  QLabel *Label_parameter3_min= new QLabel(tr(String_label_parameter3_min.c_str()));
  QLabel *Label_parameter3_max= new QLabel(tr(String_label_parameter3_max.c_str()));

  Line_edit_parameter3=new QLineEdit();
  Line_edit_parameter3->setAlignment(Qt::AlignRight);
  Line_edit_parameter3->setReadOnly(true);
  Line_edit_parameter3->setEnabled(false);
  Line_edit_parameter3->setToolTip(tr(String_label_parameter3_tooltip.c_str()));
  Str=Str.number(dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->medium_gaussian_size());
  Line_edit_parameter3->setText(Str);

  Slider_parameter3 = new QSlider(Qt::Horizontal);
  Slider_parameter3->setRange(Parameter3_min_value,Parameter3_max_value);
  Slider_parameter3->setSingleStep(Parameter3_single_step);
  Slider_parameter3->setPageStep(Parameter3_page_step);
  Slider_parameter3->setTickInterval(Parameter3_tick_interval);
  Slider_parameter3->setTickPosition(QSlider::TicksRight);
  Slider_parameter3->setTracking(Parameter3_set_tracking);
  Slider_parameter3->setToolTip(tr(String_parameter3_slider_tooltip.c_str()));
  set_parameter3(dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->medium_gaussian_size());

#ifndef TEST
  Grid_parameter3->addWidget(Line_edit_parameter3,0,1,Qt::AlignCenter);
#endif
  Grid_parameter3->addWidget(Label_parameter3_min,1,0,Qt::AlignRight);
  Grid_parameter3->addWidget(Slider_parameter3,1,1);
  Grid_parameter3->addWidget(Label_parameter3_max,1,2,Qt::AlignLeft);

  Group_box_parameter3->setLayout(Grid_parameter3);

  connect(Slider_parameter3, SIGNAL(valueChanged(int)),this,SLOT(set_parameter3_slot(int)));

  // parameter 4
  Group_box_parameter4=new QGroupBox(tr(String_group_box_parameter4.c_str()));
  Group_box_parameter4->setAlignment(Qt::AlignCenter);

  QGridLayout *Grid_parameter4 = new QGridLayout;

  QLabel *Label_parameter4_min= new QLabel(tr(String_label_parameter4_min.c_str()));
  QLabel *Label_parameter4_max= new QLabel(tr(String_label_parameter4_max.c_str()));

  Line_edit_parameter4=new QLineEdit();
  Line_edit_parameter4->setAlignment(Qt::AlignRight);
  Line_edit_parameter4->setReadOnly(true);
  Line_edit_parameter4->setEnabled(false);
  Line_edit_parameter4->setToolTip(tr(String_label_parameter4_tooltip.c_str()));
  Str=Str.number(dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->big_gaussian_size());
  Line_edit_parameter4->setText(Str);

  Slider_parameter4 = new QSlider(Qt::Horizontal);
  Slider_parameter4->setRange(Parameter4_min_value,Parameter4_max_value);
  Slider_parameter4->setSingleStep(Parameter4_single_step);
  Slider_parameter4->setPageStep(Parameter4_page_step);
  Slider_parameter4->setTickInterval(Parameter4_tick_interval);
  Slider_parameter4->setTickPosition(QSlider::TicksRight);
  Slider_parameter4->setTracking(Parameter4_set_tracking);
  Slider_parameter4->setToolTip(tr(String_parameter4_slider_tooltip.c_str()));
  set_parameter4(dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->big_gaussian_size());

#ifndef TEST
  Grid_parameter4->addWidget(Line_edit_parameter4,0,1,Qt::AlignCenter);
#endif
  Grid_parameter4->addWidget(Label_parameter4_min,1,0,Qt::AlignRight);
  Grid_parameter4->addWidget(Slider_parameter4,1,1);
  Grid_parameter4->addWidget(Label_parameter4_max,1,2,Qt::AlignLeft);

  Group_box_parameter4->setLayout(Grid_parameter4);

  connect(Slider_parameter4, SIGNAL(valueChanged(int)),this,SLOT(set_parameter4_slot(int)));



  // Parameter5
  QGroupBox *Group_box_parameter5=new QGroupBox(tr(String_group_box_parameter5.c_str()));
  Group_box_parameter5->setAlignment(Qt::AlignCenter);

  QGridLayout *Grid_parameter5 = new QGridLayout;

  QLabel *Label_parameter5_min= new QLabel(tr(String_label_parameter5_min.c_str()));
  QLabel *Label_parameter5_max= new QLabel(tr(String_label_parameter5_max.c_str()));

  Line_edit_parameter5=new QLineEdit();
  Line_edit_parameter5->setAlignment(Qt::AlignRight);
  Line_edit_parameter5->setReadOnly(true);
  Line_edit_parameter5->setEnabled(false);
  Line_edit_parameter5->setToolTip(tr(String_label_parameter5_tooltip.c_str()));
  // warnig to the adjust
  sprintf(Aux,"%4.5f",dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->contrast());
  Str=Aux;
  Line_edit_parameter5->setText(Str);

  Slider_parameter5 = new QSlider(Qt::Horizontal);
  Slider_parameter5->setRange(Parameter5_min_value,Parameter5_max_value);
  Slider_parameter5->setSingleStep(Parameter5_single_step);
  Slider_parameter5->setPageStep(Parameter5_page_step);
  Slider_parameter5->setTickInterval(Parameter5_tick_interval);
  Slider_parameter5->setTickPosition(QSlider::TicksRight);
  Slider_parameter5->setTracking(Parameter5_set_tracking);
  // warnig to the adjust
//  Slider_parameter5->setValue(dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->contrast()*_f_retinex_c_ns::Contrast_divisor);
  float Val=dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->contrast();
  Slider_parameter5->setToolTip(tr(String_parameter5_tooltip.c_str()));
  set_parameter5(dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->contrast());

#ifndef TEST
  Grid_parameter5->addWidget(Line_edit_parameter5,0,1,Qt::AlignCenter);
#endif
  Grid_parameter5->addWidget(Label_parameter5_min,1,0,Qt::AlignRight);
  Grid_parameter5->addWidget(Slider_parameter5,1,1);
  Grid_parameter5->addWidget(Label_parameter5_max,1,2,Qt::AlignLeft);

  Group_box_parameter5->setLayout(Grid_parameter5);

  connect(Slider_parameter5, SIGNAL(valueChanged(int)),this,SLOT(set_parameter5_slot(int)));

  // Parameter 6
  Group_box_parameter6=new QGroupBox(tr(String_group_box_parameter6.c_str()));
  Group_box_parameter6->setAlignment(Qt::AlignCenter);

  QGridLayout *Grid_parameter6 = new QGridLayout;

  QLabel *Label_parameter6_min= new QLabel(tr(String_label_parameter6_min.c_str()));
  QLabel *Label_parameter6_max= new QLabel(tr(String_label_parameter6_max.c_str()));

  Line_edit_parameter6=new QLineEdit();
  Line_edit_parameter6->setAlignment(Qt::AlignRight);
  Line_edit_parameter6->setReadOnly(true);
  Line_edit_parameter6->setEnabled(false);
  Line_edit_parameter6->setToolTip(tr(String_label_parameter6_tooltip.c_str()));
  Str=Str.number(dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->bright());
  Line_edit_parameter6->setText(Str);

  Slider_parameter6 = new QSlider(Qt::Horizontal);
  Slider_parameter6->setRange(Parameter6_min_value,Parameter6_max_value);
  Slider_parameter6->setSingleStep(Parameter6_single_step);
  Slider_parameter6->setPageStep(Parameter6_page_step);
  Slider_parameter6->setTickInterval(Parameter6_tick_interval);
  Slider_parameter6->setTickPosition(QSlider::TicksRight);
  Slider_parameter6->setTracking(Parameter6_set_tracking);
//  Slider_parameter6->setValue(dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->bright());
  Slider_parameter6->setToolTip(tr(String_parameter6_slider_tooltip.c_str()));
  set_parameter6(dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->bright());

#ifndef TEST
  Grid_parameter6->addWidget(Line_edit_parameter6,0,1,Qt::AlignCenter);
#endif
  Grid_parameter6->addWidget(Label_parameter6_min,1,0,Qt::AlignRight);
  Grid_parameter6->addWidget(Slider_parameter6,1,1);
  Grid_parameter6->addWidget(Label_parameter6_max,1,2,Qt::AlignLeft);

  Group_box_parameter6->setLayout(Grid_parameter6);

//  Vertical_box_main->addWidget(Group_box_parameter1);
//  Vertical_box_main->addWidget(Group_box_parameter3);

//  Group_box_main->setLayout(Vertical_box_main);

  connect(Slider_parameter6, SIGNAL(valueChanged(int)),this,SLOT(set_parameter6_slot(int)));

  // next step button
  Button_next_step=new QPushButton(String_next_step);
  connect(Button_next_step, SIGNAL(clicked()),this,SLOT(button_next_step_slot()));
  Button_next_step->setEnabled(true);

  // previous step button
  Button_previous_step=new QPushButton(String_previous_step);
  connect(Button_previous_step, SIGNAL(clicked()),this,SLOT(button_previous_step_slot()));
  Button_previous_step->setEnabled(false);


  // Parameter 7
  Group_box_parameter7=new QGroupBox(tr(String_group_box_parameter7.c_str()));
  Group_box_parameter7->setAlignment(Qt::AlignCenter);

  QVBoxLayout *Verticalbox_images=new QVBoxLayout;

  // original button
  Button_original=new QPushButton(String_button_original);
  connect(Button_original, SIGNAL(clicked()),this,SLOT(button_original_slot()));

  // stippling button
  Button_stippling=new QPushButton(String_button_stippling);
  connect(Button_stippling, SIGNAL(clicked()),this,SLOT(button_stippling_slot()));

  Verticalbox_images->addWidget(Button_original);
  Verticalbox_images->addWidget(Button_stippling);

  Group_box_parameter7->setLayout(Verticalbox_images);


  //-------------
  // separator
  QFrame *Separator=new QFrame();
  Separator->setFrameStyle(QFrame::HLine);

  QFrame *Separator1=new QFrame();
  Separator1->setFrameStyle(QFrame::HLine);

  Verticalbox_layers->addWidget(Group_box_parameter1);
  Verticalbox_layers->addWidget(Group_box_parameter2);
  Verticalbox_layers->addWidget(Group_box_parameter3);
  Verticalbox_layers->addWidget(Group_box_parameter4);
  Verticalbox_layers->addWidget(Group_box_parameter5);
  Verticalbox_layers->addWidget(Group_box_parameter6);
  Verticalbox_layers->addWidget(Separator);
  Verticalbox_layers->addWidget(Button_previous_step);
  Verticalbox_layers->addWidget(Button_next_step);
  Verticalbox_layers->addStretch();
  Verticalbox_layers->addWidget(Separator1);
  Verticalbox_layers->addWidget(Group_box_parameter7);


  Widget_parameters->setLayout(Verticalbox_layers);

  Widget_parameters->setEnabled(true);

}


//HEA

void _gl_widget::set_parameter1(float Value)
{
  QString Str;
  char Aux[100];
  int Int_value;

  Int_value=(int)(255.0f*(Value/(float)RETINEX_MAX_COLOR_RESTORATION_VARIANCE));
  sprintf(Aux,"%2.2f",Value);
  Str=Aux;
  Line_edit_parameter1->setText(Str);
  Slider_parameter1->blockSignals(true);
  Slider_parameter1->setValue(Int_value);
  Slider_parameter1->blockSignals(false);
}

//HEA

void _gl_widget::set_parameter2(int Value)
{
  QString Str;

  Str=Str.number(Value);
  Line_edit_parameter2->setText(Str);
  Slider_parameter2->blockSignals(true);
  Slider_parameter2->setValue(Value);
  Slider_parameter2->blockSignals(false);
}

//HEA

void _gl_widget::set_parameter3(int Value)
{
  QString Str;

  Str=Str.number(Value);
  Line_edit_parameter3->setText(Str);
  Slider_parameter3->blockSignals(true);
  Slider_parameter3->setValue(Value);
  Slider_parameter3->blockSignals(false);
}

//HEA

void _gl_widget::set_parameter4(int Value)
{
  QString Str;

  Str=Str.number(Value);
  Line_edit_parameter4->setText(Str);
  Slider_parameter4->blockSignals(true);
  Slider_parameter4->setValue(Value);
  Slider_parameter4->blockSignals(false);
}

//HEA

void _gl_widget::set_parameter5(float Value)
{
  QString Str;
  char Aux[100];

  sprintf(Aux,"%4.2f",Value);
//  Value=Value*(float)Parameter1_divisor;

  Str=Aux;
  Line_edit_parameter5->setText(Str);
  Slider_parameter5->blockSignals(true);
  Slider_parameter5->setValue(int(Value*float(Parameter5_divisor)));
  Slider_parameter5->blockSignals(false);
}

//HEA

void _gl_widget::set_parameter6(int Value)
{
  QString Str;

  Str=Str.number(Value);
  Line_edit_parameter6->setText(Str);
  Slider_parameter6->blockSignals(true);
  Slider_parameter6->setValue(Value);
  Slider_parameter6->blockSignals(false);
}


//HEA

void _gl_widget::set_parameter1_slot(int Value)
{
  QString Str;
  char Aux[100];
  float Variance;

  Variance=(float)RETINEX_MAX_COLOR_RESTORATION_VARIANCE*(1-((255.0-(float)Value)/255.0));
  Vec_data_filters[Image_number].Color_restoration=Variance;

  sprintf(Aux,"%2.2f",Variance);
  Str=Aux;
  Line_edit_parameter1->setText(Str);
  dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->color_restoration_variance(Variance);
  update_filters(FILTER_RETINEX_C);

  Vec_count_events[Image_number][0]++;
//  cout << "Num events=" << Vec_count_events[Image_number][0] << endl;
}

//HEA

void _gl_widget::set_parameter2_slot(int Value)
{
  QString Str;
  char Aux[100];
  int Medium_value=dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->medium_gaussian_size();

  if (Value%2==0) Value++;
  // check that the small value to be small than the medium value
  if (Value>=Medium_value){
    Value=Medium_value-2;
  }

  Slider_parameter2->blockSignals(true);
  Slider_parameter2->setValue(Value);
  Slider_parameter2->blockSignals(false);

  Vec_data_filters[Image_number].Small_gaussian=Value;
  sprintf(Aux,"%2d",Value);
  Str=Aux;
  Line_edit_parameter2->setText(Str);
  dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->small_gaussian_size(Value);
  update_filters(FILTER_RETINEX_C);

  Vec_count_events[Image_number][1]++;
}

//HEA

void _gl_widget::set_parameter3_slot(int Value)
{
  QString Str;
  char Aux[100];
  int Small_value=dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->small_gaussian_size();
  int Big_value=dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->big_gaussian_size();

  if (Value%2==0) Value++;
  // check that the small value to be small than the medium value
  if (Value<=Small_value){
    Value=Small_value+2;
  }
  if (Value>=Big_value){
    Value=Big_value-2;
  }

  Slider_parameter3->blockSignals(true);
  Slider_parameter3->setValue(Value);
  Slider_parameter3->blockSignals(false);

  Vec_data_filters[Image_number].Medium_gaussian=Value;
  sprintf(Aux,"%2d",Value);
  Str=Aux;
  Line_edit_parameter3->setText(Str);
  dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->medium_gaussian_size(Value);
  update_filters(FILTER_RETINEX_C);

  Vec_count_events[Image_number][2]++;
}

//HEA

void _gl_widget::set_parameter4_slot(int Value)
{
  QString Str;
  char Aux[100];
  int Medium_value=dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->medium_gaussian_size();

  if (Value%2==0) Value++;
  // check that the small value to be small than the medium value
  if (Value<=Medium_value){
    Value=Medium_value+2;
  }

  Slider_parameter4->blockSignals(true);
  Slider_parameter4->setValue(Value);
  Slider_parameter4->blockSignals(false);

  Vec_data_filters[Image_number].Big_gaussian=Value;
  sprintf(Aux,"%2d",Value);
  Str=Aux;
  Line_edit_parameter4->setText(Str);
  dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->big_gaussian_size(Value);
  update_filters(FILTER_RETINEX_C);

  Vec_count_events[Image_number][3]++;
}

//HEA

void _gl_widget::set_parameter5_slot(int Value1)
{
  QString Str;
  char Aux[100];
  float Value=float(Value1)/float(Parameter5_divisor);

  Vec_data_filters[Image_number].Contrast=Value;
  sprintf(Aux,"%4.2f",Value);
  Str=Aux;
  Line_edit_parameter5->setText(Str);
  dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->contrast(Value);
  update_filters(FILTER_RETINEX_C);

  Vec_count_events[Image_number][4]++;
}

//HEA

void _gl_widget::set_parameter6_slot(int Value)
{
  QString Str;
  char Aux[100];

  Vec_data_filters[Image_number].Bright=Value;
  sprintf(Aux,"%2d",Value);
  Str=Aux;
  Line_edit_parameter6->setText(Str);
  dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->bright(Value);
  update_filters(FILTER_RETINEX_C);

  Vec_count_events[Image_number][5]++;
}

//HEA

void _gl_widget::update_filters(unsigned int Pos)
{
  if (Pos<Vec_filters.size()){
    for (unsigned int i=Pos;i<Vec_filters.size();i++){
      Vec_filters[i]->update();
    }
    refresh_image();
  }
}

void _gl_widget::button_next_step_slot()
{
  QMessageBox MsgBox;
  MsgBox.setIcon(QMessageBox::Information);
  MsgBox.setFixedSize(500,300);
  MsgBox.setText(String_warning_box);
  MsgBox.setInformativeText(String_message);
  MsgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
  MsgBox.setDefaultButton(QMessageBox::Cancel);

  // trick to change the messagebox size
  QSpacerItem *Horizontal_spacer=new QSpacerItem(500,0,QSizePolicy::Minimum,QSizePolicy::Expanding);
  QGridLayout *Layout=(QGridLayout *) MsgBox.layout();
  Layout->addItem(Horizontal_spacer,Layout->rowCount(),0,1,Layout->columnCount());
  int Ret=MsgBox.exec();

  if (Ret==QMessageBox::Ok){
    next_image();
  }
}


void _gl_widget::button_previous_step_slot()
{
  previous_image();
}


void _gl_widget::button_original_slot()
{
  Selected_image_index=ORIGINAL;
  refresh_image();
}



void _gl_widget::button_stippling_slot()
{
  Selected_image_index=FILTER_STIPPLING_EBG;
  refresh_image();
}



void _gl_widget::next_image()
{
  if (Image_number==-1){
    input_data();  

    if (DEBUG) cout << "Input data is Ok" << endl;
    if (DEBUG) Output_stream.flush();
  }
  Image_number++;
  if (Image_number<MAX_IMAGES){
    if (Image_number>0){
      Button_previous_step->setEnabled(true);

      Stop_time=std::chrono::steady_clock::now();
      std::chrono::duration<double> Elapsed = Stop_time-Start_time;
      Vec_elapsed_time[Image_number-1]+=float(Elapsed.count());

//      cout << "Image=" << Image_number-1 << " Elapsed time=" << Vec_elapsed_time[Image_number-1] << " " << float(Elapsed.count()) << endl;

      Start_time=std::chrono::steady_clock::now();
    }
    else{
      Button_previous_step->setEnabled(false);

      Start_time=std::chrono::steady_clock::now();
    }

    read_image(Vec_file_names[Image_number]);

    if (DEBUG) cout << "Read image is Ok" << endl;
    if (DEBUG) Output_stream.flush();

    dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->color_restoration_variance(Vec_data_filters[Image_number].Color_restoration);
    dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->small_gaussian_size(Vec_data_filters[Image_number].Small_gaussian);
    dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->medium_gaussian_size(Vec_data_filters[Image_number].Medium_gaussian);
    dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->big_gaussian_size(Vec_data_filters[Image_number].Big_gaussian);
    dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->contrast(Vec_data_filters[Image_number].Contrast);
    dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->bright(Vec_data_filters[Image_number].Bright);

    set_parameter1(Vec_data_filters[Image_number].Color_restoration);
    set_parameter2(Vec_data_filters[Image_number].Small_gaussian);
    set_parameter3(Vec_data_filters[Image_number].Medium_gaussian);
    set_parameter4(Vec_data_filters[Image_number].Big_gaussian);
    set_parameter5(Vec_data_filters[Image_number].Contrast);
    set_parameter6(Vec_data_filters[Image_number].Bright);

    update_filters(FILTER_RETINEX_C);
  }
  else{
    Stop_time=std::chrono::steady_clock::now();
    std::chrono::duration<double> Elapsed =std::chrono::duration_cast<std::chrono::duration<double>>(Stop_time-Start_time);
    Vec_elapsed_time[Image_number-1]+=float(Elapsed.count());

    QMessageBox MsgBox;
    MsgBox.setIcon(QMessageBox::Information);
    MsgBox.setFixedSize(500,300);
    MsgBox.setText(String_warning);
    MsgBox.setInformativeText(String_message_end);
    MsgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    MsgBox.setDefaultButton(QMessageBox::Ok);

    // trick to change the messagebox size
    QSpacerItem *Horizontal_spacer=new QSpacerItem(500,0,QSizePolicy::Minimum,QSizePolicy::Expanding);
    QGridLayout *Layout=(QGridLayout *) MsgBox.layout();
    Layout->addItem(Horizontal_spacer,Layout->rowCount(),0,1,Layout->columnCount());
    int Result=MsgBox.exec();

    if (Result==QMessageBox::Ok){
      time_t Now = time(nullptr);
      tm *Local_time = localtime(&Now);
      string File_name;
      string String_now;

      // print various components of tm structure.
      string Year=QString("%1").arg(1900+Local_time->tm_year,4,10,QChar('0')).toStdString();
      string Month=QString("%1").arg(1 + Local_time->tm_mon,2,10,QChar('0')).toStdString();
      string Day=QString("%1").arg(Local_time->tm_mday,2,10,QChar('0')).toStdString();
      string Hour=QString("%1").arg(1 + Local_time->tm_hour,2,10,QChar('0')).toStdString();
      string Minutes=QString("%1").arg(1 + Local_time->tm_min,2,10,QChar('0')).toStdString();
      string Seconds=QString("%1").arg(1 + Local_time->tm_sec,2,10,QChar('0')).toStdString();

      String_now=Year+Month+Day+Hour+Minutes+Seconds;

      File_name="results_"+QString("%1").arg(Personal_data.Test_id,2,10,QChar('0')).toStdString()+"_"+String_now+".txt";

      save_data(File_name,Dir_results);
      _upload Upload;
      Upload.upload(File_name,Dir_results,Url);

      if (DEBUG) cout << "Saved data is Ok" << endl;
      if (DEBUG) Output_stream.close();

      exit(0);
    }
  }
}

void _gl_widget::previous_image()
{
  Image_number--;
  if (Image_number<0) Image_number=0;

  if (Image_number>0){
    Button_previous_step->setEnabled(true);
  }
  else{
    Button_previous_step->setEnabled(false);
  }

  Stop_time=std::chrono::steady_clock::now();
  std::chrono::duration<double> Elapsed = Stop_time-Start_time;
  Vec_elapsed_time[Image_number+1]+=float(Elapsed.count());

//  cout << "Image=" << Image_number+1 << " Elapsed time=" << Vec_elapsed_time[Image_number+1] << " " << float(Elapsed.count()) << endl;

  Start_time=std::chrono::steady_clock::now();

  read_image(Vec_file_names[Image_number]);

  dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->color_restoration_variance(Vec_data_filters[Image_number].Color_restoration);
  dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->small_gaussian_size(Vec_data_filters[Image_number].Small_gaussian);
  dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->medium_gaussian_size(Vec_data_filters[Image_number].Medium_gaussian);
  dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->big_gaussian_size(Vec_data_filters[Image_number].Big_gaussian);
  dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->contrast(Vec_data_filters[Image_number].Contrast);
  dynamic_cast<_filter_retinex_c *>(Vec_filters[FILTER_RETINEX_C].get())->bright(Vec_data_filters[Image_number].Bright);

  set_parameter1(Vec_data_filters[Image_number].Color_restoration);
  set_parameter2(Vec_data_filters[Image_number].Small_gaussian);
  set_parameter3(Vec_data_filters[Image_number].Medium_gaussian);
  set_parameter4(Vec_data_filters[Image_number].Big_gaussian);
  set_parameter5(Vec_data_filters[Image_number].Contrast);
  set_parameter6(Vec_data_filters[Image_number].Bright);

  update_filters(FILTER_RETINEX_C);
}


void _gl_widget::input_data()
{
  Dialogbox=new QDialog;

  Dialogbox->setFixedSize(500,300);

//  QSizePolicy Q(QSizePolicy::Expanding,QSizePolicy::Expanding);

  QGroupBox *Groupbox1=new QGroupBox;
  Groupbox1->setAlignment(Qt::AlignCenter);

  QVBoxLayout *Verticalbox = new QVBoxLayout();

  QLabel *Label_test_id = new QLabel(String_test_id);
  Combo_test_id = new QComboBox;
  for (uint i=0;i<Strings_test_id.size();i++) Combo_test_id->addItem(Strings_test_id[i]);
  Combo_test_id->setCurrentIndex(Personal_data.Test_id);


  QLabel *Label_age = new QLabel(String_age);
//  QLineEdit *Lineedit_age = new QLineEdit;
//  Spinbox_age = new QSpinBox;
  Combo_age = new QComboBox;
  for (uint i=0;i<Strings_age.size();i++) Combo_age->addItem(Strings_age[i]);
  Combo_age->setCurrentIndex(Personal_data.Age);

  //
  QLabel *Label_sex = new QLabel(String_sex);
  Combo_sex = new QComboBox;
  for (uint i=0;i<Strings_sex.size();i++) Combo_sex->addItem(Strings_sex[i]);
  Combo_sex->setCurrentIndex(Personal_data.Sex);

  //  connect(Combo_output_size, SIGNAL(currentIndexChanged(int)),this,SLOT(output_size_slot(int)));
  QLabel *Label_activity = new QLabel(String_activity);
  Combo_activity = new QComboBox;
  for (uint i=0;i<Strings_activity.size();i++) Combo_activity->addItem(Strings_activity[i]);
  Combo_activity->setCurrentIndex(Personal_data.Activity);

  QLabel *Label_stippling_relationship = new QLabel(String_stippling_relationship);
  Combo_stippling_relationship = new QComboBox;
  for (uint i=0;i<Strings_stippling_relationship.size();i++) Combo_stippling_relationship->addItem(Strings_stippling_relationship[i]);
  Combo_stippling_relationship->setCurrentIndex(Personal_data.Stippling_relationship);

  QLabel *Label_creating_stippling= new QLabel(String_creating_stippling);
  Combo_creating_stippling = new QComboBox;
  for (uint i=0;i<Strings_creating_stippling.size();i++) Combo_creating_stippling->addItem(Strings_creating_stippling[i]);
  Combo_creating_stippling->setCurrentIndex(Personal_data.Creating_stippling);


  QGridLayout *layout = new QGridLayout;
  layout->addWidget(Label_test_id, 0, 0);
  layout->addWidget(Combo_test_id, 0, 1);
  layout->addWidget(Label_age, 1, 0);
  layout->addWidget(Combo_age, 1, 1);
  layout->addWidget(Label_sex, 2, 0);
  layout->addWidget(Combo_sex, 2, 1);
  layout->addWidget(Label_activity, 3, 0);
  layout->addWidget(Combo_activity, 3, 1);
  layout->addWidget(Label_stippling_relationship, 4, 0);
  layout->addWidget(Combo_stippling_relationship, 4, 1);
  layout->addWidget(Label_creating_stippling, 5, 0);
  layout->addWidget(Combo_creating_stippling, 5, 1);

  Groupbox1->setLayout(layout);


  QPushButton *Button_ok=new QPushButton("Ok");
  Button_ok->setMaximumWidth(100);
  connect(Button_ok, SIGNAL(clicked()),this,SLOT(button_ok_slot()));


  Verticalbox->addWidget(Groupbox1);
  Verticalbox->addStretch(1);
  Verticalbox->addWidget(Button_ok);

  Dialogbox->setLayout(Verticalbox);

//  Dialogbox.setLayout(layout);

  // trick to change the messagebox size
//  QSpacerItem *Horizontal_spacer=new QSpacerItem(500,0,QSizePolicy::Minimum,QSizePolicy::Expanding);
//  QGridLayout *Layout=(QGridLayout *) Dialogbox.layout();
//  Layout->addItem(Horizontal_spacer,Layout->rowCount(),0,1,Layout->columnCount());


  if (Dialogbox->exec()==QDialog::Rejected){
    message_fault();
    input_data();
  }
  else{
    //
    char Dir_name[200];

    for (unsigned int i=0;i<5;i++){
      sprintf(Dir_name,"/variable_%03d_%03d/image%03d.png",Personal_data.Test_id*5,Personal_data.Test_id*5+4,i);
      Vec_file_names[5+i]=Vec_file_names[5+i]+string(Dir_name);
    }
  }
}



void _gl_widget::message_fault()
{
  QMessageBox Messagebox;
  Messagebox.setText(String_fault);
  Messagebox.exec();
}


bool _gl_widget::check_file(int Pos)
{
  char Dir_name[200];
  sprintf(Dir_name,"images/variable_%03d_%03d/image%03d.png",Pos*5,Pos*5+4,0);

  struct stat Buffer;
  if (stat(Dir_name,&Buffer)==0) return true;
  else return false;
}


void _gl_widget::button_ok_slot()
{
  bool Check_file=true;

  Personal_data.Test_id=Combo_test_id->currentIndex();
  Personal_data.Age=Combo_age->currentIndex();
  Personal_data.Sex=Combo_sex->currentIndex();
  Personal_data.Activity=Combo_activity->currentIndex();
  Personal_data.Stippling_relationship=Combo_stippling_relationship->currentIndex();
  Personal_data.Creating_stippling=Combo_creating_stippling->currentIndex();

  if (Combo_test_id->currentIndex()==0 || Combo_age->currentIndex()==0 || Combo_sex->currentIndex()==0 || Combo_activity->currentIndex()==0 || Combo_stippling_relationship->currentIndex()==0 || Combo_creating_stippling->currentIndex()==0 || (Check_file=check_file(Combo_test_id->currentIndex()-1))==false){
    if (Check_file==false){
      QMessageBox Messagebox;
      Messagebox.setText(String_bad_id);
      Messagebox.exec();
    }
    Dialogbox->done(QDialog::Rejected);
  }
  else {
    // one less because the first case, 0, is the null case
    Personal_data.Test_id--;
    Personal_data.Age--;
    Personal_data.Sex--;
    Personal_data.Activity--;
    Personal_data.Stippling_relationship--;
    Personal_data.Creating_stippling--;
    Dialogbox->done(QDialog::Accepted);
  }
}


void _gl_widget::save_data(string File_name,string File_dir)
{
  std::ofstream File;

  File.open (File_dir+File_name);

  File << std::to_string(Personal_data.Test_id) << ";" << endl;
  File << std::to_string(Personal_data.Age+18) << ";" << endl;
  File << std::to_string(Personal_data.Sex) << ";" << endl;
  File << std::to_string(Personal_data.Activity) << ";" << endl;
  File << std::to_string(Personal_data.Stippling_relationship) << ";" << endl;
  File << std::to_string(Personal_data.Creating_stippling) << ";" << endl;
  for (unsigned int i=0;i<MAX_IMAGES;i++){
    File << std::to_string(Vec_elapsed_time[i]) << "; ";
    for (unsigned int j=0;j<6;j++){
      File << std::to_string(Vec_count_events[i][j]) << "; ";
    }
    File << std::to_string(Vec_data_filters[i].Color_restoration) << "; ";
    File << std::to_string(Vec_data_filters[i].Small_gaussian) << "; ";
    File << std::to_string(Vec_data_filters[i].Medium_gaussian) << "; ";
    File << std::to_string(Vec_data_filters[i].Big_gaussian) << "; ";
    File << std::to_string(Vec_data_filters[i].Contrast) << "; ";
    File << std::to_string(Vec_data_filters[i].Bright) << "; ";
    File << std::endl;
  }
  File.close();
}
