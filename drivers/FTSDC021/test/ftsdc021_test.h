/**
 * -----------------------------------------------------------------------------
 * 	Copyright  Faraday Technology Corp 2010-2014.  All rights reserved.
 * -----------------------------------------------------------------------------
 * FILENAME:  ftsdc021_test.h
 * DEPARTMENT :CTD/SD
 * VERSION: Revision:1.0
 * -----------------------------------------------------------------------------
 *  MAJOR REVISION HISTORY
 * DATE        	AUTHOR       	 DESCRIPTION
 *
 * -----------------------------------------------------------------------------
 */
#ifndef FTSDC021_TEST_H
#define FTSDC021_TEST_H

#include "../portme_ftsdc021.h"
#include "../core/ftsdc021.h"
#include "../core/ftsdc021_cqhci.h"
#include "../core/ftsdc021_cprm.h"
#include "../core/ftsdc021_rpmb.h"
#include "ftsdc021_mmc_test.h"
#include "ftsdc021_sdio_test.h"

/* Command functions */
int32_t SDC_burnin_test(int32_t argc, char *const argv[]);
int32_t SDC_abort_test(int32_t argc, char *const argv[]);
int32_t SDC_cprm_test(int32_t argc, char *const argv[]);
int32_t SDC_settrans_cmd(int32_t argc, char *const argv[]);
int32_t SDC_setflow_cmd(int32_t argc, char *const argv[]);
int32_t SDC_setInfinite_mode(int32_t argc, char *const argv[]);
int32_t SDC_switch_cmd(int32_t argc, char *const argv[]);
int32_t SDC_set_clock(int32_t argc, char *const argv[]);
int32_t SDC_set_base_clock(int32_t argc, char *const argv[]);
int32_t SDC_bus_width_cmd(int32_t argc, char *const argv[]);
int32_t SDC_read_scr_cmd(int32_t argc, char *const argv[]);
int32_t SDC_send_abort_cmd(int32_t argc, char *const argv[]);
int32_t SDC_read_card_status(int32_t argc, char *const argv[]);
int32_t SDC_repo_written_num(int32_t argc, char *const argv[]);
int32_t SDC_tune_cmd(int32_t argc, char *const argv[]);
int32_t SDC_read_cmd(int32_t argc, char *const argv[]);
int32_t SDC_write_cmd(int32_t argc, char *const argv[]);
int32_t SDC_erase_cmd(int32_t argc, char *const argv[]);
int32_t SDC_discard_cmd(int32_t argc, char *const argv[]);
int32_t SDC_software_reset(int32_t argc, char *const argv[]);
int32_t SDC_scan_card_cmd(int32_t argc, char *const argv[]);
int32_t SDC_ShowDscpTbl(int32_t argc, char *const argv[]);
int32_t SDC_dump_registers(int32_t argc, char *const argv[]);
int32_t SDC_performance_test(int32_t argc, char *const argv[]);

uint32_t SDC_data_check_for_erasing(uint32_t start_block, uint32_t blk_cnt);
uint32_t SDC_readWrite_burnin(uint32_t * result, uint32_t mins, uint8_t cnt_tp,
			    uint8_t ran_cnt_tp);
uint32_t SDC_readWrite_auto(uint32_t mins, uint8_t tp_list, uint8_t md_list,
			  uint8_t cnt_tp, uint8_t ran_cnt_tp);
uint32_t SDC_burnin_speed_and_width(uint32_t mins, uint8_t md_list, uint8_t cnt_tp,
				  uint8_t ran_cnt_tp);
void SDC_test_init(void);

#endif
