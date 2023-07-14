DAEMON_NAME           = wsdd
DAEMON_MAJOR_VERSION  = 2
DAEMON_MINOR_VERSION  = 1
DAEMON_PATCH_VERSION  = 0
DAEMON_PID_FILE_NAME  = $(DAEMON_NAME).pid
DAEMON_LOG_FILE_NAME  = $(DAEMON_NAME).log
DAEMON_NO_CHDIR       = 1
DAEMON_NO_CLOSE_STDIO = 0




GSOAP_VERSION     = 2.8.106
GSOAP_INSTALL_DIR = ./gsoap-2.8
GSOAP_DIR         = $(GSOAP_INSTALL_DIR)/gsoap
GSOAP_PLUGIN_DIR  = $(GSOAP_DIR)/plugin
GSOAP_IMPORT_DIR  = $(GSOAP_DIR)/import


SOAPCPP2          = $(GSOAP_DIR)/src/soapcpp2
WSDL2H            = $(GSOAP_DIR)/wsdl/wsdl2h
GSOAP_CONFIGURE   = --disable-c-locale --disable-ssl


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

CC               ?= gcc




SOAP_SRC = $(GSOAP_DIR)/stdsoap2.c         \
           $(GSOAP_PLUGIN_DIR)/wsaapi.c    \
           $(GSOAP_PLUGIN_DIR)/wsddapi.c   \
           $(GENERATED_DIR)/soapClient.c



# Add your source files to the list.
# Supported *.c  *.cpp  *.S files.
# For other file types write a template rule for build, see below.
SOURCES  = $(COMMON_DIR)/$(DAEMON_NAME).c   \
           $(COMMON_DIR)/daemon.c           \
           $(COMMON_DIR)/net_utils.c        \
           $(COMMON_DIR)/file_utils.c       \
           $(COMMON_DIR)/wsdd_utils.c       \
           $(COMMON_DIR)/client_events.c    \
           $(GENERATED_DIR)/soapC.c         \
           $(SOAP_SRC)





OBJECTS  := $(patsubst %.c,  %.o, $(SOURCES) )
OBJECTS  := $(patsubst %.cpp,%.o, $(OBJECTS) )
OBJECTS  := $(patsubst %.S,  %.o, $(OBJECTS) )


DEBUG_SUFFIX   = debug

DEBUG_OBJECTS := $(patsubst %.o, %_$(DEBUG_SUFFIX).o, $(OBJECTS) )




.PHONY: all
all: debug release



.PHONY: release
release: CFLAGS := -s  $(CFLAGS)
release: $(DAEMON_NAME)



.PHONY: debug
debug: DAEMON_NO_CLOSE_STDIO = 1
debug: CFLAGS := -DDEBUG  -g  $(CFLAGS)
debug: $(DAEMON_NAME)_$(DEBUG_SUFFIX)



# release
$(DAEMON_NAME): .depend $(OBJECTS)
	$(call build_bin, $(OBJECTS))


# debug
$(DAEMON_NAME)_$(DEBUG_SUFFIX): .depend $(DEBUG_OBJECTS)
	$(call build_bin, $(DEBUG_OBJECTS))



# Build release objects
%.o: %.c
	$(build_object)


%.o: %.cpp
	$(build_object)


%.o: %.S
	$(build_object)



# Build debug objects
%_$(DEBUG_SUFFIX).o: %.c
	$(build_object)


%_$(DEBUG_SUFFIX).o: %.cpp
	$(build_object)


%_$(DEBUG_SUFFIX).o: %.S
	$(build_object)



.PHONY: clean
clean:
	-@rm -f $(DAEMON_NAME)
	-@rm -f $(DAEMON_NAME)_$(DEBUG_SUFFIX)
	-@rm -f $(OBJECTS)
	-@rm -f $(DEBUG_OBJECTS)
	-@rm -f .depend
	-@rm -f -d -R $(GENERATED_DIR)
	-@rm -f *.*~



.PHONY: distclean
distclean: clean
	-@rm -f -d -R SDK
	-@rm -f -d -R $(GSOAP_INSTALL_DIR)
	-@rm -f RECV.log SENT.log TEST.log



.depend: $(GENERATED_DIR)/soapC.c
	-@rm -f .depend
	@echo "Generating dependencies..."
	@for src in $(SOURCES) ; do \
        echo "  [depend]  $$src" ; \
        $(CC) $(CFLAGS) -MT ".depend $${src%.*}.o $${src%.*}_$(DEBUG_SUFFIX).o" -MM $$src >> .depend ; \
    done



ifeq "$(findstring $(MAKECMDGOALS),clean distclean)"  ""
    include $(wildcard .depend)
endif




# ---- gSOAP ----

$(GENERATED_DIR)/wsdd.h:
	@$(build_gsoap)
	@mkdir -p $(GENERATED_DIR)
	$(WSDL2H) -cg -t $(GSOAP_DIR)/WS/typemap.dat  -o $@  wsdl/remotediscovery.wsdl



$(GENERATED_DIR)/soapC.c: $(GENERATED_DIR)/wsdd.h
	$(SOAPCPP2) -C -L -x -c -2 -d $(GENERATED_DIR) -I$(GSOAP_DIR):$(GSOAP_IMPORT_DIR) $<



# This targets is needed for parallel work of make
$(OBJECTS) $(DEBUG_OBJECTS) $(SOAP_SRC): $(GENERATED_DIR)/soapC.c





# Common commands
BUILD_ECHO = echo "\n  [build]  $@:"


define build_object
    @$(BUILD_ECHO)
    $(CC) -c $< -o $@  $(CFLAGS)
endef



define build_bin
    @$(BUILD_ECHO)
    $(CC)  $1 -o $@  $(CFLAGS)
    @echo "\n---- Compiled $@ ver $(DAEMON_MAJOR_VERSION).$(DAEMON_MINOR_VERSION).$(DAEMON_PATCH_VERSION) ----\n"
endef



define build_gsoap

    # get archive
    if [ ! -f SDK/gsoap.zip ]; then \
        mkdir -p SDK; \
        wget -O ./SDK/gsoap.zip.tmp "https://downloads.sourceforge.net/gsoap2/gsoap_$(GSOAP_VERSION).zip"         || \
        wget -O ./SDK/gsoap.zip.tmp "http://downloads.yoctoproject.org/mirror/sources/gsoap_$(GSOAP_VERSION).zip" || \
        wget -O ./SDK/gsoap.zip.tmp "http://sources.openembedded.org/gsoap_$(GSOAP_VERSION).zip"                  || \
        wget -O ./SDK/gsoap.zip.tmp "https://mirrors.kernel.org/yocto-sources/gsoap_$(GSOAP_VERSION).zip"         && \
        mv ./SDK/gsoap.zip.tmp ./SDK/gsoap.zip; \
    fi

    # unzip
    if [ ! -f $(GSOAP_INSTALL_DIR)/README.txt ]; then \
         unzip ./SDK/gsoap.zip; \
    fi

    # build
    if [ ! -f $(SOAPCPP2) ] || [ ! -f $(WSDL2H) ]; then \
         cd $(GSOAP_INSTALL_DIR); \
         ./configure $(GSOAP_CONFIGURE) && \
         make -j1; \
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

