//LIC

#ifndef SHADERS_H
#define SHADERS_H

#include <fstream>
#include <string>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "vertex.h"

#define GL_GLEXT_PROTOTYPES

//HEA

class _shaders
{
public:
  GLuint load_shaders(const GLchar **Vertex_shader_source,const GLchar **Fragment_shader_source);
};
#endif
