/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2019  Free Software Foundation, Inc.
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

#ifndef GRUB_IEEE1275_TPM_HEADER
#define GRUB_IEEE1275_TPM_HEADER      1

#include <grub/err.h>
#include <grub/types.h>
#include <grub/ieee1275/ieee1275.h>

extern grub_ieee1275_ihandle_t grub_ieee1275_tpm_ihandle;
extern grub_uint8_t grub_ieee1275_tpm_version;

grub_err_t grub_ieee1275_tpm_init (void);

#endif
