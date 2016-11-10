# Begin standard header
sp 		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)
# End standard header

LIBRARY_SOURCES += $(call filelist, \
	painter_brush.cpp painter_brush_shader_set.cpp \
	painter_brush_shader_data.cpp \
	linear_gradient_params.cpp radial_gradient_params.cpp \
	pen_params.cpp image_params.cpp \
	repeat_window_params.cpp \
	transformation_translation_params.cpp \
	transformation_matrix_params.cpp \
	unified_brush_params.cpp)

# Begin standard footer
d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
# End standard footer
