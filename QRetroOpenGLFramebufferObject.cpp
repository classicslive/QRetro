#include "QRetroOpenGLFramebufferObject.h"

#if QRETRO_HAVE_OPENGL

QRetroOpenGLFramebufferObject::QRetroOpenGLFramebufferObject(int width,
  int height, QOpenGLFramebufferObjectFormat format) :
  QOpenGLFramebufferObject(width, height, format)
{
  /*QOpenGLShader vert{ QOpenGLShader::Vertex };
  QOpenGLShader frag{ QOpenGLShader::Fragment };

  if (!vert.compileSourceFile("C:/test/test.vert"))
    return;
  if (!frag.compileSourceFile("C:/test/test.frag"))
    return;

  if (!m_Shader.addShader(&vert))
    return;
  if (!m_Shader.addShader(&frag))
    return;

  if (!m_Shader.link())
    return;*/
}

void QRetroOpenGLFramebufferObject::apply(void)
{
  QOpenGLFramebufferObject::bind();

  // Apply your shader program here
  m_Shader.bind();

  // Draw a quad to cover the entire framebuffer
  glBegin(GL_TRIANGLES);

  // First triangle
  glVertex2f(-1.0, -1.0); // bottom-left
  glVertex2f( 1.0, -1.0); // bottom-right
  glVertex2f( 1.0,  1.0); // top-right

  // Second triangle
  glVertex2f(-1.0, -1.0); // bottom-left
  glVertex2f( 1.0,  1.0); // top-right
  glVertex2f(-1.0,  1.0); // top-left

  glEnd();

  // Unbind the shader program
  m_Shader.release();
}

#endif
