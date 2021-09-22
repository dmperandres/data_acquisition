//LIC

#include "filter_retinex_c.h"
#include "glwidget.h"


using namespace _f_retinex_c_ns;

//HEA

_filter_retinex_c::_filter_retinex_c(_gl_widget *GL_widget1)
{
  GL_widget=GL_widget1;

  Vec_kernel_size[0]=RETINEX_SMALL_GAUSSIAN_KERNEL_SIZE_DEFAULT;
  Vec_kernel_size[1]=RETINEX_MEDIUM_GAUSSIAN_KERNEL_SIZE_DEFAULT;
  Vec_kernel_size[2]=RETINEX_BIG_GAUSSIAN_KERNEL_SIZE_DEFAULT;
  Num_kernels=RETINEX_NUM_KERNELS_DEFAULT;
  Color_restoration_variance=RETINEX_COLOR_RESTORATION_VARIANCE_DEFAULT;

  Contrast=CONTRAST_DEFAULT;
  Bright=BRIGHT_DEFAULT;

  Mode=MODE_UNIFORM;

  Num_channels_input_image_0=3;
  Num_channels_output_image_0=3;

  Type_filter=_f_filter_ns::FILTER_RETINEX;

  Scaling_factor=1;
  Change_output_image_size=false;
  Use_dots=false;

  Tex_orig=0;
  Tex_orig_float=0;
  for (unsigned int i=0;i<6;i++){
    Tex_gaussian[i]=0;
  }

  Buffer_gaussians_coeffs=0;
  Buffer_mean=0;
  Buffer_square=0;

  Fbo_retinex=0;
}

//HEA

_filter_retinex_c::~_filter_retinex_c()
{
  glDeleteTextures(1,&Tex_orig);
  glDeleteTextures(1,&Tex_orig_float);
  glDeleteTextures(6,&Tex_gaussian[0]);
  glDeleteBuffers(1,&Buffer_gaussians_coeffs);
  glDeleteBuffers(1,&Buffer_mean);
  glDeleteBuffers(1,&Buffer_square);
  glDeleteFramebuffers(1,&Fbo_retinex);
  glDeleteProgram(Program0);
  glDeleteProgram(Program1a);
  glDeleteProgram(Program1b);
  glDeleteProgram(Program2);
  glDeleteProgram(Program3);
  glDeleteProgram(Program4);
  glDeleteProgram(Program5);

  glDeleteVertexArrays(1,&VAO1);
}

//HEA

