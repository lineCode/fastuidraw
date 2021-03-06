# Begin standard header
sp 		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)
# End standard header

FASTUIDRAW_RESOURCE_STRING += $(call filelist, \
	fastuidraw_blend_util.frag.glsl.resource_string \
	fastuidraw_fbf_w3c_normal.glsl.resource_string \
	fastuidraw_fbf_w3c_multiply.glsl.resource_string \
	fastuidraw_fbf_w3c_screen.glsl.resource_string \
	fastuidraw_fbf_w3c_overlay.glsl.resource_string \
	fastuidraw_fbf_w3c_darken.glsl.resource_string \
	fastuidraw_fbf_w3c_lighten.glsl.resource_string \
	fastuidraw_fbf_w3c_color_dodge.glsl.resource_string \
	fastuidraw_fbf_w3c_color_burn.glsl.resource_string \
	fastuidraw_fbf_w3c_hardlight.glsl.resource_string \
	fastuidraw_fbf_w3c_softlight.glsl.resource_string \
	fastuidraw_fbf_w3c_difference.glsl.resource_string \
	fastuidraw_fbf_w3c_exclusion.glsl.resource_string \
	fastuidraw_fbf_w3c_hue.glsl.resource_string \
	fastuidraw_fbf_w3c_saturation.glsl.resource_string \
	fastuidraw_fbf_w3c_color.glsl.resource_string \
	fastuidraw_fbf_w3c_luminosity.glsl.resource_string \
	)

# Begin standard footer
d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
# End standard footer
