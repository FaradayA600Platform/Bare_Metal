/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_mmc_test.h
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	 DESCRIPTION
 * 2020/1       Sanjin Liu       Header file for ftsdc021_mmc_test.c
 * -----------------------------------------------------------------------------
 */
#ifndef FTSDC021_MMC_TEST_H
#define FTSDC021_MMC_TEST_H

#include "../portme_ftsdc021.h"

int32_t SDC_rw_ext_csd_cmd(int32_t argc, char *const argv[]);
int32_t SDC_CQE_enable(int32_t argc, char *const argv[]);
int32_t SDC_packed_test(int32_t argc, char *const argv[]);
int32_t SDC_boot_size_cmd(int32_t argc, char *const argv[]);
int32_t SDC_boot_part_cmd(int32_t argc, char *const argv[]);
int32_t SDC_boot_ack_cmd(int32_t argc, char *const argv[]);
int32_t SDC_boot_read_cmd(int32_t argc, char *const argv[]);
int32_t SDC_boot_write_cmd(int32_t argc, char *const argv[]);
int32_t SDC_go_boot_image(int32_t argc, char *const argv[]);
int32_t SDC_write_protection_cmd(int32_t argc, char *const argv[]);
int32_t SDC_reliable_write_test(int32_t argc, char *const argv[]);
int32_t SDC_context_test(int32_t argc, char *const argv[]);
int32_t SDC_power_off(int32_t argc, char *const argv[]);
int32_t SDC_rpmb_cmd(int32_t argc, char *const argv[]);
int32_t SDC_set_time(int32_t argc, char *const argv[]);

#endif