void _filter_retinex_c::create_buffers()
{
  int Width1=Input_image_0->cols;
  int Height1=Input_image_0->rows;


  // GPU
  // load shaders and create buffers
  _shaders Shader;

  /**********************************************************/
  // load the programs

  static const GLchar *P0_vs[]={
    "#version 450 core                                               \n"
    "                                                                \n"
    "void main(void)                                                 \n"
    "{                                                               \n"
    "    const vec4 vertices[] = vec4[](vec4(-1.0, -1.0, 0.5, 1.0),  \n"
    "                                   vec4( 1.0, -1.0, 0.5, 1.0),  \n"
    "                                   vec4(-1.0,  1.0, 0.5, 1.0),  \n"
    "                                   vec4( 1.0,  1.0, 0.5, 1.0)); \n"
    "                                                                \n"
    "    gl_Position = vertices[gl_VertexID];                        \n"
    "}                                                               \n"
  };

  static const GLchar * P0_fs[]={
    "#version 450 core                                              \n"
    "layout (binding = 0,rgba8ui) uniform uimage2D Image_orig;      \n"
    "layout (binding = 1,rgba32f) uniform image2D Image_orig_float; \n"
    "                                                               \n"
    "void main(void)                                                \n"
    "{                                                              \n"
    "  ivec2 Pos=ivec2(gl_FragCoord.xy);                            \n"
    "                                                               \n"
    "   uvec4 Color=imageLoad(Image_orig,Pos);                      \n"
    "   vec4 Color1=vec4(Color)+vec4(1);                            \n"
    "   imageStore(Image_orig_float,Pos,Color1);                    \n"
    "}                                                              \n"
  };

  static const GLchar *P1a_fs[]={
    "#version 450 core                                                                                      \n"
    "                                                                                                       \n"
    "layout (location=0) uniform int Width;                                                                 \n"
    "layout (location=1) uniform int Height;                                                                \n"
    "layout (location=2) uniform int Pass;                                                                  \n"
    "layout (location=3) uniform int Kernel_size[3];                                                        \n"
    "                                                                                                       \n"
    "layout (binding=0,std430) buffer SSB                                                                   \n"
    "{                                                                                                      \n"
    "  float Coeff[768];                                                                                    \n"
    "};                                                                                                     \n"
    "                                                                                                       \n"
    "layout (binding = 0,rgba8ui) uniform uimage2D Image_orig;                                              \n"
    "layout (binding = 1,rgba32f) uniform image2D Image_orig_float;                                         \n"
    "layout (binding = 2,rgba32f) uniform image2D Image_gauss[6];                                           \n"
    "                                                                                                       \n"
    "                                                                                                       \n"
    "void main(void)                                                                                        \n"
    "{                                                                                                      \n"
    "  ivec2 Pos;                                                                                           \n"
    "  ivec2 Pos_aux1;                                                                                      \n"
    "  ivec2 Pos_aux2;                                                                                      \n"
    "  int i;                                                                                               \n"
    "  int j;                                                                                               \n"
    "                                                                                                       \n"
    "                                                                                                       \n"
    "  Pos=ivec2(gl_FragCoord.xy);                                                                          \n"
    "                                                                                                       \n"
    "  for (i=0;i<3;i++){                                                                                   \n"
    "    vec4 Color_aux = vec4(0.0);                                                                        \n"
    "                                                                                                       \n"
    "    Pos_aux1=Pos-ivec2(0,Kernel_size[i]>> 1);                                                          \n"
    "                                                                                                       \n"
    "    for (j=0;j<Kernel_size[i];j++){                                                                    \n"
    "      Pos_aux2=Pos_aux1+ivec2(0,j);                                                                    \n"
    "      // check if the position is inside the image                                                     \n"
    "      if (Pos_aux2.x>=0 && Pos_aux2.x<Width && Pos_aux2.y>=0 && Pos_aux2.y<Height){                    \n"
    "        Color_aux+=imageLoad(Image_orig_float,Pos_aux2)*Coeff[i*256+j];                                \n"
    "      }                                                                                                \n"
    "      else{ // is outside                                                                              \n"
    "        if (Pos_aux2.x<0) Pos_aux2.x=0;                                                                \n"
    "        if (Pos_aux2.x>=Width) Pos_aux2.x=Width-1;                                                     \n"
    "        if (Pos_aux2.y<0) Pos_aux2.y=0;                                                                \n"
    "        if (Pos_aux2.y>Height) Pos_aux2.y=Height-1;                                                    \n"
    "        Color_aux+=imageLoad(Image_orig_float,Pos_aux2)*Coeff[i*256+j];                                \n"
    "      }                                                                                                \n"
    "    }                                                                                                  \n"
    "    imageStore(Image_gauss[i],Pos,Color_aux);                                                          \n"
    "  }                                                                                                    \n"
    "                                                                                                       \n"
    "}                                                                                                      \n"
  };

  static const GLchar *P1b_fs[]={
    "#version 450 core                                                                                      \n"
    "                                                                                                       \n"
    "layout (location=0) uniform int Width;                                                                 \n"
    "layout (location=1) uniform int Height;                                                                \n"
    "layout (location=2) uniform int Pass;                                                                  \n"
    "layout (location=3) uniform int Kernel_size[3];                                                        \n"
    "                                                                                                       \n"
    "layout (binding=0,std430) buffer SSB                                                                   \n"
    "{                                                                                                      \n"
    "  float Coeff[768];                                                                                    \n"
    "};                                                                                                     \n"
    "                                                                                                       \n"
    "layout (binding = 0,rgba8ui) uniform uimage2D Image_orig;                                              \n"
    "layout (binding = 1,rgba32f) uniform image2D Image_orig_float;                                         \n"
    "layout (binding = 2,rgba32f) uniform image2D Image_gauss[6];                                           \n"
    "                                                                                                       \n"
    "                                                                                                       \n"
    "void main(void)                                                                                        \n"
    "{                                                                                                      \n"
    "  ivec2 Pos;                                                                                           \n"
    "  ivec2 Pos_aux1;                                                                                      \n"
    "  ivec2 Pos_aux2;                                                                                      \n"
    "  int i;                                                                                               \n"
    "  int j;                                                                                               \n"
    "                                                                                                       \n"
    "  Pos=ivec2(gl_FragCoord.xy);                                                                          \n"
    "                                                                                                       \n"
    "  for (i=0;i<3;i++){                                                                                   \n"
    "    vec4 Color_aux = vec4(0.0);                                                                        \n"
    "                                                                                                       \n"
    "    Pos_aux1=Pos-ivec2(Kernel_size[i]>> 1,0);                                                          \n"
    "                                                                                                       \n"
    "    for (j=0;j<Kernel_size[i];j++){                                                                    \n"
    "      Pos_aux2=Pos_aux1+ivec2(j,0);                                                                    \n"
    "      // check if the position is inside the image                                                     \n"
    "      if (Pos_aux2.x>=0 && Pos_aux2.x<Width && Pos_aux2.y>=0 && Pos_aux2.y<Height){                    \n"
    "        Color_aux+=imageLoad(Image_gauss[i],Pos_aux2)*Coeff[i*256+j];                                  \n"
    "      }                                                                                                \n"
    "      else{ // is outside                                                                              \n"
    "        if (Pos_aux2.x<0) Pos_aux2.x=0;                                                                \n"
    "        if (Pos_aux2.x>=Width) Pos_aux2.x=Width-1;                                                     \n"
    "        if (Pos_aux2.y<0) Pos_aux2.y=0;                                                                \n"
    "        if (Pos_aux2.y>Height) Pos_aux2.y=Height-1;                                                    \n"
    "        Color_aux+=imageLoad(Image_gauss[i],Pos_aux2)*Coeff[i*256+j];                                  \n"
    "      }                                                                                                \n"
    "    }                                                                                                  \n"
    "                                                                                                       \n"
    "    imageStore(Image_gauss[3+i],Pos,Color_aux);                                                        \n"
    "  }                                                                                                    \n"
    "}                                                                                                      \n"
  };

  static const GLchar *P2_fs[]={
    "#version 450 core                                                              \n"
    "                                                                               \n"
    "layout (binding = 0,rgba8ui) uniform uimage2D Image_orig;                      \n"
    "layout (binding = 1,rgba32f) uniform image2D Image_orig_float;                 \n"
    "layout (binding = 2,rgba32f) uniform image2D Image_gauss[6];                   \n"
    "                                                                               \n"
    "                                                                               \n"
    "void main(void)                                                                \n"
    "{                                                                              \n"
    "  int i;                                                                       \n"
    "  ivec2 Pos;                                                                   \n"
    "                                                                               \n"
    "  Pos=ivec2(gl_FragCoord.xy);                                                  \n"
    "                                                                               \n"
    "  vec4 Color_orig=log(imageLoad(Image_orig_float,Pos));                        \n"
    "  vec4 Color_gauss=vec4(0);                                                    \n"
    "                                                                               \n"
    "  for (i=0;i<3;i++){                                                           \n"
    "    Color_gauss+=0.33333333*(Color_orig-log(imageLoad(Image_gauss[3+i],Pos))); \n"
    "  }                                                                            \n"
    "  imageStore(Image_gauss[0],Pos,Color_gauss);                                  \n"
    "}                                                                              \n"
  };

  static const GLchar *P3_fs[]={
    "#version 450 core                                              \n"
    "                                                               \n"
    "uniform float Alpha=128.0;                                     \n"
    "uniform float Gain=1.0;                                        \n"
    "uniform vec4 Offset=vec4(0.0);                                 \n"
    "                                                               \n"
    "                                                               \n"
    "layout (binding = 0,rgba8ui) uniform uimage2D Image_orig;      \n"
    "layout (binding = 1,rgba32f) uniform image2D Image_orig_float; \n"
    "layout (binding = 2,rgba32f) uniform image2D Image_gauss[6];   \n"
    "                                                               \n"
    "void main(void)                                                \n"
    "{                                                              \n"
    "  ivec2 Pos;                                                   \n"
    "                                                               \n"
    "  Pos=ivec2(gl_FragCoord.xy);                                  \n"
    "                                                               \n"
    "  // original                                                  \n"
    "  vec4 Color_aux=imageLoad(Image_orig_float,Pos);              \n"
    " // retinex                                                    \n"
    "  vec4 Color_aux1=imageLoad(Image_gauss[0],Pos);               \n"
    "  float Log_rgb=log(Color_aux.x+Color_aux.y+Color_aux.z);      \n"
    "                                                               \n"
    "  Color_aux=log(Alpha*Color_aux)-vec4(Log_rgb);                \n"
    "  Color_aux=Gain*Color_aux*Color_aux1;                         \n"
    "  Color_aux=Color_aux+Offset;                                  \n"
    "                                                               \n"
    "  // the color restoration                                     \n"
    "  imageStore(Image_gauss[0],Pos,Color_aux);                    \n"
    "  // the square of it                                          \n"
    "  imageStore(Image_gauss[1],Pos,Color_aux*Color_aux);          \n"
    "}                                                              \n"
  };

  static const GLchar *P4_vs[]={
    "#version 450 core                                                                       \n"
    "                                                                                        \n"
    "layout (location=0) uniform int Step1;                                                  \n"
    "layout (location=1) uniform int Step2;                                                  \n"
    "                                                                                        \n"
    "                                                                                        \n"
    "layout(binding=1,std430) buffer SSB1                                                    \n"
    "{                                                                                       \n"
    "vec4 Data[];                                                                            \n"
    "};                                                                                      \n"
    "                                                                                        \n"
    "layout(binding=2,std430) buffer SSB2                                                    \n"
    "{                                                                                       \n"
    "vec4 Data_square[];                                                                     \n"
    "};                                                                                      \n"
    "                                                                                        \n"
    "                                                                                        \n"
    "void main()                                                                             \n"
    "{                                                                                       \n"
    "  // for the mean                                                                       \n"
    "  vec4 Suma=Data[gl_VertexID*Step1]+Data[gl_VertexID*Step1+Step2];                      \n"
    "  Data[gl_VertexID*Step1]=Suma;                                                         \n"
    "                                                                                        \n"
    "  // for the standar deviation                                                          \n"
    "  vec4 Suma_square=Data_square[gl_VertexID*Step1]+Data_square[gl_VertexID*Step1+Step2]; \n"
    "  Data_square[gl_VertexID*Step1]=Suma_square;                                           \n"
    "}                                                                                       \n"
  };

  static const GLchar *P4_fs[]={
    "#version 450 core \n"
    "                  \n"
    "                  \n"
    "void main(void)   \n"
    "{                 \n"
    "}                 \n"
  };

  static const GLchar *P5_fs[]={
    "#version 450 core                                              \n"
    "                                                               \n"
    "layout (location=0) uniform float Min;                         \n"
    "layout (location=1) uniform float Range;                       \n"
    "layout (location=2) uniform float Contrast;                    \n"
    "layout (location=3) uniform float Bright;                      \n"
    "                                                               \n"
    "layout (binding = 0,rgba8ui) uniform uimage2D Image_orig;      \n"
    "layout (binding = 1,rgba32f) uniform image2D Image_orig_float; \n"
    "layout (binding = 2,rgba32f) uniform image2D Image_gauss[6];   \n"
    "                                                               \n"
    "void main(void)                                                \n"
    "{                                                              \n"
    "  ivec2 Pos;                                                   \n"
    "                                                               \n"
    "  Pos=ivec2(gl_FragCoord.xy);                                  \n"
    "  // original                                                  \n"
    "  float Factor=255.0/Range;                                    \n"
    "  vec4 Color_aux=imageLoad(Image_gauss[0],Pos);                \n"
    "  vec4 Result=Factor*(Color_aux-vec4(Min));                    \n"
    "  Result=clamp(Result,0.0,255.0);                              \n"
    "  float Gray=Result.x*0.299+Result.y*0.587+Result.z*0.114;     \n"
    "  Gray=Contrast*Gray+Bright;                                   \n"
    "  Gray=clamp(Gray,0.0,255.0);                                  \n"
    "  Result=vec4(Gray,Gray,Gray,Gray);                            \n"
    "  imageStore(Image_gauss[0],Pos,clamp(Result,0.0,255.0));      \n"
    "}                                                              \n"
  };


  // Program for adding 1
  Program0=Shader.load_shaders(P0_vs,P0_fs);
  if (Program0==0){
    cout << "Error with program" << endl;
    exit(-1);
  }

  // Program for doing the gaussian pass 1
  Program1a=Shader.load_shaders(P0_vs,P1a_fs);
  if (Program1a==0){
   cout << "Error with program" << endl;
   exit(-1);
  }

  // Program for doing the gaussian pass 2
  Program1b=Shader.load_shaders(P0_vs,P1b_fs);
  if (Program1b==0){
   cout << "Error with program" << endl;
   exit(-1);
  }

  Program2=Shader.load_shaders(P0_vs,P2_fs);
  if (Program2==0){
   cout << "Error with program" << endl;
   exit(-1);
  }

  Program3=Shader.load_shaders(P0_vs,P3_fs);
  if (Program3==0){
   cout << "Error with program" << endl;
   exit(-1);
  }

  Program4=Shader.load_shaders(P4_vs,P4_fs);
  if (Program4==0){
   cout << "Error with program" << endl;
   exit(-1);
  }

  Program5=Shader.load_shaders(P0_vs,P5_fs);
  if (Program5==0){
   cout << "Error with program" << endl;
   exit(-1);
  }


//  cv::Mat Image;
//  cv::cvtColor(*Input_image_0,Image,CV_RGB2RGBA);

  // create the textures for the input images
  if (Tex_orig>0) glDeleteTextures(1,&Tex_orig);

  glCreateTextures(GL_TEXTURE_2D,1,&Tex_orig);
  glTextureStorage2D(Tex_orig,1,GL_RGBA8,Width1,Height1);
//  glTextureSubImage2D(Tex_orig,0,0,0,Width1,Height1,GL_RGBA,GL_UNSIGNED_BYTE,&Image.data[0]);

//  // buffer for gaussian coefficients. A SSBO is used
  if (Buffer_gaussians_coeffs>0) glDeleteBuffers(1,&Buffer_gaussians_coeffs);

  glGenBuffers(1, &Buffer_gaussians_coeffs);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, Buffer_gaussians_coeffs);
  glBufferData(GL_SHADER_STORAGE_BUFFER,3*256*sizeof(GLfloat),nullptr,GL_DYNAMIC_COPY);

  //
  if (Tex_orig_float>0) glDeleteTextures(1,&Tex_orig_float);

  glCreateTextures(GL_TEXTURE_2D,1,&Tex_orig_float);
  glTextureStorage2D(Tex_orig_float,1,GL_RGBA32F,Width1,Height1);
