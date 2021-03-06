#include <fstream>
#include <dirent.h>

#include <fastuidraw/painter/painter.hpp>
#include <fastuidraw/text/glyph_cache.hpp>
#include <fastuidraw/text/font_freetype.hpp>
#include <fastuidraw/text/glyph_selector.hpp>
#include <fastuidraw/gl_backend/gl_context_properties.hpp>

#include "sdl_painter_demo.hpp"
#include "PainterWidget.hpp"
#include "simple_time.hpp"
#include "PanZoomTracker.hpp"
#include "text_helper.hpp"
#include "read_path.hpp"
#include "ImageLoader.hpp"
#include "colorstop_command_line.hpp"
#include "command_line_list.hpp"
#include "cycle_value.hpp"
#include "text_helper.hpp"

#include "cell.hpp"
#include "table.hpp"
#include "random.hpp"

using namespace fastuidraw;

bool
compare_named_images(const named_image &lhs,
                     const named_image &rhs)
{
  return lhs.second < rhs.second;
}

class painter_cells:public sdl_painter_demo
{
public:
  painter_cells(void);

  ~painter_cells();

protected:
  void
  derived_init(int w, int h);

  void
  draw_frame(void);

  void
  handle_event(const SDL_Event &ev);

private:
  typedef std::pair<enum PainterEnums::composite_mode_t, std::string> named_composite_mode;
  typedef std::pair<enum PainterEnums::blend_w3c_mode_t, std::string> named_blend_mode;

  static
  void
  generate_random_colors(int count, std::vector<vec4> &out_values, bool force_opaque);

  static
  void
  dump_file(const std::string &filename, std::vector<std::string> &dest);

  void
  add_images(const std::string &filename, std::vector<named_image> &dest);

  void
  add_single_image(const std::string &filename, std::vector<named_image> &dest);

  void
  update_cts_params(void);

  command_line_argument_value<float> m_table_width, m_table_height;
  command_line_argument_value<int> m_num_cells_x, m_num_cells_y;
  command_line_argument_value<int> m_cell_group_size;
  command_line_argument_value<std::string> m_font;
  enumerated_command_line_argument_value<enum glyph_type> m_text_renderer;
  command_line_argument_value<int> m_text_renderer_realized_pixel_size;
  command_line_argument_value<float> m_pixel_size;
  command_line_argument_value<float> m_fps_pixel_size;
  command_line_list<std::string> m_strings;
  command_line_list<std::string> m_files;
  command_line_list<std::string> m_images;
  enumerated_command_line_argument_value<enum PainterBrush::image_filter> m_image_filter;
  command_line_argument_value<unsigned int> m_image_mipmap_level;
  command_line_argument_value<bool> m_use_atlas;
  command_line_argument_value<bool> m_draw_image_name;
  command_line_argument_value<int> m_num_background_colors;
  command_line_argument_value<bool> m_background_colors_opaque;
  command_line_argument_value<int> m_num_text_colors;
  command_line_argument_value<bool> m_text_colors_opaque;
  command_line_argument_value<int> m_num_rect_colors;
  command_line_argument_value<bool> m_rect_colors_opaque;
  command_line_argument_value<float> m_min_x_velocity, m_max_x_velocity;
  command_line_argument_value<float> m_min_y_velocity, m_max_y_velocity;
  command_line_argument_value<int> m_min_degree_per_second;
  command_line_argument_value<int> m_max_degree_per_second;
  command_line_argument_value<int> m_table_rotate_degrees_per_s;
  command_line_argument_value<float> m_change_stroke_width_rate;

  command_line_argument_value<int> m_num_frames;
  command_line_argument_value<int> m_skip_frames;
  command_line_argument_value<bool> m_init_show_all_table;
  command_line_argument_value<bool> m_init_table_rotating;
  command_line_argument_value<bool> m_init_table_clipped;
  command_line_argument_value<bool> m_init_cell_rotating;
  command_line_argument_value<bool> m_init_draw_text;
  command_line_argument_value<bool> m_init_draw_images;
  command_line_argument_value<float> m_init_stroke_width;
  command_line_argument_value<bool> m_init_anti_alias_stroking;

