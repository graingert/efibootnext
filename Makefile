PREFIX ?= /etc

GRUB_DIR = $(PREFIX)/grub.d
INITRAMFS_HOOKS_DIR = $(PREFIX)/initramfs-tools/hooks
INITRAMFS_SCRIPTS_DIR = $(PREFIX)/initramfs-tools/scripts/init-premount

install:
	install -m 755 50_efibootnext $(DESTDIR)$(GRUB_DIR)/50_efibootnext
	install -m 755 efibootnext-hook $(DESTDIR)$(INITRAMFS_HOOKS_DIR)/efibootnext
	install -m 755 efibootnext-premount $(DESTDIR)$(INITRAMFS_SCRIPTS_DIR)/efibootnext

uninstall:
	rm -f $(DESTDIR)$(GRUB_DIR)/50_efibootnext
	rm -f $(DESTDIR)$(INITRAMFS_HOOKS_DIR)/efibootnext
	rm -f $(DESTDIR)$(INITRAMFS_SCRIPTS_DIR)/efibootnext