//  glTextureSubImage2D(Tex_orig,0,0,0,Width1,Height1,GL_RGB,GL_UNSIGNED_BYTE,&Image.data[0]);

//  GLfloat Data[]={254.0,0.0,0.0,0.0};
//  glClearTexImage(Tex_orig_float,0,GL_RGBA,GL_FLOAT,&Data[0]);

////  glGenTextures(1, &Tex_orig_float);
////  glBindTexture(GL_TEXTURE_2D,Tex_orig_float);
////  glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,Width1,Height1);

////  glGenTextures(6, &Tex_gaussian[0]);
////  for (unsigned int i=0;i<6;i++){
////    glBindTexture(GL_TEXTURE_2D,Tex_gaussian[i]);
////    glTexStorage2D(GL_TEXTURE_2D,1,GL_RGBA32F,Width1,Height1);
////  }
///

  for (unsigned int i=0;i<6;i++){
    if (Tex_gaussian[i]>0) glDeleteTextures(1,&Tex_gaussian[i]);
  }

  glCreateTextures(GL_TEXTURE_2D,6,&Tex_gaussian[0]);
  for (unsigned int i=0;i<6;i++){
    glTextureStorage2D(Tex_gaussian[i],1,GL_RGBA32F,Width1,Height1);
  }


  // buffers for obtaining the mean and the standard deviation
  // All the pixel are added and for each pixel the 3 components
  // It can be done very fast with the parallel prefix sum
  // The orinal number of values must be copied in a buffer that is power of 2
  float Log_size=log2(float(Width1*Height1));
  Size_buffer=int(Log_size);
  if ((Log_size-float(Size_buffer))>0) Size_buffer++;

  Size_buffer=int(powf(2,float(Size_buffer)));

  // Buffer_mean is for the addition of the normal values
  // It is binded in GL_PIXEL_PACK_BUFFER for copying the textures with glReadPixels or glGetTextureImage
  if (Buffer_mean>0) glDeleteBuffers(1,&Buffer_mean);

  glGenBuffers(1, &Buffer_mean);
  glBindBuffer(GL_PIXEL_PACK_BUFFER, Buffer_mean);
  glBufferData(GL_PIXEL_PACK_BUFFER,Size_buffer*4*sizeof(GLfloat),nullptr,GL_DYNAMIC_COPY);
  // clear to 0.0 because probably there is more positions than values
