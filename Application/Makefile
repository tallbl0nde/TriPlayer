#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

#---------------------------------------------------------------------------------
# Options for compilation
# TARGET: Name of the output file(s)
# BUILD: Directory where object files & intermediate files will be placed
# SOURCES: List of directories containing source code
# INCLUDES: List of directories containing header files
# ROMFS: Directory containing data to be added to RomFS
#---------------------------------------------------------------------------------
TARGET		:=	TriPlayer
BUILD		:=	build
SOURCES		:=	source ../Common/source source/sysmodule source/utils source/ui source/ui/screen source/ui/element
INCLUDES	:=	include ../Common/include include/sysmodule include/utils include/ui include/ui/screen include/ui/element
ROMFS		:=	romfs

#---------------------------------------------------------------------------------
# Application version
#---------------------------------------------------------------------------------
VER_MAJOR	:= 1
VER_MINOR	:= 0
VER_MICRO	:= 0

#---------------------------------------------------------------------------------
# Options for .nacp information
#---------------------------------------------------------------------------------
APP_TITLE   := 	TriPlayer
APP_AUTHOR	:= 	tallbl0nde
APP_VERSION	:=	$(VER_MAJOR).$(VER_MINOR).$(VER_MICRO)
ICON 		:= 	icon.jpg

#---------------------------------------------------------------------------------
# Options for code generation
# LIBS: Libraries to link against
# (I dunno what the rest is)
#---------------------------------------------------------------------------------
LIBS	:=  -lAether -lnx -lSQLite `sdl2-config --libs` -lSDL2_ttf `freetype-config --libs` -lSDL2_gfx -lSDL2_image -lpng -ljpeg -lwebp

ARCH	:=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

CFLAGS	:=	-g -Wall -O2 -ffunction-sections $(ARCH) $(DEFINES) $(INCLUDE) -D__SWITCH__ \
			-DVER_MAJOR=$(VER_MAJOR) -DVER_MINOR=$(VER_MINOR) -DVER_MICRO=$(VER_MICRO) -DVER_STRING=\"$(VER_MAJOR).$(VER_MINOR).$(VER_MICRO)\"
ifdef NXLINK
CFLAGS	+= -D_NXLINK_
endif

CXXFLAGS	:= $(CFLAGS) -fno-rtti -std=gnu++17

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(PORTLIBS) $(LIBNX) $(CURDIR)/libs/Aether $(CURDIR)/../Common/libs/SQLite

#---------------------------------------------------------------------------------
# This is all wizardry to me also
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

export LD	:=	$(CXX)

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 	:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) $(foreach dir,$(LIBDIRS),-I$(dir)/include) -I$(CURDIR)/$(BUILD)
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

#---------------------------------------------------------------------------------
# End of wizardry
# Set .nro flags
#---------------------------------------------------------------------------------

# Add icon and nacp flags (as these will always be present)
export APP_ICON := $(TOPDIR)/$(ICON)
export NROFLAGS += --icon=$(APP_ICON) --nacp=$(CURDIR)/$(TARGET).nacp --romfsdir=$(CURDIR)/$(ROMFS)

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
# Default build target: everything
#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
ifeq ($(wildcard $(CURDIR)/libs/Aether/LICENSE),)
	@$(error "Please run 'git submodule update --init' before running 'make'")
endif
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) -C $(CURDIR)/libs/Aether -f $(CURDIR)/libs/Aether/Makefile
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
# Clean removes all build files + sdcard directory
#---------------------------------------------------------------------------------
clean:
	@echo Cleaning ALL Application + Aether build files...
	@rm -rf $(BUILD) $(TARGET).nro $(TARGET).nacp $(TARGET).elf
	@$(MAKE) -C $(CURDIR)/libs/Aether -f $(CURDIR)/libs/Aether/Makefile clean
#---------------------------------------------------------------------------------
else
.PHONY:	all all-nxlink clean

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# Main target
#---------------------------------------------------------------------------------
# Important binaries are copied into approporiate places
# (I gave up figuring out how to do it properly)
all: $(OUTPUT).nro

$(OUTPUT).nro	:	$(OUTPUT).elf $(OUTPUT).nacp
$(OUTPUT).elf	:	$(OFILES)
$(OFILES_SRC)	: $(HFILES_BIN)

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	%_bin.h :	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------