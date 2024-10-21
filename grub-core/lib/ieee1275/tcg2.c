/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2024 IBM Corporation
 *  Copyright (C) 2024 Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <grub/types.h>
#include <grub/tpm.h>
#include <grub/ieee1275/tpm.h>
#include <grub/mm.h>
#include <grub/misc.h>

#include <tcg2.h>

grub_ieee1275_ihandle_t grub_ieee1275_tpm_ihandle;
grub_uint8_t grub_ieee1275_tpm_version;

#define IEEE1275_IHANDLE_INVALID ((grub_ieee1275_ihandle_t) 0)

static void
tpm_get_tpm_version (void)
{
  grub_ieee1275_phandle_t vtpm;
  char buffer[20];

  if (!grub_ieee1275_finddevice ("/vdevice/vtpm", &vtpm) &&
      !grub_ieee1275_get_property (vtpm, "compatible", buffer,
				   sizeof (buffer), NULL) &&
      !grub_strcmp (buffer, "IBM,vtpm20"))
    grub_ieee1275_tpm_version = 2;
}

grub_err_t
grub_ieee1275_tpm_init (void)
{
  static int init_success = 0;

  if (!init_success)
    {
      if (grub_ieee1275_open ("/vdevice/vtpm", &grub_ieee1275_tpm_ihandle) < 0)
	{
	  grub_ieee1275_tpm_ihandle = IEEE1275_IHANDLE_INVALID;
	  return GRUB_ERR_UNKNOWN_DEVICE;
	}

      init_success = 1;

      tpm_get_tpm_version ();
    }

  return GRUB_ERR_NONE;
}

grub_err_t
grub_tcg2_get_max_output_size (grub_size_t *size)
{
  struct tpm_get_maximum_cmd_size
  {
    struct grub_ieee1275_common_hdr common;
    grub_ieee1275_cell_t method;
    grub_ieee1275_cell_t ihandle;
    grub_ieee1275_cell_t catch_result;
    grub_ieee1275_cell_t size;
  };
  struct tpm_get_maximum_cmd_size args;
  static int error_displayed = 0;
  grub_err_t err;

  err = grub_ieee1275_tpm_init ();
  if (err != GRUB_ERR_NONE)
      return err;

  INIT_IEEE1275_COMMON (&args.common, "call-method", 2, 2);
  args.method = (grub_ieee1275_cell_t) "get-maximum-cmd-size";
  args.ihandle = grub_ieee1275_tpm_ihandle;

  if (IEEE1275_CALL_ENTRY_FN (&args) == -1)
    return GRUB_ERR_INVALID_COMMAND;

  /*
   * catch_result is set if firmware does not support get-maximum-cmd-size
   * rc is GRUB_IEEE1275_CELL_FALSE (0) on failure
   */
  if (args.catch_result)
    {
      if (!error_displayed)
        {
          error_displayed++;
          return grub_error (GRUB_ERR_BAD_DEVICE,
                             "get-maximum-cmd-size failed: Firmware is likely too old.\n");
        }
      return GRUB_ERR_INVALID_COMMAND;
    }

  *size = args.size;

  return GRUB_ERR_NONE;
}

grub_err_t
grub_tcg2_submit_command (grub_size_t input_size,
			  grub_uint8_t *input,
			  grub_size_t output_size,
			  grub_uint8_t *output)
{
  struct tpm_pass_through_to_tpm
  {
    struct grub_ieee1275_common_hdr common;
    grub_ieee1275_cell_t method;
    grub_ieee1275_cell_t ihandle;
    grub_ieee1275_cell_t buf_size;
    grub_ieee1275_cell_t buf_addr;
    grub_ieee1275_cell_t catch_result;
    grub_ieee1275_cell_t resp_size;
  };
  struct tpm_pass_through_to_tpm args;
  static int error_displayed = 0;
  grub_err_t err;

  if (input_size == 0  || input == NULL ||
      output_size == 0 || output == NULL)
    return GRUB_ERR_BAD_ARGUMENT;

  err = grub_ieee1275_tpm_init ();
  if (err != GRUB_ERR_NONE)
      return err;

  INIT_IEEE1275_COMMON (&args.common, "call-method", 4, 2);
  args.method = (grub_ieee1275_cell_t) "pass-through-to-tpm";
  args.ihandle = grub_ieee1275_tpm_ihandle;
  args.buf_size = (grub_ieee1275_cell_t) input_size;
  args.buf_addr = (grub_ieee1275_cell_t) input;

  if (IEEE1275_CALL_ENTRY_FN (&args) == -1)
    return GRUB_ERR_INVALID_COMMAND;

  /*
   * catch_result is set if firmware does not support pass-through-to-tpm
   */
  if (args.catch_result)
    {
      if (!error_displayed)
        {
          error_displayed++;
          return grub_error (GRUB_ERR_BAD_DEVICE,
                             "pass-through-to-tpm failed: Firmware is likely too old.\n");
        }
      return GRUB_ERR_INVALID_COMMAND;
    }

  grub_memcpy (output, input, args.resp_size);

  return GRUB_ERR_NONE;
}
