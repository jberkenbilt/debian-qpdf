BINS_libtests = \
	ascii85 \
	bits \
	buffer \
	flate \
	hex \
	lzw \
	md5 \
	pcre \
	png_filter \
	pointer_holder \
	qexc \
	qutil \
	rc4

TARGETS_libtests = $(foreach B,$(BINS_libtests),libtests/$(OUTPUT_DIR)/$(B))

$(TARGETS_libtests): $(TARGETS_libqpdf)

INCLUDES_libtests = include libqpdf

TC_SRCS_libtests = $(wildcard libqpdf/*.cc) $(wildcard libtests/*.cc) \
	libqpdf/bits.icc

# -----

$(foreach B,$(BINS_libtests),$(eval \
  OBJS_$(B) = $(call src_to_obj,libtests/$(B).cc)))

ifeq ($(GENDEPS),1)
-include $(foreach B,$(BINS_libtests),$(call obj_to_dep,$(OBJS_$(B))))
endif

$(foreach B,$(BINS_libtests),$(eval \
  $(OBJS_$(B)): libtests/$(OUTPUT_DIR)/%.o: libtests/$(B).cc ; \
	$(call compile,libtests/$(B).cc,$(INCLUDES_libtests))))

$(foreach B,$(BINS_libtests),$(eval \
  libtests/$(OUTPUT_DIR)/$(B): $(OBJS_$(B)) ; \
	$(call makebin,$(OBJS_$(B)),$$@)))