  CellSharedState m_cell_shared_state;
  TableParams m_table_params;
  PanZoomTrackerSDLEvent m_zoomer;
  Table *m_table;
  simple_time m_time, m_draw_timer;
  PainterPackedValue<PainterBrush> m_text_brush;

  unsigned int m_current_composite;
  std::vector<named_composite_mode> m_composite_labels;

  unsigned int m_current_blend;
  std::vector<named_blend_mode> m_blend_labels;

  int m_frame;
  uint64_t m_benchmark_time_us;
  simple_time m_benchmark_timer;
  std::vector<uint64_t> m_frame_times;
};

painter_cells::
painter_cells(void):
  m_table_width(800, "table_width", "Table Width", *this),
  m_table_height(600, "table_height", "Table Height", *this),
  m_num_cells_x(10, "num_cells_x", "Number of cells across", *this),
  m_num_cells_y(10, "num_cells_y", "Number of cells down", *this),
  m_cell_group_size(1, "cell_group_size", "width and height in number of cells for cell group size", *this),
  m_font(default_font(), "font", "File from which to take font", *this),
  m_text_renderer(fastuidraw::curve_pair_glyph,
                  enumerated_string_type<enum glyph_type>()
                  .add_entry("coverage", coverage_glyph, "coverage glyphs (i.e. alpha masks)")
                  .add_entry("distance_field", distance_field_glyph, "distance field glyphs")
                  .add_entry("curve_pair", curve_pair_glyph, "curve-pair glyphs"),
                  "text_renderer",
                  "Specifies how to render text", *this),
  m_text_renderer_realized_pixel_size(24,
                                      "text_renderer_stored_pixel_size_non_scalable",
                                      "Only has effect if text_renderer value is a text rendering value "
                                      "where the font data is not scalable (i.e. coverage). Specifies "
                                      "the value to realize the glyph data to render",
                                      *this),
  m_pixel_size(24.0f, "font_pixel_size", "Render size for text rendering", *this),
  m_fps_pixel_size(24.0f, "fps_font_pixel_size", "Render size for text rendering of fps", *this),
  m_strings("add_string", "add a string to use by the cells", *this),
  m_files("add_string_file", "add a string to use by a cell, taken from file", *this),
  m_images("add_image", "Add an image to use by the cells", *this),
  m_image_filter(PainterBrush::image_filter_nearest,
                 enumerated_string_type<enum PainterBrush::image_filter>()
                 .add_entry("nearest", PainterBrush::image_filter_nearest, "nearest filtering")
                 .add_entry("linear", PainterBrush::image_filter_linear, "(bi)linear filtering")
                 .add_entry("cubic", PainterBrush::image_filter_cubic, "(bi)cubic filtering"),
                 "image_filter",
                 "Specifies how to filter the images applied to the rects",
                 *this),
  m_image_mipmap_level(0, "image_mipmap_levels",
                       "Maximum level of mipmap filtering applied "
                       "(when use_atlas is true, this is clamped to "
                       "log2_color_tile_size)",
                       *this),
  m_use_atlas(true, "use_atlas",
              "If false, each image is realized as a texture; if "
              "GL_ARB_bindless_texture or GL_NV_bindless_texture "
              "is supported, the Image objects are realized as bindless "
              "texture, thus avoding draw breaks; if both of these "
              "extensions is not present, then images are realized as "
              "bound textures which means that a draw break will be present "
              "whenever the image changes, harming performance.",
              *this),
  m_draw_image_name(false, "draw_image_name", "If true draw the image name in each cell as part of the text", *this),
  m_num_background_colors(1, "num_background_colors", "Number of distinct background colors in cells", *this),
  m_background_colors_opaque(false, "background_colors_opaque",
                             "If true, all background colors for rects are forced to be opaque",
                             *this),
  m_num_text_colors(1, "num_text_colors", "Number of distinct text colors in cells", *this),
  m_text_colors_opaque(true, "text_colors_opaque",
                       "If true, all text colors are forced to be opaque",
                       *this),
  m_num_rect_colors(0, "num_rect_colors", "Number of distinct colors which modulates the image, 0 means no modulation", *this),
  m_rect_colors_opaque(true, "rect_colors_opaque",
                       "If true, all rect colors are forced to be opaque",
                       *this),
  m_min_x_velocity(-10.0f, "min_x_velocity", "Minimum x-velocity for cell content in pixels/s", *this),
  m_max_x_velocity(+10.0f, "max_x_velocity", "Maximum x-velocity for cell content in pixels/s", *this),
  m_min_y_velocity(-10.0f, "min_y_velocity", "Minimum y-velocity for cell content in pixels/s", *this),
  m_max_y_velocity(+10.0f, "max_y_velocity", "Maximum y-velocity for cell content in pixels/s", *this),
  m_min_degree_per_second(60, "min_degree_velocity", "max rotation speed in degrees/second", *this),
  m_max_degree_per_second(60, "max_degree_velocity", "max rotation speed in degrees/second", *this),
  m_table_rotate_degrees_per_s(20, "table_degree_velocity", "rotation speed of table in degrees/second", *this),
  m_change_stroke_width_rate(10.0f, "change_stroke_width_rate",
                             "rate of change in pixels/sec for changing stroke width "
                             "when changing stroke when key is down",
                             *this),
  m_num_frames(-1, "num_frames",
               "If positive, then run demo in benchmark mode terminating after the given number of frames",
               *this),
  m_skip_frames(1, "num_skip_frames",
                "If num_frames > 0, then gives the number of frames to ignore in benchmarking",
                *this),
  m_init_show_all_table(true, "init_show_all_table",
                        "If true, initialize scroll and zoom to show entire table",
                        *this),
  m_init_table_rotating(false, "init_table_rotating",
                        "If true, initialize table to be rotating",
                        *this),
  m_init_table_clipped(false, "init_table_clipped",
                       "If true, initialize to enable clipping on the table",
                       *this),
  m_init_cell_rotating(false, "init_cell_rotating",
                       "If true, intialize to have cells rotating",
                       *this),
  m_init_draw_text(true, "init_draw_text",
                   "If true, intialize to draw text in cells",
                   *this),
  m_init_draw_images(true, "init_draw_image",
                   "If true, intialize to draw image in cells",
                   *this),
  m_init_stroke_width(10.0f, "init_stroke_width",
                      "Initial value for stroking width",
                      *this),
  m_init_anti_alias_stroking(true, "init_antialias_stroking",
                             "Initial value for anti-aliasing for stroking",
                             *this),
  m_table(nullptr),
  m_current_composite(0),
  m_current_blend(0)
{
  std::cout << "Controls:\n"
            << "\t[: decrease stroke width(hold left-shift for slower rate and right shift for faster)\n"
            << "\t]: increase stroke width(hold left-shift for slower rate and right shift for faster)\n"
            << "\ta: toggle anti-aliasing of stroking\n"
            << "\tp: pause cell rotate\n"
            << "\t0: set zoom factor to 1.0\n"
            << "\tc: toggle clipping of table\n"
            << "\tv: toggle table rotating\n"
            << "\tr: toggle rotating individual cells\n"
            << "\tt: toggle draw cell text\n"
            << "\ti: toggle draw cell image\n"
            << "\tb: cycle composite mode applied to image rect\n"
            << "\tctrl-b: cycle blend mode applied to image rect\n"
            << "\tLeft Mouse Drag: pan\n"
            << "\tHold Left Mouse, then drag up/down: zoom out/in\n";

}

