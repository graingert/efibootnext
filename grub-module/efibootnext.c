/* efibootnext.c - Set EFI BootNext variable from GRUB.
 *
 * GRUB module that provides a `bootnext` command to set the UEFI
 * BootNext NVRAM variable and (optionally) reboot.  This lets a GRUB
 * menu entry hand control to a different EFI boot option for one boot
 * only, without touching BootOrder.
 *
 * Usage (from grub.cfg or the GRUB shell):
 *
 *   bootnext 0003          # set BootNext to Boot0003, don't reboot
 *   bootnext --reboot 0003 # set BootNext to Boot0003 and reboot
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026  Thomas Grainger <tagrain@gmail.com>
 *
 * Based on grub-core/commands/efi/efifwsetup.c from GRUB, which is:
 *   Copyright (C) 2012  Free Software Foundation, Inc.
 */

#include <grub/dl.h>
#include <grub/efi/api.h>
#include <grub/efi/efi.h>
#include <grub/err.h>
#include <grub/extcmd.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/types.h>

GRUB_MOD_LICENSE ("GPLv3+");

/* ------------------------------------------------------------------ */

static const struct grub_arg_option options[] =
  {
    { "reboot", 'r', 0, N_("Reboot immediately after setting BootNext."),
      0, 0 },
    { 0, 0, 0, 0, 0, 0 }
  };

/* ------------------------------------------------------------------ */

/*
 * Validate that Boot#### exists in NVRAM for the requested entry.
 * Returns GRUB_ERR_NONE on success.
 */
static grub_err_t
check_boot_entry_exists (grub_uint16_t entry)
{
  char name[sizeof ("Boot0000")];
  grub_efi_guid_t global = GRUB_EFI_GLOBAL_VARIABLE_GUID;
  grub_size_t size = 0;
  void *data;

  grub_snprintf (name, sizeof (name), "Boot%04x", (unsigned) entry);

  data = grub_efi_get_variable (name, &global, &size);
  if (!data)
    return grub_error (GRUB_ERR_FILE_NOT_FOUND,
                       N_("EFI variable `%s' not found"), name);

  grub_free (data);
  return GRUB_ERR_NONE;
}

/* ------------------------------------------------------------------ */

static grub_err_t
grub_cmd_bootnext (grub_extcmd_context_t ctxt,
                   int argc, char **args)
{
  struct grub_arg_list *state = ctxt->state;
  grub_efi_guid_t global = GRUB_EFI_GLOBAL_VARIABLE_GUID;
  grub_efi_uint16_t entry;
  unsigned long parsed;
  grub_err_t status;

  if (argc != 1)
    return grub_error (GRUB_ERR_BAD_ARGUMENT,
                       N_("exactly one hex boot entry number required"));

  /* ---- parse the hex string ---- */

  parsed = grub_strtoul (args[0], NULL, 16);
  if (grub_errno != GRUB_ERR_NONE)
    return grub_errno;

  if (parsed > 0xffff)
    return grub_error (GRUB_ERR_OUT_OF_RANGE,
                       N_("boot entry number `0x%lx' out of range"),
                       parsed);

  entry = (grub_efi_uint16_t) parsed;

  /* ---- make sure Boot#### exists ---- */

  status = check_boot_entry_exists (entry);
  if (status != GRUB_ERR_NONE)
    return status;

  /* ---- write BootNext ---- */

  /*
   * Per the UEFI specification BootNext is a uint16 stored with
   * attributes NV + BS + RT (same as BootOrder, Boot####, etc.).
   */
  status = grub_efi_set_variable ("BootNext", &global,
                                  &entry, sizeof (entry));
  if (status != GRUB_ERR_NONE)
    return grub_error (status,
                       N_("failed to set BootNext to %04x"),
                       (unsigned) entry);

  grub_printf ("BootNext set to Boot%04x\n", (unsigned) entry);

  /* ---- optionally reboot ---- */

  if (state[0].set)   /* --reboot / -r */
    grub_reboot ();

  return GRUB_ERR_NONE;
}

/* ------------------------------------------------------------------ */

static grub_extcmd_t cmd;

GRUB_MOD_INIT (efibootnext)
{
  cmd = grub_register_extcmd ("bootnext", grub_cmd_bootnext, 0,
                              N_("[-r|--reboot] XXXX"),
                              N_("Set EFI BootNext to Boot#### and "
                                 "optionally reboot."),
                              options);
}

GRUB_MOD_FINI (efibootnext)
{
  grub_unregister_extcmd (cmd);
}
