include .config
# Some functions for candiness-looking
err =                                           \
        @echo -e "\e[1;31m*\e[0m $(1)\e[0m";    \
        @exit 1;
inf =                                           \
        @echo -e "\e[1;32m*\e[0m $(1)\e[0m";
wrn =                                           \
        @echo -e "\e[1;33m*\e[0m $(1)\e[0m";
ext =                                           \
        @echo -e "\e[1;35m*\e[0m $(1)\e[0m";

UNAME            ?= $(shell uname)
DESTDIR          ?= /
PKG_CONFIG       ?= pkg-config
INSTALL          ?= install
RM               ?= rm
LUA_IMPL         ?= lua
LUA_BIN          ?= $(LUA_IMPL)
LUA_CMODULE_DIR  ?= $(shell $(PKG_CONFIG) --variable INSTALL_CMOD $(LUA_IMPL))
LIBDIR           ?= $(shell $(PKG_CONFIG) --variable libdir $(LUA_IMPL))
LUA_INC          ?= $(shell $(PKG_CONFIG) --variable INSTALL_INC $(LUA_IMPL))
CC               ?= cc

ifeq ($(UNAME), Linux)
OS_FLAGS         ?= -shared
endif
ifeq ($(UNAME), Darwin)
OS_FLAGS         ?= -bundle -undefined dynamic_lookup
endif

BIN               = src/fann.so
OBJ               = src/fann.o
INCLUDES          = -I$(LUA_INC)
DEFINES           =
LIBS              = -L$(LIBDIR) -lfann
COMMONFLAGS       = -O2 -g -std=c99 -pipe -fPIC $(OS_FLAGS)
LF                = $(LIBS) $(COMMONFLAGS) $(LDFLAGS)
CF                = -c $(INCLUDES) $(DEFINES) $(COMMONFLAGS) $(CFLAGS)

SRC               = src/fann.c
HDR               = src/fann.h
TEST_FLS          = test/module.lua \
                    test/xor.data \
                    test/xortest.data
OTHER_FILES       = Makefile \
	            .config \
	            README \
	            LICENSE \
	            TODO
DOCS              = doc/luafann.html
VERSION           = "Lua-FANN-0.3"

all: $(BIN)

$(BIN): $(OBJ)
	@$(call ext,"Library compiling and linking...")
	@$(CC) $(LF) $^ -o $@
	@$(call inf,"Library compiling and linking is done!")

%.o: %.c
	@$(call ext,"Object files compliling in progress...")
	@$(CC) $(CF) -c $^ -o $@
	@$(call inf,"Object files compliling is done!")

clean:
	@$(call wrn,"Cleaning...")
	@$(RM) -f $(OBJ) $(BIN) test/*.net test/$(BIN) $(DOCS)
	@$(call inf,"Cleaning is done!")

docs: $(DOCS)

$(DOCS): $(SRC)
	@$(call ext,"Documentation generation...")
	@mkdir -p doc
	@$(LUA_BIN) doc.lua < $< > $@
	@$(call inf,"Documentation generation is done!")

test: all
	@$(call wrn,"Testing...")
	-@ln -sf ../$(BIN) test/
	-@cd test && $(LUA_BIN) module.lua
	@$(call inf,"Testing is done!")

dep:
	@$(call ext,"Making depends...")
	@makedepend $(DEFINES) -Y $(SRC) >/dev/null 2>&1
	@$(RM) -f Makefile.bak
	@$(call inf,"Making depends is done!")

install: all
	@$(call ext,"Installing...")
	@$(INSTALL) -d $(LUA_CMODULE_DIR)
	@$(INSTALL) $(BIN) $(LUA_CMODULE_DIR)
	@$(call inf,"Installing is done!")

uninstall: clean
	@$(call wrn,"Uninstalling...")
	@cd $(LUA_CMODULE_DIR);
	@$(RM) -f $(BIN)
	@$(call inf,"Uninstalling is done!")

dist: $(VERSION).tar.gz

$(VERSION).tar.gz: $(SRC) $(TEST_FLS) $(OTHER_FILES)
	@$(call ext,"Creating $(VERSION).tar.gz...")
	@mkdir $(VERSION)
	@mkdir $(VERSION)/src
	@cp $(SRC) $(VERSION)/src
	@mkdir $(VERSION)/test
	@cp $(TEST_FLS) $(VERSION)/test
	@cp $(OTHER_FILES) $(VERSION)
	@tar -czf $(VERSION).tar.gz $(VERSION)
	@$(RM) -rf $(VERSION)
	@$(call inf,"Creating is done!")