//  glClearNamedBufferSubData(Buffer_mean,GL_RGB32F,0,Size_buffer*3*sizeof(GLfloat),GL_RGB,GL_FLOAT,nullptr);
  if (Buffer_square>0) glDeleteBuffers(1,&Buffer_square);

  glGenBuffers(1, &Buffer_square);
  glBindBuffer(GL_PIXEL_PACK_BUFFER, Buffer_square);
  glBufferData(GL_PIXEL_PACK_BUFFER,Size_buffer*4*sizeof(GLfloat),nullptr,GL_DYNAMIC_COPY);
  // clear to 0.0
//  glClearNamedBufferSubData(Buffer_mean,GL_RGB32F,0,Size_buffer*3*sizeof(GLfloat),GL_RGB,GL_FLOAT,nullptr);
  // it is necessary to unbind otherwise the glReadPixels or glGetTextureImage cannot copy to local values
  glBindBuffer(GL_PIXEL_PACK_BUFFER,0);

  // At least one VAO
  glCreateVertexArrays(1,&VAO1);
  glBindVertexArray(VAO1);

  static const GLenum Buffers[]={GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2};
  if (Fbo_retinex>0) glDeleteFramebuffers(1,&Fbo_retinex);

//  glGenFramebuffers(1,&Fbo_retinex);
//  glBindFramebuffer(GL_FRAMEBUFFER,Fbo_retinex);
//  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,Tex_orig_float,0);
//  glDrawBuffers(1,Buffers);
//  glBindFramebuffer(GL_DRAW_FRAMEBUFFER,GL_widget->defaultFramebufferObject());
//  glBindFramebuffer(GL_FRAMEBUFFER,0);
}

