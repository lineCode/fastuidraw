# Begin standard header
sp 		:= $(sp).x
dirstack_$(sp)	:= $(d)
d		:= $(dir)
# End standard header

dir := $(d)/image_test
include $(dir)/Rules.mk

dir := $(d)/gradient_test
include $(dir)/Rules.mk

dir := $(d)/glyph_test
include $(dir)/Rules.mk

# Begin standard footer
d		:= $(dirstack_$(sp))
sp		:= $(basename $(sp))
# End standard footer
