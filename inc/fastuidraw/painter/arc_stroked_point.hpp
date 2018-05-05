/*!
 * \file stroked_point.hpp
 * \brief file stroked_point.hpp
 *
 * Copyright 2018 by Intel.
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


#pragma once

#include <fastuidraw/util/vecN.hpp>
#include <fastuidraw/util/util.hpp>
#include <fastuidraw/painter/painter_attribute.hpp>

namespace fastuidraw  {

/*!\addtogroup Paths
 * @{
 */

/*!
 * \brief
 * An ArcStrokedPoint holds the data for a point of stroking
 * where the segments can be arcs of a circle. The upshot
 * is that a fragment shader will determine per-pixel coverage.
 * In addition, the data is so that changing the stroking width
 * or miter limit does not change the stroking data.
 */
class ArcStrokedPoint
{
public:
  enum offset_type_t
    {
      /*!
       * A point of an arc. Indicates the point is at the outer stroking
       * boundary of the arc when stroked (i.e. the distance between the
       * point and the center of the arc is given by R + S where R is
       * the radius of the arc and S is the radius of the stroking).
       */
      offset_arc_point_outer_stroking_boundary,

      /*!
       * A point of an arc. Indicates the point is at the inner stroking
       * boundary of the arc when stroked (i.e. the distance between the
       * point and the center of the arc is given by R - S where R is
       * the radius of the arc and S is the radius of the stroking).
       */
      offset_arc_point_inner_stroking_boundary,

      /*!
       * The point of an arc that extends beyond the outer stroking boundary.
       */
      offset_arc_point_beyond_outer_stroking_boundary,

      /*!
       * A point of an arc. When the stroking radius is smaller than the
       * arc radius, position of this point is the same as \ref
       * offset_arc_point_outer_stroking_boundary; when the stroking
       * radius is greater that the arc-radius the position is the
       * center of the arc.
       */
      offset_arc_point_outer_stroking_boundary_origin,

      /*!
       * A point of an arc. When the stroking radius is smaller than the
       * arc radius, position of this point is the same as \ref
       * offset_arc_point_inner_stroking_boundary; when the stroking
       * radius is greater that the arc-radius the position is the
       * center of the arc.
       */
      offset_arc_point_inner_stroking_boundary_origin,

      /*!
       * The point is part of a line-segment.
       */
      offset_line_segment,

      /*!
       * The point is a point of an arc-join
       */
      offset_arc_join,
    };

  /*!
   * \brief
   * Enumeration encoding of bits of \ref m_packed_data
   * common to all offset types.
   */
  enum packed_data_bit_layout_common_t
    {
      /*!
       * Bit0 for holding the offset_type() value
       * of the point.
       */
      offset_type_bit0 = 0,

      /*!
       * number of bits needed to hold the
       * offset_type() value of the point.
       */
      offset_type_num_bits = 4,

      /*!
       * Bit indicates that the point in on the stroking
       * boundary, only applies to \ref offset_arc_join
       * and \ref offset_line_segment. When the bit is up
       * indicates to push the point in the direction
       * of \ref m_offset_direction.
       */
      boundary_bit = offset_type_bit0 + offset_type_num_bits,

      /*!
       * Bit indicates that point is on the end of a
       * segment.
       */
      end_segment_bit = offset_type_bit0 + offset_type_num_bits,

      /*!
       * Bit0 for holding the depth() value
       * of the point
       */
      depth_bit0,

      /*!
       * number of bits needed to hold the
       * depth() value of the point.
       */
      depth_num_bits = 20,

      /*!
       * Number of bits used on common packed data
       */
      number_common_bits = depth_bit0 + depth_num_bits,
    };

  /*!
   * \brief
   * Enumeration holding bit masks generated from
   * values in \ref packed_data_bit_layout_common_t.
   */
  enum packed_data_bit_masks_t
    {
      /*!
       * Mask generated for \ref offset_type_bit0 and
       * \ref offset_type_num_bits
       */
      offset_type_mask = FASTUIDRAW_MASK(offset_type_bit0, offset_type_num_bits),

      /*!
       * Mask generated for \ref boundary_bit
       */
      boundary_mask = FASTUIDRAW_MASK(boundary_bit, 1),

      /*!
       * Mask generated for \ref end_segment_bit
       */
      end_segment_mask = FASTUIDRAW_MASK(end_segment_bit, 1),