//HEA

void _filter_retinex_c::reset_data()
{
  Vec_kernel_size[0]=RETINEX_SMALL_GAUSSIAN_KERNEL_SIZE_DEFAULT;
  Vec_kernel_size[1]=RETINEX_MEDIUM_GAUSSIAN_KERNEL_SIZE_DEFAULT;
  Vec_kernel_size[2]=RETINEX_BIG_GAUSSIAN_KERNEL_SIZE_DEFAULT;
  Num_kernels=RETINEX_NUM_KERNELS_DEFAULT;
  Color_restoration_variance=RETINEX_COLOR_RESTORATION_VARIANCE_DEFAULT;
  Mode=MODE_UNIFORM;
  Contrast=CONTRAST_DEFAULT;
  Bright=BRIGHT_DEFAULT;
}

//HEA

void _filter_retinex_c::small_gaussian_size(int Small_gaussian_size1)
{
  Vec_kernel_size[0]=Small_gaussian_size1;

  if (Input_image_0!=nullptr && (Input_image_0->rows<Vec_kernel_size[0] || Input_image_0->cols<Vec_kernel_size[0])){
    if (Input_image_0->rows<Input_image_0->cols) Vec_kernel_size[0]=Input_image_0->rows;
    else Vec_kernel_size[0]=Input_image_0->cols;
  }
}

