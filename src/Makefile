BUILD_DIR	:= build_all
CTAGS		:= ctags --c-kinds=+p --c++-kinds=+p --fields=+iaS --extras=+qf -R

td_makefile := $(BUILD_DIR)/Makefile

all: tags

tags: trader
	./updtags.sh

trader: $(td_makefile)
	$(MAKE) -C $(BUILD_DIR)

$(td_makefile): CMakeLists.txt
	./prebuild.sh

clean:
	#rm -rvf $(BUILD_DIR)/*
	$(MAKE) clean -C $(BUILD_DIR)

.PHONY: all trader clean