      /*!
       * Mask generated for \ref depth_bit0 and \ref depth_num_bits
       */
      depth_mask = FASTUIDRAW_MASK(depth_bit0, depth_num_bits),
    };

  /*!
   * Give the position of the point on the path.
   */
  vec2 m_position;

  /*!
   * Gives the unit vector in which to push the point.
   * For those points that are arc's the location of
   * the center is always given by
   *   \ref m_position - \ref m_radius * \ref m_offset_direction
   */
  vec2 m_offset_direction;

  /*!
   * If a point from an arc-segment, gives the radius
   * of the arc.
   */
  float m_radius;

  /*!
   * If a point from an arc-segment, gives the angle
   * of the arc.
   */
  float m_arc_angle;

  /*!
   * Gives the distance of the point from the start
   * of the -edge- on which the point resides.
   */
  float m_distance_from_edge_start;

  /*!
   * Gives the distance of the point from the start
   * of the -contour- on which the point resides.
   */
  float m_distance_from_contour_start;

  /*!
   * Gives the length of the edge on which the
   * point lies. This value is the same for all
   * points along a fixed edge.
   */
  float m_edge_length;

  /*!
   * Gives the length of the contour open on which
   * the point lies. This value is the same for all
   * points along a fixed contour.
   */
  float m_open_contour_length;

  /*!
   * Gives the length of the contour closed on which
   * the point lies. This value is the same for all
   * points along a fixed contour.
   */
  float m_closed_contour_length;

  /*!
   * Bit field with data packed as according to
   * \ref packed_data_bit_layout_common_t, \ref
   * packed_data_bit_layout_rounded_join_t, \ref
   * packed_data_bit_adjustable_cap_t and \ref
   * packed_data_bit_sub_edge_t. See also,
   * \ref packed_data_bit_masks_t for bit masks
   * generated.
   */
  uint32_t m_packed_data;

  /*!
   * Provides the point type from a value of \ref m_packed_data.
   * The return value is one of the enumerations of
   * \ref offset_type_t.
   * \param packed_data_value value suitable for \ref m_packed_data
   */
  static
  enum offset_type_t
  offset_type(uint32_t packed_data_value)
  {
    uint32_t v;
    v = unpack_bits(offset_type_bit0, offset_type_num_bits, packed_data_value);
    return static_cast<enum offset_type_t>(v);
  }

  /*!
   * Provides the point type for the point. The return value
   * is one of the enumerations of \ref offset_type_t.
   */
  enum offset_type_t
  offset_type(void) const
  {
    return offset_type(m_packed_data);
  }

  /*!
   * When stroking the data, the depth test to only
   * pass when the depth value is -strictly- larger
   * so that a fixed pixel is not stroked twice by
   * a single path. The value returned by depth() is
   * a relative z-value for a vertex. The points
   * drawn first have the largest z-values.
   */
  uint32_t
  depth(void) const
  {
    return unpack_bits(depth_bit0, depth_num_bits, m_packed_data);
  }

  /*!
   * Pack the data of this \ref ArcStrokedPoint into a \ref
   * PainterAttribute. The packing is as follows:
   * - PainterAttribute::m_attrib0 .xy -> \ref m_position (float)
   * - PainterAttribute::m_attrib0 .zw -> \ref m_offset_direction (float)
   * - PainterAttribute::m_attrib1 .x  -> \ref m_distance_from_edge_start (float)
   * - PainterAttribute::m_attrib1 .y  -> \ref m_distance_from_contour_start (float)
   * - PainterAttribute::m_attrib1 .z  -> \ref m_radius (float)
   * - PainterAttribute::m_attrib1 .w  -> \ref m_arc_angle (float)
   * - PainterAttribute::m_attrib2 .x  -> \ref m_packed_data (uint)
   * - PainterAttribute::m_attrib2 .y  -> \ref m_edge_length (float)
   * - PainterAttribute::m_attrib2 .z  -> \ref m_open_contour_length (float)
   * - PainterAttribute::m_attrib2 .w  -> \ref m_closed_contour_length (float)
   *
   * \param dst PainterAttribute to which to pack
   */
  void
  pack_point(PainterAttribute *dst) const;

  /*!
   * Unpack an \ref ArcStrokedPoint from a \ref PainterAttribute.
   * \param dst point to which to unpack data
   * \param src PainterAttribute from which to unpack data
   */
  static
  void
  unpack_point(ArcStrokedPoint *dst, const PainterAttribute &src);
};


/*! @} */

}
