/*!
 * \file path_util_private.cpp
 * \brief file path_util_private.cpp
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

#include <cmath>
#include <complex>
#include <fastuidraw/util/math.hpp>
#include <fastuidraw/painter/arc_stroked_point.hpp>
#include "path_util_private.hpp"

unsigned int
fastuidraw::detail::
number_segments_for_tessellation(float radius, float arc_angle,
                                 const TessellatedPath::TessellationParams &P)
{
  return number_segments_for_tessellation(arc_angle, P.m_threshhold / radius);
}

unsigned int
fastuidraw::detail::
number_segments_for_tessellation(float arc_angle, float distance_thresh)
{
  float needed_sizef, d, theta;

  d = t_max(1.0f - distance_thresh, 0.5f);
  theta = t_max(0.00001f, 0.5f * std::acos(d));
  needed_sizef = t_abs(arc_angle) / theta;

  /* we ask for one more than necessary, to ensure that we BEAT
   *  the tessellation requirement.
   */
  return 1 + fastuidraw::t_max(3u, static_cast<unsigned int>(needed_sizef));
}

float
fastuidraw::detail::
distance_to_line(const vec2 &q, const vec2 &p1, const vec2 &p2)
{
  vec2 delta(p2 - p1);
  float num, den;

  num = delta.y() * q.x() - delta.x() * q.y() - p2.x() * p1.y() - p1.x() * p2.y();
  den = delta.magnitudeSq();

  return t_sqrt(num / den);
}

void
fastuidraw::detail::
bouding_box_union_arc(const vec2 &center, float radius,
                      float start_angle, float end_angle,
                      BoundingBox<float> *dst)
{
  float delta_angle(start_angle - end_angle);
  float half_angle(0.5f * delta_angle), d;
  vec2 p0, p1, z, z0, z1;

  p0 = vec2(t_cos(start_angle), t_sin(start_angle));
  p1 = vec2(t_cos(end_angle), t_sin(end_angle));

  d = 1.0f - t_cos(delta_angle * 0.5f);
  z = vec2(t_cos(start_angle + half_angle), t_sin(start_angle + half_angle));
  z0 = p0 + d * z;
  z1 = p1 + d * z;

  dst->union_point(center + radius * p0);
  dst->union_point(center + radius * p1);
  dst->union_point(center + radius * z0);
  dst->union_point(center + radius * z1);
}

void
fastuidraw::detail::
compute_arc_join_size(unsigned int cnt,
                      unsigned int *out_vertex_cnt,
                      unsigned int *out_index_cnt)
{
  *out_vertex_cnt = 4 + cnt;
  *out_index_cnt = 3 * (2 + cnt);
}

void
fastuidraw::detail::
add_triangle_fan(unsigned int begin, unsigned int end,
                 fastuidraw::c_array<unsigned int> indices,
                 unsigned int &index_offset)
{
  for(unsigned int i = begin + 1; i < end - 1; ++i, index_offset += 3)
    {
      indices[index_offset + 0] = begin;
      indices[index_offset + 1] = i;
      indices[index_offset + 2] = i + 1;
    }
}

void
fastuidraw::detail::
pack_arc_join(ArcStrokedPoint pt, unsigned int count,
              vec2 n_start, float delta_angle, vec2 n_end,
              unsigned int depth,
              c_array<PainterAttribute> dst_pts,
              unsigned int &vertex_offset,
              c_array<PainterIndex> dst_indices,
              unsigned int &index_offset)
{
  std::complex<float> arc_start(n_start.x(), n_start.y());
  unsigned int i, center;
  float theta, per_element, beyond;
  float cv;
  fastuidraw::vec2 v;

  per_element = delta_angle / static_cast<float>(count);
  cv = fastuidraw::t_cos(per_element * 0.5);
  beyond = 1.0f / cv;
  center = vertex_offset;

  pt.m_offset_direction = fastuidraw::vec2(0.0f, 0.0f);
  pt.radius() = 0.0f;
  pt.arc_angle() = per_element;
  pt.m_packed_data = arc_stroked_point_pack_bits(0, fastuidraw::ArcStrokedPoint::offset_arc_join, depth);
  pt.pack_point(&dst_pts[vertex_offset]);
  ++vertex_offset;

  pt.m_offset_direction = n_start;
  pt.radius() = 0.0f;
  pt.arc_angle() = per_element;
  pt.m_packed_data = arc_stroked_point_pack_bits(1, fastuidraw::ArcStrokedPoint::offset_arc_join, depth);
  pt.pack_point(&dst_pts[vertex_offset]);
  ++vertex_offset;

  for (theta = 0.0f, i = 0; i <= count; ++i, theta += per_element)
    {
      float s, c;
      std::complex<float> cs_as_complex;

      c = fastuidraw::t_cos(theta);
      s = fastuidraw::t_sin(theta);
      cs_as_complex = std::complex<float>(c, s) * arc_start;

      pt.m_offset_direction = beyond * fastuidraw::vec2(cs_as_complex.real(),
                                                        cs_as_complex.imag());
      pt.radius() = 0.0f;
      pt.arc_angle() = per_element;
      pt.m_packed_data = arc_stroked_point_pack_bits(1, fastuidraw::ArcStrokedPoint::offset_arc_join, depth);
      pt.pack_point(&dst_pts[vertex_offset]);
      ++vertex_offset;
    }

  pt.m_offset_direction = n_end;
  pt.radius() = 0.0f;
  pt.arc_angle() = per_element;
  pt.m_packed_data = arc_stroked_point_pack_bits(1, fastuidraw::ArcStrokedPoint::offset_arc_join, depth);
  pt.pack_point(&dst_pts[vertex_offset]);
  ++vertex_offset;

  add_triangle_fan(center, vertex_offset, dst_indices, index_offset);
}
