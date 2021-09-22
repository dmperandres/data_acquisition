#DEFINES = WINDOWS
DEFINES += LINUX
DEFINES += X64

#DEFINES+=DEBUG
#DEFINES+=REDIRECT

DEFINES+=ENGLISH
#DEFINES+=SPANISH


HEADERS+= \
    vertex.h \
    image_IO.h \
    random.h \
    line_edit.h \
    images_tab.h \
    glwidget.h \
    window.h \
    shaders.h \
    filter.h \
    filter_halftoning_ost.h \
    filter_stippling_ebg.h \
    filter_border_mask.h \
    triangulate.h \
#    test.h \
    upload.h \
    filter_retinex_c.h

SOURCES+= \
    image_IO.cc \
    random.cc \
    line_edit.cc \
    images_tab.cc \
    filter.cc \
    filter_halftoning_ost.cc \
    filter_stippling_ebg.cc \
    filter_retinex_c.cc \
    filter_border_mask.cc \
    triangulate.cc \
    glwidget.cc \
    window.cc \
    shaders.cc \
#    test.cc \
    upload.cc \
    main.cc


DEFINE_WINDOWS:HEADERS+=dirent_w.h


!linux {
INCLUDEPATH += "H:\CODIGO\OPENCV310X64\OPENCV\BUILD\INCLUDE"
INCLUDEPATH += "H:\CODIGO\OPENCV310X64\OPENCV\BUILD\INCLUDE\OPENCV2"
INCLUDEPATH += "H:\CODIGO\OPENCV310X64\OPENCV\BUILD\INCLUDE\OPENCV"
INCLUDEPATH += "H:\CODIGO\GLEW\INCLUDE"

LIBS += -lopengl32 -L"H:\CODIGO\OPENCV310X64\OPENCV\BUILD\X64\VC14\LIB" -lopencv_world310 -L"H:\CODIGO\GLEW\LIB\RELEASE\X64" -lglew32
}

linux {
# LINUX OPENCV3.1
INCLUDEPATH += /casa/dmartin/codigo/funciontecas/opencv3.1/include
INCLUDEPATH += /casa/dmartin/codigo/funciontecas/opencv3.1/include/opencv2
INCLUDEPATH += /casa/dmartin/codigo/funciontecas/opencv3.1/include/opencv
INCLUDEPATH += /casa/dmartin/codigo/funciontecas/glew/include


LIBS += \
-L/casa/dmartin/codigo/funciontecas/glew/lib -lGLEW \
    -L/casa/dmartin/codigo/funciontecas/opencv3.1/lib -lopencv_core -lopencv_highgui \
    -lopencv_imgproc -lopencv_imgcodecs \
    -L/casa/dmartin/codigo/funciontecas/glu/lib64 -lGLU \
    -L/usr/X11R6/lib64 -lGL
#    -L/usr/lib/ -lGL
}

CONFIG += c++17
QT += opengl
QT += svg
QT += network