//HEA

void _filter_retinex_c::medium_gaussian_size(int Medium_gaussian_size1)
{
  Vec_kernel_size[1]=Medium_gaussian_size1;

  if (Input_image_0!=nullptr && (Input_image_0->rows<Vec_kernel_size[1] || Input_image_0->cols<Vec_kernel_size[1])){
    if (Input_image_0->rows<Input_image_0->cols) Vec_kernel_size[1]=Input_image_0->rows;
    else Vec_kernel_size[1]=Input_image_0->cols;
  }
}

//HEA

void _filter_retinex_c::big_gaussian_size(int Big_gaussian_size1)
{
  Vec_kernel_size[2]=Big_gaussian_size1;

  if (Input_image_0!=nullptr && (Input_image_0->rows<Vec_kernel_size[2] || Input_image_0->cols<Vec_kernel_size[2])){
    if (Input_image_0->rows<Input_image_0->cols) Vec_kernel_size[2]=Input_image_0->rows;
    else Vec_kernel_size[2]=Input_image_0->cols;
  }
}

//HEA

void _filter_retinex_c::update()
{
//  GL_widget->makeCurrent();

  int Width1=Input_image_0->cols;
  int Height1=Input_image_0->rows;

//  cout << "width =" << Width1 << " " << Height1 << endl;

  if (Input_image_0->cols!=Output_image_0->cols || Input_image_0->rows!=Output_image_0->rows){
    Output_image_0->release();
    if (Input_image_0->channels()==1) Output_image_0->create(Height1,Width1,CV_8UC1);
    else Output_image_0->create(Height1,Width1,CV_8UC3);

    Input_changed=true;
  }

  if (Input_changed==true){
    create_buffers();
    Input_changed=false;
  }


  cv::Mat Image;
  cv::cvtColor(*Input_image_0,Image,CV_RGB2RGBA);
  glTextureSubImage2D(Tex_orig,0,0,0,Width1,Height1,GL_RGBA,GL_UNSIGNED_BYTE,&Image.data[0]);

  //****************************** PROGRAMS
//#define SHOW_DATA

  glBindVertexArray(VAO1);
//  glBindFramebuffer(GL_FRAMEBUFFER,Fbo_retinex);

//  cout << "fbo_retinex=" << Fbo_retinex << endl;


  // program to add 1 to the color value to remove the 0 values (for the log operation)
  glUseProgram(Program0);
  glBindImageTexture(0,Tex_orig,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA8UI);
  glBindImageTexture(1,Tex_orig_float,0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA32F);

  glViewport(0,0,Width1,Height1);
  // the results are in Tex_orig_float
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

//  vector<GLfloat> Data;
//  Data.resize(Width1*Height1*4*sizeof(GLfloat));

//  cout << "tex_orig_float=" << Tex_orig_float << endl;

//  glBindTexture(GL_TEXTURE_2D,Tex_orig_float);
//  glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_FLOAT,&Data[0]);

//  for (unsigned int i=0;i<3;i++){
//    cout << "i=" << i << " initial0=" << Data[i] << endl;
//  }

  // compute coefficients and put them in buffer
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,Buffer_gaussians_coeffs);

  GLfloat *Map=static_cast<GLfloat *>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER,0,3*256*sizeof(GLfloat),GL_MAP_WRITE_BIT));
  for (unsigned int i=0;i<3;i++){
    cv::Mat Coef=cv::getGaussianKernel(Vec_kernel_size[i],-1.0,CV_32F);
    for (int j=0;j<Vec_kernel_size[i];j++) Map[i*256+j]=Coef.at<float>(j);
    Coef.~Mat();
  }
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  // Program to apply the gaussian filter in 2 pass
  // this is for the first pass
  glUseProgram(Program1a);
  for (unsigned int i=0;i<6;i++){
    glBindImageTexture(2+i,Tex_gaussian[i],0,GL_FALSE,0,GL_READ_WRITE,GL_RGBA32F);
  }
  glViewport(0,0,Width1,Height1);
  glUniform1i(0,Width1);
  glUniform1i(1,Height1);
  // index of pass
  glUniform1i(2,0);
  glUniform1i(3,Vec_kernel_size[0]);
  glUniform1i(4,Vec_kernel_size[1]);
  glUniform1i(5,Vec_kernel_size[2]);
  // the results are in Tex_gaussian [0-2]
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