painter_cells::
~painter_cells()
{
  if (m_table != nullptr)
    {
      FASTUIDRAWdelete(m_table);
    }
}

void
painter_cells::
generate_random_colors(int count, std::vector<vec4> &out_values, bool force_opaque)
{
  out_values.resize(count);
  for(int i = 0; i < count; ++i)
    {
      out_values[i] = random_value(vec4(0.0f, 0.0f, 0.0f, 0.2f),
                                   vec4(1.0f, 1.0f, 1.0f, 0.8f));
      if (force_opaque)
        {
          out_values[i].w() = 1.0f;
        }
    }
}

void
painter_cells::
dump_file(const std::string &filename, std::vector<std::string> &dest)
{
  std::ifstream istr(filename.c_str());
  if (istr)
    {
      std::ostringstream str;
      str << istr.rdbuf();
      dest.push_back(str.str());
    }
}

void
painter_cells::
add_images(const std::string &filename, std::vector<named_image> &dest)
{
  DIR *dir;
  struct dirent *entry;

  dir = opendir(filename.c_str());
  if (!dir)
    {
      add_single_image(filename, dest);
      return;
    }

  for(entry = readdir(dir); entry != nullptr; entry = readdir(dir))
    {
      std::string file;
      file = entry->d_name;
      if (file != ".." && file != ".")
        {
          add_images(filename + "/" + file, dest);
        }
    }
  closedir(dir);
}

