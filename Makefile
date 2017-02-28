DAEMON_NAME           = wsdd
DAEMON_MAJOR_VERSION  = 0
DAEMON_MINOR_VERSION  = 0
DAEMON_PATCH_VERSION  = 0
DAEMON_PID_FILE_NAME  = $(DAEMON_NAME).pid
DAEMON_LOG_FILE_NAME  = $(DAEMON_NAME).log
DAEMON_NO_CHDIR       = 1
DAEMON_NO_CLOSE_STDIO = 0




GSOAP_DIR         = ./gsoap-2.8/gsoap
GSOAP_PLUGIN_DIR  = $(GSOAP_DIR)/plugin
GSOAP_IMPORT_DIR  = $(GSOAP_DIR)/import


SOAPCPP2          = $(GSOAP_DIR)/src/soapcpp2
WSDL2H            = $(GSOAP_DIR)/wsdl/wsdl2h


COMMON_DIR        = ./src
GENERATED_DIR     = ./generated


CFLAGS            = -DDAEMON_NAME='"$(DAEMON_NAME)"'
CFLAGS           += -DDAEMON_MAJOR_VERSION=$(DAEMON_MAJOR_VERSION)
CFLAGS           += -DDAEMON_MINOR_VERSION=$(DAEMON_MINOR_VERSION)
CFLAGS           += -DDAEMON_PATCH_VERSION=$(DAEMON_PATCH_VERSION)
CFLAGS           += -DDAEMON_PID_FILE_NAME='"$(DAEMON_PID_FILE_NAME)"'
CFLAGS           += -DDAEMON_LOG_FILE_NAME='"$(DAEMON_LOG_FILE_NAME)"'
CFLAGS           += -DDAEMON_NO_CHDIR=$(DAEMON_NO_CHDIR)
CFLAGS           += -DDAEMON_NO_CLOSE_STDIO=$(DAEMON_NO_CLOSE_STDIO)

CFLAGS           += -I$(COMMON_DIR)
CFLAGS           += -I$(GENERATED_DIR)
CFLAGS           += -I$(GSOAP_DIR) -I$(GSOAP_PLUGIN_DIR) -I$(GSOAP_IMPORT_DIR)
CFLAGS           += -O2  -Wall  -pipe

GCC              =  gcc




# Add your source files to the list.
# Supported *.c  *.cpp  *.S files.
# For other file types write a template rule for build, see below.
SOURCES  = $(COMMON_DIR)/$(DAEMON_NAME).c   \
           $(COMMON_DIR)/daemon.c           \
           $(COMMON_DIR)/net_utils.c        \
           $(COMMON_DIR)/file_utils.c       \
           $(COMMON_DIR)/client_events.c    \
           $(GENERATED_DIR)/soapC.c         \
           $(GENERATED_DIR)/soapClient.c    \
           $(GSOAP_DIR)/stdsoap2.c          \
           $(GSOAP_PLUGIN_DIR)/wsaapi.c     \
           $(GSOAP_PLUGIN_DIR)/wsddapi.c




OBJECTS  := $(patsubst %.c,  %.o, $(SOURCES) )
OBJECTS  := $(patsubst %.cpp,%.o, $(OBJECTS) )
OBJECTS  := $(patsubst %.S,  %.o, $(OBJECTS) )


DEBUG_OBJECTS := $(patsubst %.o, %_debug.o, $(OBJECTS) )




.PHONY: all
all: generate debug release



.PHONY: release
release: CFLAGS := -s  $(CFLAGS)
release: $(DAEMON_NAME)



.PHONY: debug
debug: DAEMON_NO_CLOSE_STDIO = 1
debug: CFLAGS := -DDEBUG  -g  $(CFLAGS)
debug: $(DAEMON_NAME)_debug



# release
$(DAEMON_NAME): .depend $(OBJECTS)
	$(call build_bin, $(OBJECTS))


# debug
$(DAEMON_NAME)_debug: .depend $(DEBUG_OBJECTS)
	$(call build_bin, $(DEBUG_OBJECTS))



# Build release objects
%.o: %.c
	$(build_object)


%.o: %.cpp
	$(build_object)


%.o: %.S
	$(build_object)



# Build debug objects
%_debug.o: %.c
	$(build_object)


%_debug.o: %.cpp
	$(build_object)


%_debug.o: %.S
	$(build_object)



.PHONY: clean
clean:
	-@rm -f $(DAEMON_NAME)
	-@rm -f $(DAEMON_NAME)_debug
	-@rm -f $(OBJECTS)
	-@rm -f $(DEBUG_OBJECTS)
	-@rm -f .depend
	-@rm -f -d -R $(GENERATED_DIR)
	-@rm -f *.*~



.PHONY: distclean
distclean: clean
	-@rm -f -d -R SDK
	-@rm -f -d -R gsoap-2.8



.depend: cmd  = echo "  [depend]  $(var)" &&
.depend: cmd += $(GCC) $(CFLAGS) -MT ".depend $(basename $(var)).o $(basename $(var))_debug.o"  -MM $(var) >> .depend;
.depend: $(SOURCES)
	@rm -f .depend
	@echo "Generating dependencies..."
	@$(foreach var, $(SOURCES), $(cmd))


include $(wildcard .depend)




generated/wsdd.h:
	@$(build_gsoap)
	@mkdir -p $(GENERATED_DIR)
	$(WSDL2H) -cg -t $(GSOAP_DIR)/WS/typemap.dat  -o $@  wsdl/remotediscovery.wsdl



generated/soapC.c: $(GENERATED_DIR)/wsdd.h
	$(SOAPCPP2) -L -x -c -2 -d $(GENERATED_DIR) -I$(GSOAP_DIR):$(GSOAP_IMPORT_DIR) $<



.PHONY: generate
generate: $(GENERATED_DIR)/soapC.c




# Common commands
BUILD_ECHO = echo "\n  [build]  $@:"


define build_object
    @$(BUILD_ECHO)
    $(GCC) -c $< -o $@  $(CFLAGS)
endef



define build_bin
    @$(BUILD_ECHO)
    $(GCC)  $1 -o $@  $(CFLAGS)
    @echo "\n---- Compiled $@ ver $(DAEMON_MAJOR_VERSION).$(DAEMON_MINOR_VERSION).$(DAEMON_PATCH_VERSION) ----\n"
endef



define build_gsoap

    # get archive
    if [ ! -f SDK/gsoap_2.8.43.zip ]; then \
        mkdir -p SDK; \
        wget -O ./SDK/gsoap_2.8.43.zip "https://vorboss.dl.sourceforge.net/project/gsoap2/gsoap-2.8/gsoap_2.8.43.zip"; \
    fi

    # unzip
    if [ ! -f gsoap-2.8/README.txt ]; then \
         unzip SDK/gsoap_2.8.43.zip; \
    fi

    # build
    if [ ! -f $(SOAPCPP2) ] || [ ! -f $(WSDL2H) ]; then \
         cd gsoap-2.8; \
         ./configure; \
         make; \
         cd ..;\
    fi
endef




.PHONY: help
help:
	@echo "make [command]"
	@echo "command is:"
	@echo "   all       -  build daemon in release and debug mode"
	@echo "   debug     -  build in debug mode (#define DEBUG 1)"
	@echo "   release   -  build in release mode (strip)"
	@echo "   clean     -  remove all generated files"
	@echo "   distclean -  clean + remove all SDK files"
	@echo "   help      -  this help"

