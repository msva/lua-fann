include .config

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
	$(CC) $(LF) $^ -o $@

%.o: %.c
	$(CC) $(CF) -c $^ -o $@

clean:
	$(RM) -f $(OBJ) $(BIN) test/*.net test/*.so $(DOCS)

docs: $(DOCS)

$(DOCS): $(SRC)
	mkdir -p doc
	$(LUA_BIN) doc.lua < $< > $@

test: all
	-ln -sf ../$(BIN) test/
	cd test && $(LUA_BIN) module.lua

dep:
	makedepend $(DEFINES) -Y $(SRC) >/dev/null 2>&1
	$(RM) -f Makefile.bak

install: all
	$(INSTALL) -d $(DESTDIR)$(LUA_CMODULE_DIR)
	$(INSTALL) $(BIN) $(DESTDIR)$(LUA_CMODULE_DIR)

uninstall: clean
	cd $(LUA_CMODULE_DIR);
	$(RM) -f $(BIN)

dist: $(VERSION).tar.gz

$(VERSION).tar.gz: $(SRC) $(TEST_FLS) $(OTHER_FILES)
	@mkdir $(VERSION)
	@mkdir $(VERSION)/src
	@cp $(SRC) $(HDR) $(VERSION)/src
	@mkdir $(VERSION)/test
	@cp $(TEST_FLS) $(VERSION)/test
	@cp $(OTHER_FILES) $(VERSION)
	@tar -czf $(VERSION).tar.gz $(VERSION)
	@$(RM) -rf $(VERSION)
