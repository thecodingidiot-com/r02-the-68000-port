GENDEV ?= /opt/gendev
SGDKDIR ?= $(GENDEV)/sgdk

include $(SGDKDIR)/mkfiles/Makefile.rom

# SGDK's own symbol.txt rule (mkfiles/makefile.gen) hardcodes
# `nm --plugin=liblto_plugin-0.dll`, a Windows-toolchain DLL that
# doesn't exist on Linux -- it failed every build without ever
# touching rom.bin. Point NM at a wrapper that strips the flag rather
# than carry the broken one (see tools/nm-strip-plugin.sh).
NM := $(CURDIR)/tools/nm-strip-plugin.sh
