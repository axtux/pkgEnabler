.SUFFIXES:
ifeq ($(strip $(PSL1GHT)),)
$(error "PSL1GHT must be set in the environment.")
endif

include $(PSL1GHT)/host/ppu.mk

TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCE		:=	source
INCLUDE		:=	include
DATA		:=	data
LIBS		:=	-lgcm_sys -lreality -lsysutil -lio

TITLE		:=	Package Enabler
APPID		:=	PKGENABLE
CONTENTID	:=	UP0001-$(APPID)_00-0000000000000000
PKGFILES	:=	release
SFOXML		:=	sfo.xml
ICON0       	:= 	$(CURDIR)/ICON0.PNG

ONP		:=	$(CURDIR)/include/OFW/nas_plugin.sprx
OCG		:=	$(CURDIR)/include/OFW/category_game.xml
CNP		:=	$(CURDIR)/include/CFW/nas_plugin.sprx
CCG		:=	$(CURDIR)/include/CFW/category_game.xml

CFLAGS		+= -O2 -Wall -std=gnu99
CXXFLAGS	+= -O2 -Wall

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export VPATH	:=	$(foreach dir,$(SOURCE),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))
export BUILDDIR	:=	$(CURDIR)/$(BUILD)
export DEPSDIR	:=	$(BUILDDIR)

CFILES		:= $(foreach dir,$(SOURCE),$(notdir $(wildcard $(dir)/*.c)))
CXXFILES	:= $(foreach dir,$(SOURCE),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:= $(foreach dir,$(SOURCE),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:= $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.bin)))
VCGFILES	:= $(foreach dir,$(SOURCE),$(notdir $(wildcard $(dir)/*.vcg)))

ifeq ($(strip $(CXXFILES)),)
export LD	:=	$(CC)
else
export LD	:=	$(CXX)
endif

export OFILES	:=	$(CFILES:.c=.o) \
					$(CXXFILES:.cpp=.o) \
					$(SFILES:.S=.o) \
					$(VCGFILES:.vcg=.vcg.o) \
					$(BINFILES:.bin=.bin.o)

export BINFILES	:=	$(BINFILES:.bin=.bin.h)

export INCLUDES	:=	$(foreach dir,$(INCLUDE),-I$(CURDIR)/$(dir)) \
					-I$(CURDIR)/$(BUILD)

.PHONY: $(BUILD) clean pkg run

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	@cp $(ICON0) $(BUILD)
	@mkdir -p $(BUILD)/pkg/USRDIR/OFW
	@mkdir -p $(BUILD)/pkg/USRDIR/CFW
	@cp $(ONP) $(BUILD)/pkg/USRDIR/OFW
	@cp $(OCG) $(BUILD)/pkg/USRDIR/OFW
	@cp $(CNP) $(BUILD)/pkg/USRDIR/CFW
	@cp $(CCG) $(BUILD)/pkg/USRDIR/CFW
clean:
	@echo "[RM]  $(notdir $(OUTPUT))"
	@rm -rf $(BUILD) $(OUTPUT).elf $(OUTPUT).self $(OUTPUT).a $(OUTPUT)*.pkg

run: $(BUILD)
	@$(PS3LOADAPP) $(OUTPUT).self

pkg: $(BUILD) $(OUTPUT).pkg

else

DEPENDS	:= $(OFILES:.o=.d)

$(OUTPUT).self: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
$(OFILES): $(BINFILES)

-include $(DEPENDS)

endif
