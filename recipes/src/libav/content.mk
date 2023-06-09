content: src/lib/libav lib/import lib/mk LICENSE

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/libav)

src/lib/libav:
	mkdir -p $@
	cp -r $(PORT_DIR)/src/lib/libav/* $@
	cp -r $(REP_DIR)/src/lib/libav/* $@

lib/import:
	mkdir -p $@
	cp $(REP_DIR)/lib/import/import-av* $@
	cp $(REP_DIR)/lib/import/import-swscale.mk $@

lib/mk:
	mkdir -p $@
	cp $(addprefix $(REP_DIR)/$@/,av* swscale.mk) $@
	for spec in x86 x86_32 x86_64 arm arm_64; do \
	  mkdir -p $@/spec/$$spec; \
	  cp $(addprefix $(REP_DIR)/$@/spec/$$spec/,av*) $@/spec/$$spec/; done

LICENSE:
	cp $(PORT_DIR)/src/lib/libav/LICENSE $@