//  glBindTexture(GL_TEXTURE_2D,Tex_gaussian[0]);
//  glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_FLOAT,&Data[0]);

//  cout << "initial1=" << Data[0] << endl;


  // this is for the second pass
  glUseProgram(Program1b);
  glViewport(0,0,Width1,Height1);
  glUniform1i(0,Width1);
  glUniform1i(1,Height1);
  // index of pass
  glUniform1i(2,1);
  glUniform1i(3,Vec_kernel_size[0]);
  glUniform1i(4,Vec_kernel_size[1]);
  glUniform1i(5,Vec_kernel_size[2]);
  // the results are in Tex_gaussian [3-5]
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


//  vector<GLfloat> Data;
//  Data.resize(Width1*Height1*4*sizeof(GLfloat));

//  glBindTexture(GL_TEXTURE_2D,Tex_gaussian[3]);
//  glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_FLOAT,&Data[0]);

//  cout << "initial2=" << Data[0] << endl;

  glUseProgram(Program2);
  glViewport(0,0,Width1,Height1);
  // the results are in Tex_msr
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

//  glBindTexture(GL_TEXTURE_2D,Tex_gaussian[0]);
//  glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_FLOAT,&Data[0]);

//  cout << "initial3=" << Data[0] << endl;


  glUseProgram(Program3);
  glViewport(0,0,Width1,Height1);
  // the results are in Tex_color_restoration and Tex_square
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

//  glBindTexture(GL_TEXTURE_2D,Tex_gaussian[0]);
//  glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_FLOAT,&Data[0]);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

