#ifndef QRETRO_OPENGLFRAMEBUFFEROBJECT_H
#define QRETRO_OPENGLFRAMEBUFFEROBJECT_H

#if QRETRO_HAVE_OPENGL
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>

class QRetroOpenGLFramebufferObject : public QOpenGLFramebufferObject
{
public:
  QRetroOpenGLFramebufferObject(int width, int height,
                                QOpenGLFramebufferObjectFormat format);

  void apply(void);

private:
  QOpenGLShaderProgram m_Shader;
};

#endif
#endif
