# GRUB → Windows reboot via EFI BootNext (BitLocker-safe)

Boot Windows from a GRUB menu entry without breaking BitLocker TPM measurements.

## Problem

Chainloading `\EFI\Microsoft\Boot\bootmgfw.efi` from GRUB breaks TPM PCR
measurements, causing BitLocker to prompt for a recovery key every time.

## Solution

Instead of chainloading, boot a minimal Linux kernel/initramfs that:

1. Reads a `bootnext=` parameter from the kernel command line
2. Mounts `efivarfs` and calls `efibootmgr --bootnext` to set the UEFI `BootNext` variable
3. Immediately reboots

The firmware then boots Windows natively with correct TPM state. BitLocker is happy.

The premount script runs before the LUKS prompt, so you never have to enter your
Linux disk encryption password just to boot Windows.

## Files

| File | Install to |
|------|------------|
| `efibootnext-hook` | `/etc/initramfs-tools/hooks/efibootnext` |
| `efibootnext-premount` | `/etc/initramfs-tools/scripts/init-premount/efibootnext` |
| `40_custom` | `/etc/grub.d/40_custom` |

## Install

```sh
# install efibootmgr if not already present
sudo apt install efibootmgr

# copy files
sudo cp efibootnext-hook /etc/initramfs-tools/hooks/efibootnext
sudo cp efibootnext-premount /etc/initramfs-tools/scripts/init-premount/efibootnext
sudo chmod +x /etc/initramfs-tools/hooks/efibootnext
sudo chmod +x /etc/initramfs-tools/scripts/init-premount/efibootnext

# append the menuentry to 40_custom (or replace if you have no other custom entries)
sudo cp 40_custom /etc/grub.d/40_custom
sudo chmod +x /etc/grub.d/40_custom

# rebuild initramfs and grub
sudo update-initramfs -u
sudo update-grub
```

## Configuration

Find your Windows Boot Manager EFI entry number:

```sh
efibootmgr -v
```

Look for something like:

```
Boot0007* Windows Boot Manager	HD(...)/File(\EFI\Microsoft\Boot\bootmgfw.efi)
```

Update the `bootnext=` value in `40_custom` to match (e.g. `bootnext=0007`).

Find your `/boot` partition UUID:

```sh
findmnt -no UUID /boot
```

Update the `search --fs-uuid --set=root` line in `40_custom` to match.

## Verify

Check efibootmgr is in the initramfs:

```sh
lsinitramfs /boot/initrd.img | grep efibootmgr
```

Check the GRUB menu entry exists:

```sh
grep -E "^menuentry|^submenu" /boot/grub/grub.cfg
```

## How it works

```
GRUB
 → boots linux kernel + initramfs with bootnext=0007
   → init-premount/efibootnext runs (before LUKS prompt)
     → mounts efivarfs
     → efibootmgr --bootnext 0007
     → reboot -f
       → firmware reads BootNext, boots Windows natively
         → TPM PCRs are correct, BitLocker unlocks automatically
```

## Requirements

- Ubuntu with GRUB2 and EFI
- `efibootmgr` and `initramfs-tools` packages
- UEFI firmware (not legacy BIOS)

## License

Public domain / CC0