# Makefile for efibootnext GRUB module (out-of-tree build)
#
# Builds efibootnext.mod using compiler flags from the installed
# GRUB's modinfo.sh.
#
# Usage:
#   make                       # build for x86_64-efi (default)
#   make PLATFORM=arm64-efi    # build for aarch64
#   make install DESTDIR=...   # install the .mod file
#
# Build-depends (Debian/Ubuntu):
#   grub-efi-amd64-bin (or -arm64-bin)  build-essential

PLATFORM     ?= x86_64-efi
GRUB_MODDIR  ?= /usr/lib/grub/$(PLATFORM)
MODINFO_SH   ?= $(GRUB_MODDIR)/modinfo.sh
GRUB_INCLUDE ?= $(GRUB_MODDIR)/include

DESTDIR     ?=
INST_MODDIR ?= /usr/lib/grub/$(PLATFORM)

MODULE_NAME = efibootnext
SRC         = efibootnext.c
OBJ         = $(MODULE_NAME).o
MODULE      = $(MODULE_NAME).module
MOD         = $(MODULE_NAME).mod

CC        ?= gcc
TARGET_CC ?= $(CC)

.PHONY: all clean install check-modinfo

all: $(MOD)

# Source modinfo.sh and extract the compiler flags into a makefile fragment.
# Replace $(top_srcdir)/include and $(top_builddir)/include with the
# installed GRUB include directory, since we're building out-of-tree.
modinfo.mk: $(MODINFO_SH)
	@echo "# Auto-generated from $(MODINFO_SH)" > $@
	@. $(MODINFO_SH) && \
	  echo "GRUB_TARGET_CC = $${grub_target_cc}" >> $@ && \
	  echo "GRUB_CFLAGS    = $${grub_target_cflags}" >> $@ && \
	  echo "GRUB_CPPFLAGS  = $$(echo "$${grub_target_cppflags}" | sed 's|\$$(top_srcdir)/include|$(GRUB_INCLUDE)|g; s|\$$(top_builddir)/include|$(GRUB_INCLUDE)|g')" >> $@ && \
	  echo "GRUB_LDFLAGS   = $${grub_target_ldflags}" >> $@

-include modinfo.mk

check-modinfo:
	@test -f $(MODINFO_SH) || { \
	  echo "ERROR: $(MODINFO_SH) not found."; \
	  echo "Install grub-efi-amd64-bin (or the appropriate -bin package)."; \
	  exit 1; \
	}

$(OBJ): $(SRC) modinfo.mk | check-modinfo
	$(or $(GRUB_TARGET_CC),$(TARGET_CC)) \
	  $(GRUB_CPPFLAGS) -DGRUB_FILE=\"efibootnext.c\" \
	  $(GRUB_CFLAGS) -c -o $@ $<

$(MODULE): $(OBJ)
	$(or $(GRUB_TARGET_CC),$(TARGET_CC)) \
	  -nostdlib -Wl,-r,-d $(GRUB_LDFLAGS) -o $@ $^

$(MOD): $(MODULE)
	strip --strip-unneeded -K grub_mod_init -K grub_mod_fini \
	  -K _grub_mod_init -K _grub_mod_fini \
	  -R .note -R .comment -R .note.gnu.property \
	  -o $@ $<

install: $(MOD)
	install -d $(DESTDIR)$(INST_MODDIR)
	install -m 0644 $(MOD) $(DESTDIR)$(INST_MODDIR)/$(MOD)

clean:
	rm -f $(OBJ) $(MODULE) $(MOD) modinfo.mk
