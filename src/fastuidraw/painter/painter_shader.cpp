/*!
 * \file painter_shader.cpp
 * \brief file painter_shader.cpp
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


#include <vector>
#include <fastuidraw/painter/painter_shader.hpp>

namespace
{

  class PainterShaderPrivate
  {
  public:
    PainterShaderPrivate(unsigned int num_sub_shaders):
      m_registered_to(nullptr),
      m_number_sub_shaders(num_sub_shaders),
      m_sub_shader_ID(0)
    {
    }

    fastuidraw::PainterShader::Tag m_tag;
    const fastuidraw::PainterShaderRegistrar *m_registered_to;

    //for when shader has sub-shaders
    unsigned int m_number_sub_shaders;

    //for when shader is a sub-shader
    fastuidraw::reference_counted_ptr<fastuidraw::PainterShader> m_parent;
    unsigned int m_sub_shader_ID;
  };
}

///////////////////////////////////////////
// fastuidraw::PainterShader methods
fastuidraw::PainterShader::
PainterShader(unsigned int num_sub_shaders)
{
  m_d = FASTUIDRAWnew PainterShaderPrivate(num_sub_shaders);
}

fastuidraw::PainterShader::
PainterShader(unsigned int sub_shader,
              reference_counted_ptr<PainterShader> parent)
{
  PainterShaderPrivate *d;
  m_d = d = FASTUIDRAWnew PainterShaderPrivate(1);

  FASTUIDRAWassert(parent);
  FASTUIDRAWassert(sub_shader < parent->number_sub_shaders());

  d->m_parent = parent;
  d->m_sub_shader_ID = sub_shader;
}

fastuidraw::PainterShader::
~PainterShader()
{
  PainterShaderPrivate *d;
  d = static_cast<PainterShaderPrivate*>(m_d);
  FASTUIDRAWdelete(d);
  m_d = nullptr;
}

uint32_t
fastuidraw::PainterShader::
sub_shader(void) const
{
  PainterShaderPrivate *d;
  d = static_cast<PainterShaderPrivate*>(m_d);
  return d->m_sub_shader_ID;
}

uint32_t
fastuidraw::PainterShader::
ID(void) const
{
  PainterShaderPrivate *d;
  d = static_cast<PainterShaderPrivate*>(m_d);
  FASTUIDRAWassert(d->m_registered_to != nullptr);
  return d->m_tag.m_ID;
}

uint32_t
fastuidraw::PainterShader::
group(void) const
{
  PainterShaderPrivate *d;
  d = static_cast<PainterShaderPrivate*>(m_d);
  FASTUIDRAWassert(d->m_registered_to != nullptr);
  return d->m_tag.m_group;
}

fastuidraw::PainterShader::Tag
fastuidraw::PainterShader::
tag(void) const
{
  PainterShaderPrivate *d;
  d = static_cast<PainterShaderPrivate*>(m_d);
  FASTUIDRAWassert(d->m_registered_to != nullptr);
  return d->m_tag;
}

unsigned int
fastuidraw::PainterShader::
number_sub_shaders(void) const
{
  PainterShaderPrivate *d;
  d = static_cast<PainterShaderPrivate*>(m_d);
  return d->m_number_sub_shaders;
}

const fastuidraw::reference_counted_ptr<fastuidraw::PainterShader>&
fastuidraw::PainterShader::
parent(void) const
{
  PainterShaderPrivate *d;
  d = static_cast<PainterShaderPrivate*>(m_d);
  return d->m_parent;
}

void
fastuidraw::PainterShader::
register_shader(Tag tg, const PainterShaderRegistrar *p)
{
  PainterShaderPrivate *d;
  d = static_cast<PainterShaderPrivate*>(m_d);
  FASTUIDRAWassert(d->m_registered_to == nullptr);
  FASTUIDRAWassert(!d->m_parent);
  d->m_tag = tg;
  d->m_registered_to = p;
}

void
fastuidraw::PainterShader::
set_group_of_sub_shader(uint32_t gr)
{
  PainterShaderPrivate *d;
  d = static_cast<PainterShaderPrivate*>(m_d);

  FASTUIDRAWassert(d->m_parent);
  PainterShaderPrivate *pd;
  pd = static_cast<PainterShaderPrivate*>(d->m_parent->m_d);

  /* the parent must all-ready be registered.
   */
  FASTUIDRAWassert(pd->m_registered_to != nullptr);

  //but this shader is not yet registered!
  FASTUIDRAWassert(d->m_registered_to == nullptr);

  d->m_registered_to = pd->m_registered_to;
  d->m_tag.m_ID = pd->m_tag.m_ID + d->m_sub_shader_ID;
  d->m_tag.m_group = gr;
}

const fastuidraw::PainterShaderRegistrar*
fastuidraw::PainterShader::
registered_to(void) const
{
  PainterShaderPrivate *d;
  d = static_cast<PainterShaderPrivate*>(m_d);
  return d->m_registered_to;
}