void
painter_cells::
add_single_image(const std::string &filename, std::vector<named_image> &dest)
{
  ImageLoader image_data(filename);
  if (image_data.non_empty())
    {
      reference_counted_ptr<const Image> im;

      std::cout << "\tImage \"" << filename << "\" of size "
                << image_data.dimensions() << " loaded";

      if (m_use_atlas.value())
        {
          int slack(0);

          slack = PainterBrush::slack_requirement(m_image_filter.value());
          im = Image::create(m_painter->image_atlas(),
                             image_data.width(), image_data.height(),
                             image_data, slack);
        }
      else
        {
          im = create_texture_image(image_data.width(),
                                    image_data.height(),
                                    image_data.num_mipmap_levels(),
                                    image_data);
        }

      switch (im->type())
        {
        case Image::on_atlas:
          std::cout << " on atlas with slack = " << im->slack()
                    << ", number_mipmap_levels = "
                    << im->number_mipmap_levels();
          break;
        case Image::bindless_texture2d:
          std::cout << " bindlessly";
          break;
        case Image::context_texture2d:
          std::cout << " as bound texture (WARNING: large performance impact expected)";
          break;
        }

      std::cout << " @" << im.get() << ".\n";
      dest.push_back(named_image(im, filename));
    }
}

void
painter_cells::
derived_init(int w, int h)
{
  m_table_params.m_wh = vec2(m_table_width.value(), m_table_height.value());
  m_table_params.m_cell_count = ivec2(m_num_cells_x.value(), m_num_cells_y.value());
  m_table_params.m_line_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
  m_table_params.m_cell_state = &m_cell_shared_state;
  m_table_params.m_zoomer = &m_zoomer;
  m_table_params.m_draw_image_name = m_draw_image_name.value();
  m_table_params.m_table_rotate_degrees_per_s = m_table_rotate_degrees_per_s.value();
  m_table_params.m_timer_based_animation = (m_num_frames.value() <= 0);

  reference_counted_ptr<FreeTypeFace::GeneratorBase> gen;
  gen = FASTUIDRAWnew FreeTypeFace::GeneratorMemory(m_font.value().c_str(), 0);
  m_table_params.m_glyph_selector = m_glyph_selector;
  if (gen->check_creation() == routine_success)
    {
      m_table_params.m_font = FASTUIDRAWnew FontFreeType(gen,
                                                         FontFreeType::RenderParams(),
                                                         m_ft_lib);
    }
  else
    {
      std::cout << "\n-----------------------------------------------------"
                << "\nWarning: unable to create font from file \""
                << m_font.value() << "\"\n"
                << "-----------------------------------------------------\n";
    }

  if (!fastuidraw::GlyphRender::scalable(m_text_renderer.value()))
    {
      fastuidraw::GlyphRender r(m_text_renderer_realized_pixel_size.value());
      r.m_type = m_text_renderer.value();
      m_table_params.m_text_render = r;
    }
  else
    {
      m_table_params.m_text_render = GlyphRender(m_text_renderer.value());
    }
  m_table_params.m_pixel_size = m_pixel_size.value();

  m_table_params.m_texts.reserve(m_strings.size() + m_files.size());
  for(const auto &S : m_strings)
    {
      m_table_params.m_texts.push_back(S);
    }

  for(const auto &S : m_files)
    {
      dump_file(S, m_table_params.m_texts);
    }

  for(const auto &S : m_images)
    {
      add_images(S, m_table_params.m_images);
    }
  std::cout << "Loaded " << m_table_params.m_images.size() << " images total\n";
  std::sort(m_table_params.m_images.begin(),
            m_table_params.m_images.end(),
            compare_named_images);
  m_table_params.m_image_filter = m_image_filter.value();
  m_table_params.m_image_mipmap_level = m_image_mipmap_level.value();

  generate_random_colors(m_num_background_colors.value(), m_table_params.m_background_colors,
                         m_background_colors_opaque.value());
  generate_random_colors(m_num_text_colors.value(), m_table_params.m_text_colors,
                         m_text_colors_opaque.value());
  generate_random_colors(m_num_rect_colors.value(), m_table_params.m_rect_colors,
                         m_rect_colors_opaque.value());

  if (m_table_params.m_images.empty() && m_table_params.m_rect_colors.empty())
    {
      vec4 C(0.2f, 0.7f, 0.7f, 0.6f);
      m_table_params.m_rect_colors.push_back(C);
    }

  m_table_params.m_min_speed = vec2(m_min_x_velocity.value(), m_min_y_velocity.value());
  m_table_params.m_max_speed = vec2(m_max_x_velocity.value(), m_max_y_velocity.value());
  m_table_params.m_min_degrees_per_s = m_min_degree_per_second.value();
  m_table_params.m_max_degrees_per_s = m_max_degree_per_second.value();

  if (m_cell_group_size.value() > 0)
    {
      m_table_params.m_max_cell_group_size = m_cell_group_size.value();
    }
  else
    {
      m_table_params.m_max_cell_group_size = 2 * std::max(m_num_cells_x.value(), m_num_cells_y.value());
    }

  m_table = FASTUIDRAWnew Table(m_table_params);
  m_table->m_clipped = m_init_table_clipped.value();
  m_table->m_rotating = m_init_table_rotating.value();
  m_cell_shared_state.m_draw_text = m_init_draw_text.value();
  m_cell_shared_state.m_draw_image = m_init_draw_images.value();
  m_cell_shared_state.m_rotating = m_init_cell_rotating.value();
  m_cell_shared_state.m_stroke_width = m_init_stroke_width.value();
  m_cell_shared_state.m_anti_alias_stroking = m_init_anti_alias_stroking.value();

  /* init m_zoomer so that table contents fit into screen.
   */
  vec2 twh;
  ScaleTranslate<float> tr1, tr2;

  twh = m_table_params.m_wh / vec2(w, h);
  tr1.translation(-0.5f * m_table_params.m_wh);
  tr2.translation(0.5f * vec2(w, h));

  if (m_init_show_all_table.value())
    {
      ScaleTranslate<float> sc;
      sc.scale(1.0f / std::max(twh.x(), twh.y()));
      m_zoomer.transformation(tr2 * sc * tr1);
    }
  else
    {
      m_zoomer.transformation(tr2 * tr1);
    }


  if (m_table_params.m_font)
    {
      std::cout << "Font: " << m_table_params.m_font->properties() << "\n";
    }
  else
    {
      std::cout << "Font: nullptr\n";
    }

  std::cout << "Window resolution = " << dimensions() << "\n";

  m_frame = -m_skip_frames.value();
  if (m_num_frames.value() > 0)
    {
      m_frame_times.reserve(m_num_frames.value());
    }

#define ADD_COMPOSITE_MODE(X)                                           \
  m_composite_labels.push_back(named_composite_mode(PainterEnums::X, #X))

  ADD_COMPOSITE_MODE(composite_porter_duff_src_over);
  ADD_COMPOSITE_MODE(composite_porter_duff_clear);
  ADD_COMPOSITE_MODE(composite_porter_duff_src);
  ADD_COMPOSITE_MODE(composite_porter_duff_dst);
  ADD_COMPOSITE_MODE(composite_porter_duff_dst_over);
  ADD_COMPOSITE_MODE(composite_porter_duff_src_in);
  ADD_COMPOSITE_MODE(composite_porter_duff_dst_in);
  ADD_COMPOSITE_MODE(composite_porter_duff_src_out);
  ADD_COMPOSITE_MODE(composite_porter_duff_dst_out);
  ADD_COMPOSITE_MODE(composite_porter_duff_src_atop);
  ADD_COMPOSITE_MODE(composite_porter_duff_dst_atop);
  ADD_COMPOSITE_MODE(composite_porter_duff_xor);

#define ADD_BLEND_MODE(X) do {                                          \
    if (m_painter->default_shaders().blend_shaders().shader(PainterEnums::X)) \
      {                                                                 \
        m_blend_labels.push_back(named_blend_mode(PainterEnums::X, #X)); \
      }                                                                 \
  } while(0)

  ADD_BLEND_MODE(blend_w3c_normal);
  ADD_BLEND_MODE(blend_w3c_multiply);
  ADD_BLEND_MODE(blend_w3c_screen);
  ADD_BLEND_MODE(blend_w3c_overlay);
  ADD_BLEND_MODE(blend_w3c_darken);
  ADD_BLEND_MODE(blend_w3c_lighten);
  ADD_BLEND_MODE(blend_w3c_color_dodge);
  ADD_BLEND_MODE(blend_w3c_color_burn);
  ADD_BLEND_MODE(blend_w3c_hardlight);
  ADD_BLEND_MODE(blend_w3c_softlight);
  ADD_BLEND_MODE(blend_w3c_difference);
  ADD_BLEND_MODE(blend_w3c_exclusion);
  ADD_BLEND_MODE(blend_w3c_hue);
  ADD_BLEND_MODE(blend_w3c_saturation);
  ADD_BLEND_MODE(blend_w3c_color);
  ADD_BLEND_MODE(blend_w3c_luminosity);
}

void
painter_cells::
update_cts_params(void)
{
  const Uint8 *keyboard_state = SDL_GetKeyboardState(nullptr);
  FASTUIDRAWassert(keyboard_state != nullptr);

  float speed = static_cast<float>(m_draw_timer.restart()) * 0.001f;

  if (keyboard_state[SDL_SCANCODE_LSHIFT])
    {
      speed *= 0.1f;
    }
  if (keyboard_state[SDL_SCANCODE_RSHIFT])
    {
      speed *= 10.0f;
    }

  if (keyboard_state[SDL_SCANCODE_RIGHTBRACKET])
    {
      m_cell_shared_state.m_stroke_width += m_change_stroke_width_rate.value() * speed / m_zoomer.transformation().scale();
    }

  if (keyboard_state[SDL_SCANCODE_LEFTBRACKET])
    {
      m_cell_shared_state.m_stroke_width -= m_change_stroke_width_rate.value()  * speed / m_zoomer.transformation().scale();
      m_cell_shared_state.m_stroke_width = std::max(m_cell_shared_state.m_stroke_width, 0.0f);
    }
}

void
painter_cells::
draw_frame(void)
{
  uint64_t ms, us;
  us = m_time.restart_us();
  ms = us / 1000;

  if (m_frame == 0)
    {
      m_benchmark_timer.restart();
    }
  else if (m_frame > 0)
    {
      m_frame_times.push_back(us);
    }

  if (m_num_frames.value() > 0 && m_frame == m_num_frames.value())
    {
      m_benchmark_time_us = m_benchmark_timer.elapsed_us();
      std::cout << "Frame times(in us):\n";
      for(unsigned int i = 0, endi = m_frame_times.size(); i < endi; ++i)
        {
          std::cout << m_frame_times[i] << " us\n";
        }
      std::cout << "Did " << m_num_frames.value() << " frames in "
                << m_benchmark_time_us << "us, average time = "
                << static_cast<float>(m_benchmark_time_us) / static_cast<float>(m_frame)
                << "us\n " << 1000.0f * 1000.0f * static_cast<float>(m_frame) / static_cast<float>(m_benchmark_time_us)
                << " FPS\n";
      end_demo(0);
      return;
    }

  update_cts_params();
  m_cell_shared_state.m_cells_drawn = 0;

  m_surface->clear_color(vec4(0.5f, 0.5f, 0.5f, 1.0f));
  m_painter->begin(m_surface);

  ivec2 wh(dimensions());
  float3x3 proj(float_orthogonal_projection_params(0, wh.x(), wh.y(), 0));
  m_painter->transformation(proj);

  m_painter->save();
  m_painter->translate(m_zoomer.transformation().translation());
  m_painter->scale(m_zoomer.transformation().scale());
  /* update m_bb_min and m_bb_max as the window corners in
     coordinates; Table will do the right thing with these numbers
     to change them
   */
  m_table->m_bb_min = vec2(0.0f, 0.0f);
  m_table->m_bb_max = vec2(wh);

  m_table->paint(m_painter);
  m_painter->restore();


  if (m_table_params.m_timer_based_animation)
    {
      std::ostringstream ostr;

      ostr << "FPS = ";
      if (us > 0)
        {
          ostr << static_cast<int>(1000.0f * 1000.0f / static_cast<float>(us));
        }
      else
        {
          ostr << "NAN";
        }
      ostr << "\nms = " << ms
           << "\nDrew " << m_cell_shared_state.m_cells_drawn << " cells"
           << "\nAttribs: "
           << m_painter->query_stat(PainterPacker::num_attributes)
           << "\nIndices: "
           << m_painter->query_stat(PainterPacker::num_indices)
           << "\nGenericData: "
           << m_painter->query_stat(PainterPacker::num_generic_datas)
           << "\nHeaders: "
           << m_painter->query_stat(PainterPacker::num_headers)
           << "\n";
      if (!m_text_brush)
        {
          PainterBrush brush;
          brush.pen(0.0f, 1.0f, 1.0f, 1.0f);
          m_text_brush = m_painter->packed_value_pool().create_packed_value(brush);
        }
      draw_text(ostr.str(), m_fps_pixel_size.value(),
                m_table_params.m_font, m_table_params.m_text_render,
                PainterData(m_text_brush));
    }

  m_painter->end();
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  m_surface->blit_surface(GL_NEAREST);

  ++m_frame;
}

void
painter_cells::
handle_event(const SDL_Event &ev)
{
  m_zoomer.handle_event(ev);

  switch(ev.type)
    {
    case SDL_QUIT:
      end_demo(0);
      break;

    case SDL_WINDOWEVENT:
      if (ev.window.event == SDL_WINDOWEVENT_RESIZED)
        {
          on_resize(ev.window.data1, ev.window.data2);
        }
      break;

    case SDL_KEYUP:
      switch(ev.key.keysym.sym)
        {
        case SDLK_ESCAPE:
          end_demo(0);
          break;
        case SDLK_a:
          if (m_cell_shared_state.m_stroke_width > 0.0f)
            {
              m_cell_shared_state.m_anti_alias_stroking = !m_cell_shared_state.m_anti_alias_stroking;
              std::cout << "Stroking anti-aliasing = " << m_cell_shared_state.m_anti_alias_stroking << "\n";
            }
          break;
        case SDLK_v:
          m_table->m_rotating = !m_table->m_rotating;
          std::cout << "Table Rotating = " << m_table->m_rotating << "\n";
          break;
        case SDLK_c:
          m_table->m_clipped = !m_table->m_clipped;
          std::cout << "Table clipped = " << m_table->m_clipped << "\n";
          break;
        case SDLK_p:
          m_cell_shared_state.m_pause = !m_cell_shared_state.m_pause;
          std::cout << "Paused = " << m_cell_shared_state.m_pause << "\n";
          break;
        case SDLK_r:
          m_cell_shared_state.m_rotating = !m_cell_shared_state.m_rotating;
          std::cout << "Cell Rotating = " << m_cell_shared_state.m_rotating << "\n";
          break;
        case SDLK_t:
          m_cell_shared_state.m_draw_text = !m_cell_shared_state.m_draw_text;
          std::cout << "Draw Text = " << m_cell_shared_state.m_draw_text << "\n";
          break;
        case SDLK_i:
          m_cell_shared_state.m_draw_image = !m_cell_shared_state.m_draw_image;
          std::cout << "Draw Image = " << m_cell_shared_state.m_draw_image << "\n";
          break;
        case SDLK_b:
          if (ev.key.keysym.mod & KMOD_CTRL)
            {
              cycle_value(m_current_blend, ev.key.keysym.mod & (KMOD_SHIFT | KMOD_ALT), m_blend_labels.size());
              std::cout << "Rect Blend mode set to: " << m_blend_labels[m_current_blend].second << "\n";
              m_cell_shared_state.m_rect_blend_mode = m_blend_labels[m_current_blend].first;
            }
          else
            {
              cycle_value(m_current_composite, ev.key.keysym.mod & (KMOD_SHIFT | KMOD_ALT), m_composite_labels.size());
              std::cout << "Rect Composite mode set to: " << m_composite_labels[m_current_composite].second << "\n";
              m_cell_shared_state.m_rect_composite_mode = m_composite_labels[m_current_composite].first;
            }
          break;
        case SDLK_0:
          m_zoomer.transformation(ScaleTranslate<float>());
          break;
        }
      break;
    }
}

int
main(int argc, char **argv)
{
  painter_cells P;
  return P.main(argc, argv);
}