//  cout << "initial4=" << Data[0] << endl;


  // read the results of the color restoration for obtaining the mean. It is necessary that the buffer is binded to GL_PIXEL_PACK_BUFFER
  glClearNamedBufferSubData(Buffer_mean,GL_RGBA32F,0,Size_buffer*4*sizeof(GLfloat),GL_RGBA,GL_FLOAT,nullptr);

  glBindBuffer(GL_PIXEL_PACK_BUFFER, Buffer_mean);
  glGetTextureImage(Tex_gaussian[0],0,GL_RGBA,GL_FLOAT,Height1*Width1*4*sizeof(GLfloat),nullptr);

  // read the results of the square for obtaining the standard deviation
  glClearNamedBufferSubData(Buffer_square,GL_RGBA32F,0,Size_buffer*4*sizeof(GLfloat),GL_RGBA,GL_FLOAT,nullptr);

  glBindBuffer(GL_PIXEL_PACK_BUFFER, Buffer_square);
  glGetTextureImage(Tex_gaussian[1],0,GL_RGBA,GL_FLOAT,Height1*Width1*4*sizeof(GLfloat),nullptr);

  glBindBuffer(GL_PIXEL_PACK_BUFFER,0);

  // compute the mean and the standar deviation
  // The Buffer_mean was binded as GL_PIXEL_PACK_BUFFER to accept the values with the glReadpixels
  // Now it is binded as GL_SHADER_STORAGE_BUFFER to do the addition with the parallel prefix sum method
  glUseProgram(Program4);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER,1,Buffer_mean);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER,2,Buffer_square);

  // The addition is done by adding in a hierarchical way
  // Variables to control each pass in the addition
  int Num_pass_addition=int(log2(Size_buffer)-1);
  int Num_elements;
  int Counter;
  int Step1;
  int Step2;

  Counter=0;
  for (int i=Num_pass_addition;i>=0;i--){
    Num_elements=int(pow(2,i));
    Step1=int(pow(2,Counter+1));
    Step2=int(pow(2,Counter));
    glUniform1i(0,Step1);
    glUniform1i(1,Step2);
    glDrawArrays(GL_POINTS, 0, Num_elements); // launch one thread for each pixel.
    // This is very important. Otherwise it won't run right
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    Counter++;
  }

  // read the final result for the mean
  float *Map1=static_cast<GLfloat *>(glMapNamedBufferRange(Buffer_mean,0,3*sizeof(GLfloat),GL_MAP_READ_BIT));
  float Mean=0;
  for (unsigned int i=0;i<3;i++){
    Mean+=Map1[i];
  }
  glUnmapNamedBuffer(Buffer_mean);

  Mean=Mean/(Width1*Height1*3);

//  cout << "Mean=" << Mean << endl;

  // read the final result for the standar deviation
  Map1=static_cast<GLfloat *>(glMapNamedBufferRange(Buffer_square,0,3*sizeof(GLfloat),GL_MAP_READ_BIT));
  float Standard_deviation=0;
  for (unsigned int i=0;i<3;i++){
    Standard_deviation+=Map1[i];
  }
  glUnmapNamedBuffer(Buffer_square);

  Standard_deviation=Standard_deviation/(Width1*Height1*3);

//  cout << "STD=" << Standard_deviation << endl;

  Standard_deviation=Standard_deviation-Mean*Mean;
  Standard_deviation=sqrt(Standard_deviation);
  // Compute the Max and Min values depending on the user variable
  float Min,Max,Range;

  Min=Mean-Color_restoration_variance*Standard_deviation;
  Max=Mean+Color_restoration_variance*Standard_deviation;

  Range=Max-Min;
  if (Range==0.0f) Range=1.0;


  // now compute the color adjust
  glUseProgram(Program5);
  glViewport(0,0,Width1,Height1);
  glUniform1f(0,float(Min));
  glUniform1f(1,float(Range));
  glUniform1f(2,Contrast);
  glUniform1f(3,float(Bright));
  // the results are in Tex_color_adjust
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  cv::Mat Image1;
  Image1.create(Height1,Width1,CV_32FC4);

  glBindTexture(GL_TEXTURE_2D,Tex_gaussian[0]);
//  glBindTexture(GL_TEXTURE_2D,Tex_orig_float);
  glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_FLOAT,&Image1.data[0]);

//  glBindFramebuffer(GL_DRAW_FRAMEBUFFER,GL_widget->defaultFramebufferObject());
  glBindVertexArray(0);


  // pass to opencv
  cv::Mat Image_out_uc;
  Image1.convertTo(Image_out_uc,CV_8U);

//  cv::Vec4f Pixel;
//  for (unsigned int i=0;i<Image1.total();i++){
//    Pixel=Image1.at<cv::Vec4f>(i);
//    Output_image_0->at<unsigned char>(i)=static_cast<unsigned char>(Pixel[0]);
//  }

  if (Output_image_0->channels()==1){
    cvtColor(Image_out_uc,*Output_image_0,CV_RGBA2GRAY,1);
  }
  else{
    cvtColor(Image_out_uc,*Output_image_0,CV_RGBA2RGB,1);
  }
}




//HEA

void _filter_retinex_c::color_restoration_variance(float Color_restoration_variance1)
{
  Color_restoration_variance=Color_restoration_variance1;
}

//HEA

void _filter_retinex_c::kernel_size(unsigned int Pos,int Value)
{
  if (int(Pos)<Num_kernels) Vec_kernel_size[Pos]=Value;
}

