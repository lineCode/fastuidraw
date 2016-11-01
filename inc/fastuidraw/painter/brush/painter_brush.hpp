/*!
 * \file painter_brush.hpp
 * \brief file painter_brush.hpp
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


#pragma once

#include <fastuidraw/util/reference_counted.hpp>
#include <fastuidraw/util/vecN.hpp>
#include <fastuidraw/util/matrix.hpp>
#include <fastuidraw/util/c_array.hpp>

#include <fastuidraw/image.hpp>
#include <fastuidraw/colorstop_atlas.hpp>
#include <fastuidraw/painter/brush/pen_params.hpp>
#include <fastuidraw/painter/brush/image_params.hpp>
#include <fastuidraw/painter/brush/linear_gradient_params.hpp>
#include <fastuidraw/painter/brush/radial_gradient_params.hpp>
#include <fastuidraw/painter/brush/repeat_window_params.hpp>
#include <fastuidraw/painter/brush/transformation_translation_params.hpp>
#include <fastuidraw/painter/brush/transformation_matrix_params.hpp>

namespace fastuidraw
{
/*!\addtogroup Painter
  @{
 */

  /*!
    A PainterBrush defines a brush for painting via Painter
    providng a color (see \ref pen()), optionally applying
    an image (see \ref image() and \ref sub_image()) and
    optionally applying a linear or radial gradient (see
    \ref linear_gradient() and \ref radial_gradient()).
    In addition, a tranformation can be optionally applied
    to the brush (see \ref transformation_translate(),
    transformation_matrix() and transformation()) and a
    repeat window can also be optionally applied (see
    \ref repeat_window()).
  */
  class PainterBrush
  {
  public:
    /*!
      Indicates that PainterPackedValue<PainterBrush> does
      NOT have a copy of the value that generated it.
     */
    typedef false_type packed_value_has_value;

    /*!
      Indicates that PainterPackedValue<PainterBrush> does
      have a value derived from PainterBrush value/
     */
    typedef true_type packed_value_has_derived_value;

    /*!
      The derived value type that a PainterPackedValue<PainterBrush>
      will store when created from a PainterBrush
     */
    class derived_value_type
    {
    public:
      derived_value_type(void);
      derived_value_type(const derived_value_type &obj);
      ~derived_value_type();

      derived_value_type&
      operator=(const derived_value_type &rhs);

      uint32_t
      shader(void) const;

      const reference_counted_ptr<const Image>&
      image(void) const;

      const reference_counted_ptr<const ColorStopSequenceOnAtlas>&
      colorstops(void) const;

    private:
      friend class PainterBrush;
      void *m_d;
    };

    /*!
      Enumeration describing the roles of the bits for
      PainterBrush::shader().
     */
    enum shader_bits
      {
        /*!
          Bit is up if image is present
         */
        image_bit,

        /*!
          Bit is up if a linear gradient is present
         */
        linear_gradient_bit,

        /*!
          bit is up if a radial gradient is present
         */
        radial_gradient_bit,

        /*!
          bit is up if a gradient is repeated
         */
        repeat_gradient_bit,

        /*!
          Bit up if the brush has a repeat window
         */
        repeat_window_bit,

        /*!
          Bit up if transformation 2x2 matrix is present
         */
        transformation_translation_bit,

        /*!
          Bit up is translation is present
         */
        transformation_matrix_bit,
      };

    /*!
      Masks generated from shader_bits, use these masks on the
      return value of PainterBrush::shader() to get what features
      are active on the brush.
     */
    enum shader_masks
      {
        /*!
          bit mask for if image is used in brush.
         */
        image_mask = FASTUIDRAW_MASK(image_bit, 1),

        /*!
          bit for if linear_gradient is used in brush
         */
        linear_gradient_mask = FASTUIDRAW_MASK(linear_gradient_bit, 1),

        /*!
          bit for if radial_gradient is used in brush
         */
        radial_gradient_mask = FASTUIDRAW_MASK(radial_gradient_bit, 1),

        /*!
          bit for if repeat_gradient is used in brush
         */
        repeat_gradient_mask = FASTUIDRAW_MASK(repeat_gradient_bit, 1),

        /*!
          bit for if repeat_window is used in brush
         */
        repeat_window_mask = FASTUIDRAW_MASK(repeat_window_bit, 1),

        /*!
          bit mask for if translation is used in brush
         */
        transformation_translation_mask = FASTUIDRAW_MASK(transformation_translation_bit, 1),

        /*!
          bit mask for if matrix is used in brush
         */
        transformation_matrix_mask = FASTUIDRAW_MASK(transformation_matrix_bit, 1),
      };

    /*!
      Enumeration giving the packing order for data of a brush.
      Each enumeration is an entry and when data is packed each
      entry starts on a multiple of the alignment (see
      PainterBackend::ConfigurationBase::alignment()) to the
      destination packing store.
     */
    enum packing_order_t
      {
        /*!
          Pen packed first via \ref PenParams.
         */
        pen_packing,

        /*!
          image packing via \ref ImageParams.
         */
        image_packing,

        /*!
          gradient packing via LinearGradientParams or
          RadialGradientParams.
         */
        gradient_packing,

        /*!
          repeat window packing via \ref RepeatWindowParams
         */
        repeat_window_packing,

        /*!
          transformation translation via \ref
          TransformationTranslationParams.
         */
        transformation_translation_packing,

        /*!
          transformation matrix via \ref
          TransformationMatrixParams.
         */
        transformation_matrix_packing,
      };

    /*!
      Ctor. Initializes the brush to have no image, no gradient,
      no repeat window and no transformation with the pen color
      as (1.0, 1.0, 1.0, 1.0) which is solid white.
     */
    PainterBrush(void)
    {}

    /*!
      Ctor. Initializes the brush to have no image, no gradient,
      no repeat window and no transformation with the given
      pen color.
      \param ppen_color inital pen color
     */
    PainterBrush(const vec4 &ppen_color)
    {
      m_pen.color(ppen_color);
    }

    /*!
      Reset the brush to initial conditions.
     */
    PainterBrush&
    reset(void);

    /*!
      Set the color to the pen, default value is (1.0, 1.0, 1.0, 1.0).
     */
    PainterBrush&
    pen(const vec4 &color)
    {
      m_pen.color(color);
      return *this;
    }

    /*!
      Set the color to the pen, default value is (1.0, 1.0, 1.0, 1.0).
     */
    PainterBrush&
    pen(float r, float g, float b, float a = 1.0f)
    {
      return pen(vec4(r, g, b, a));
    }

    /*!
      Sets the brush to have an image.
      \param im handle to image to use. If handle is invalid,
                then sets brush to not have an image.
      \param f filter to apply to image, only has effect if im
               is non-NULL
     */
    PainterBrush&
    image(const reference_counted_ptr<const Image> &im,
          enum ImageParams::filter_t f = ImageParams::filter_nearest);

    /*!
      Set the brush to source from a sub-rectangle of an image
      \param im handle to image to use
      \param xy top-left corner of sub-rectangle of image to use
      \param wh width and height of sub-rectangle of image to use
      \param f filter to apply to image, only has effect if im
               is non-NULL
     */
    PainterBrush&
    sub_image(const reference_counted_ptr<const Image> &im, uvec2 xy, uvec2 wh,
              enum ImageParams::filter_t f = ImageParams::filter_nearest);

    /*!
      Sets the brush to not have an image.
     */
    PainterBrush&
    no_image(void)
    {
      return image(reference_counted_ptr<const Image>());
    }

    /*!
      Sets the brush to have a linear gradient.
      \param cs color stops for gradient. If handle is invalid,
                then sets brush to not have a gradient.
      \param start_p start position of gradient
      \param end_p end position of gradient.
      \param repeat if true, repeats the gradient, if false then
                    clamps the gradient
     */
    PainterBrush&
    linear_gradient(const reference_counted_ptr<const ColorStopSequenceOnAtlas> &cs,
                    const vec2 &start_p, const vec2 &end_p, bool repeat);
    /*!
      Sets the brush to have a radial gradient.
      \param cs color stops for gradient. If handle is invalid,
                then sets brush to not have a gradient.
      \param start_p start position of gradient
      \param start_r starting radius of radial gradient
      \param end_p end position of gradient.
      \param end_r ending radius of radial gradient
      \param repeat if true, repeats the gradient, if false then
                    clamps the gradient
     */
    PainterBrush&
    radial_gradient(const reference_counted_ptr<const ColorStopSequenceOnAtlas> &cs,
                    const vec2 &start_p, float start_r,
                    const vec2 &end_p, float end_r, bool repeat);

    /*!
      Sets the brush to not have a gradient.
     */
    PainterBrush&
    no_gradient(void);

    /*!
      Sets the brush to have a translation in its transformation.
      \param p translation value for brush transformation
     */
    PainterBrush&
    transformation_translate(const vec2 &p);

    /*!
      Sets the brush to have a matrix in its transformation.
      \param m matrix value for brush transformation
     */
    PainterBrush&
    transformation_matrix(const float2x2 &m);

    /*!
      Sets the brush to have a matrix and translation in its
      transformation
      \param p translation value for brush transformation
      \param m matrix value for brush transformation
     */
    PainterBrush&
    transformation(const vec2 &p, const float2x2 &m)
    {
      transformation_translate(p);
      transformation_matrix(m);
      return *this;
    }

    /*!
      Sets the brush to have no translation in its transformation.
     */
    PainterBrush&
    no_transformation_translation(void);

    /*!
      Sets the brush to have no matrix in its transformation.
     */
    PainterBrush&
    no_transformation_matrix(void);

    /*!
      Sets the brush to have no transformation.
     */
    PainterBrush&
    no_transformation(void)
    {
      no_transformation_translation();
      no_transformation_matrix();
      return *this;
    }

    /*!
      Sets the brush to have a repeat window
      \param pos location of repeat window
      \param size of repeat window
     */
    PainterBrush&
    repeat_window(const vec2 &pos, const vec2 &size);

    /*!
      Sets the brush to not have a repeat window
     */
    PainterBrush&
    no_repeat_window(void);

    /*!
      Returns the length of the data needed to encode the brush.
      Data is padded to be multiple of alignment and also
      sub-data of brush is padded to be along alignment
      boundaries.
      \param alignment alignment of the data store, see
                       PainterBackend::ConfigurationBase::alignment(void) const
     */
    unsigned int
    data_size(unsigned int alignment) const;

    /*!
      Encodes the data. Data is packed in the order
      specified by \ref packing_order_t.
      Data is padded to be multiple of alignment and also
      sub-data of brush is padded to be along alignment
      boundaries.
      \param dst location to which to encode the brush
      \param alignment alignment of the data store, see
                       PainterBackend::ConfigurationBase::alignment(void) const
     */
    void
    pack_data(unsigned int alignment, c_array<generic_data> dst) const;

    /*!
      Returns the brush shader ID which when tested against the
      bit masks from \ref shader_masks tells what features are
      active in the brush. The shader is decoded as follows:

      - If shader() & \ref image_mask is non zero, then an image is applied.
      - If shader() & \ref gradient_mask is non-zero, then a gradient is applied.
        The gradient is a linear gradient if shader() & \ref radial_gradient_mask
        is zero and a radial gradient otherwise.
      - If shader() & \ref radial_gradient_mask is non-zero, then a radial
        gradient is applied. Note that if shader() & \ref radial_gradient_mask
        is non-zero, then shader() & \ref gradient_mask is also non-zero.
      - If shader() & \ref gradient_repeat_mask then the gradient is repeated
        instead of clamped. Note that if shader() & \ref gradient_repeat_mask
        is non-zero, then shader() & \ref gradient_mask is also non-zero.
      - If shader() & \ref repeat_window_mask is non-zero, then a repeat
        window is applied to the brush.
      - If shader() & \ref transformation_translation_mask is non-zero, then a
        translation is applied to the brush.
      - If shader() & \ref transformation_matrix_mask is non-zero, then a
        2x2 matrix is applied to the brush.
     */
    uint32_t
    shader(void) const;

    /*!
      Returns the derived values stored by PainterPackedValue<PainterBrush>
     */
    const derived_value_type&
    derived_value(void) const
    {
      return m_derived_value;
    }

    /*!
      Returns true if and only if passed image can
      be rendered correctly with the specified filter.
      \param im handle to image
      \param f image filter to which to with which test if
               im can be rendered
     */
    static
    bool
    filter_suitable_for_image(const reference_counted_ptr<const Image> &im,
                              enum ImageParams::filter_t f)
    {
      return ImageParams::filter_suitable_for_image(im, f);
    }

    /*!
      Returns the highest quality filter with which
      an image may be rendered.
      \param im image to which to query
     */
    static
    enum ImageParams::filter_t
    best_filter_for_image(const reference_counted_ptr<const Image> &im)
    {
      return ImageParams::best_filter_for_image(im);
    }

    /*!
      Returns the slack requirement for an image to
      be rendered correctly under a filter.
      \param f filter to query
     */
    static
    int
    slack_requirement(enum ImageParams::filter_t f)
    {
      return ImageParams::slack_requirement(f);
    }


  private:
    PenParams m_pen;
    ImageParams m_image;
    LinearGradientParams m_linear_gradient;
    RadialGradientParams m_radial_gradient;
    RepeatWindowParams m_repeat_window;
    TransformationTranslationParams m_translation;
    TransformationMatrixParams m_matrix;
    derived_value_type m_derived_value;
  };
/*! @} */
}
