/*!
 * \file painter_surface_gl_private.cpp
 * \brief file painter_surface_gl_private.cpp
 *
 * Copyright 2016 by Intel.
 *
 * Contact: kevin.rogovin@intel.com
 *
 * This Source Code Form is subject to the
 * terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with
 * this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 *
 * \author Kevin Rogovin <kevin.rogovin@intel.com>
 *
 */

#include "painter_surface_gl_private.hpp"
#include "tex_buffer.hpp"
#include "texture_gl.hpp"

/////////////////////////////
//SurfaceGLPrivate methods
fastuidraw::gl::detail::SurfaceGLPrivate::
SurfaceGLPrivate(GLuint texture,
                 const PainterBackendGL::SurfaceGL::Properties &props):
  m_clear_color(0.0f, 0.0f, 0.0f, 0.0f),
  m_properties(props),
  m_auxiliary_buffer(0),
  m_buffers(0),
  m_fbo(0),
  m_own_texture(texture != 0)
{
  m_buffers[buffer_color] = texture;
  m_viewport.m_dimensions = props.dimensions();
  m_viewport.m_origin = fastuidraw::ivec2(0, 0);
}

fastuidraw::gl::detail::SurfaceGLPrivate::
~SurfaceGLPrivate()
{
  if (!m_own_texture)
    {
      m_buffers[buffer_color] = 0;
    }
  glDeleteTextures(m_auxiliary_buffer.size(), m_auxiliary_buffer.c_ptr());
  glDeleteFramebuffers(m_fbo.size(), m_fbo.c_ptr());
  glDeleteTextures(m_buffers.size(), m_buffers.c_ptr());
}

fastuidraw::gl::PainterBackendGL::SurfaceGL*
fastuidraw::gl::detail::SurfaceGLPrivate::
surface_gl(const fastuidraw::reference_counted_ptr<fastuidraw::PainterBackend::Surface> &surface)
{
  PainterBackendGL::SurfaceGL *q;

  FASTUIDRAWassert(dynamic_cast<PainterBackendGL::SurfaceGL*>(surface.get()));
  q = static_cast<PainterBackendGL::SurfaceGL*>(surface.get());
  return q;
}

GLuint
fastuidraw::gl::detail::SurfaceGLPrivate::
auxiliary_buffer(enum auxiliary_buffer_t tp)
{
  if (!m_auxiliary_buffer[tp])
    {
      GLenum internalFormat;
      ClearImageSubData clearer;

      internalFormat = auxiliaryBufferInternalFmt(tp);
      glGenTextures(1, &m_auxiliary_buffer[tp]);
      FASTUIDRAWassert(m_auxiliary_buffer[tp] != 0u);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_auxiliary_buffer[tp]);
      tex_storage<GL_TEXTURE_2D>(true,
                                 internalFormat,
                                 m_properties.dimensions());

      clearer.clear<GL_TEXTURE_2D>(m_auxiliary_buffer[tp], 0,
                                   0, 0, 0, //origin
                                   m_properties.dimensions().x(), m_properties.dimensions().y(), 1, //dimensions
                                   format_from_internal_format(internalFormat),
                                   type_from_internal_format(internalFormat));
    }

  return m_auxiliary_buffer[tp];
}

GLuint
fastuidraw::gl::detail::SurfaceGLPrivate::
buffer(enum buffer_t tp)
{
  if (m_buffers[tp] == 0)
    {
      GLenum tex_target, tex_target_binding;
      GLenum internalFormat;
      GLint old_tex;

      tex_target = (m_properties.msaa() <= 1) ?
        GL_TEXTURE_2D :
        GL_TEXTURE_2D_MULTISAMPLE;

      tex_target_binding = (tex_target == GL_TEXTURE_2D) ?
        GL_TEXTURE_BINDING_2D :
        GL_TEXTURE_BINDING_2D_MULTISAMPLE;

      internalFormat = (tp == buffer_color) ?
        GL_RGBA8 :
        GL_DEPTH24_STENCIL8;

      glGetIntegerv(tex_target_binding, &old_tex);
      glGenTextures(1, &m_buffers[tp]);
      FASTUIDRAWassert(m_buffers[tp] != 0);
      glBindTexture(tex_target, m_buffers[tp]);

      if (tex_target == GL_TEXTURE_2D)
        {
          detail::tex_storage<GL_TEXTURE_2D>(true, internalFormat,
                                             m_properties.dimensions());
          glTexParameteri(tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
      else
        {
          glTexStorage2DMultisample(tex_target, m_properties.msaa(),
                                    internalFormat,
                                    m_properties.dimensions().x(),
                                    m_properties.dimensions().y(),
                                    GL_TRUE);
        }
      glBindTexture(tex_target, old_tex);
    }

  return m_buffers[tp];
}

GLuint
fastuidraw::gl::detail::SurfaceGLPrivate::
fbo(uint32_t tp)
{
  if (m_fbo[tp] == 0)
    {
      GLint old_fbo;
      GLenum tex_target;

      tex_target = (m_properties.msaa() <= 1) ?
        GL_TEXTURE_2D :
        GL_TEXTURE_2D_MULTISAMPLE;

      glGenFramebuffers(1, &m_fbo[tp]);
      FASTUIDRAWassert(m_fbo[tp] != 0);

      glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &old_fbo);
      glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo[tp]);

      glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                             tex_target, buffer(buffer_depth), 0);

      if (tp & fbo_color_buffer)
        {
          glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                 tex_target, buffer(buffer_color), 0);
        }
      else
        {
          FASTUIDRAWassert(m_properties.msaa() <= 1);
        }

      if (tp & fbo_auxiliary_buffer)
        {
          FASTUIDRAWassert(m_properties.msaa() <= 1);
          glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                                 tex_target, auxiliary_buffer(auxiliary_buffer_u8), 0);
        }
      glBindFramebuffer(GL_READ_FRAMEBUFFER, old_fbo);
    }
  return m_fbo[tp];
}

fastuidraw::c_array<const GLenum>
fastuidraw::gl::detail::SurfaceGLPrivate::
draw_buffers(uint32_t tp)
{
  if (m_draw_buffers[tp].empty())
    {
      m_draw_buffers[tp] = c_array<const GLenum>(m_draw_buffer_values[tp]);
      if (tp & fbo_color_buffer)
        {
          m_draw_buffer_values[tp][0] = GL_COLOR_ATTACHMENT0;
        }
      else
        {
          m_draw_buffer_values[tp][0] = GL_NONE;
        }

      if (tp & fbo_auxiliary_buffer)
        {
          m_draw_buffer_values[tp][1] = GL_COLOR_ATTACHMENT1;
        }
      else
        {
          m_draw_buffers[tp] = m_draw_buffers[tp].sub_array(0, 1);
          m_draw_buffer_values[tp][1] = GL_NONE;
        }
    }

  return m_draw_buffers[tp];
}

uint32_t
fastuidraw::gl::detail::SurfaceGLPrivate::
fbo_bits(enum PainterBackendGL::auxiliary_buffer_t aux,
         enum PainterBackendGL::compositing_type_t compositing)
{
  uint32_t tp(0u);
  if (compositing != PainterBackendGL::compositing_interlock)
    {
      tp |= fbo_color_buffer;
    }

  if (aux == PainterBackendGL::auxiliary_buffer_framebuffer_fetch)
    {
      tp |= fbo_auxiliary_buffer;
    }

  return tp;
}
