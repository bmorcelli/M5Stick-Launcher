/*
 * SPDX-FileCopyrightText: 2015-2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdbool.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "bootloader_init.h"
#include "bootloader_utility.h"
#include "bootloader_common.h"

static const char* TAG = "boot";

static int select_partition_number(bootloader_state_t *bs);

/*
 * We arrive here after the ROM bootloader finished loading this second stage bootloader from flash.
 * The hardware is mostly uninitialized, flash cache is down and the app CPU is in reset.
 * We do have a stack, so we can do the initialization in C.
 */
void __attribute__((noreturn)) call_start_cpu0(void)
{
    // 1. Hardware initialization
    if (bootloader_init() != ESP_OK) {
        bootloader_reset();
    }

    // 2. Select the number of boot partition
    bootloader_state_t bs = {0};
    int boot_index = select_partition_number(&bs);
    if (boot_index == INVALID_INDEX) {
        bootloader_reset();
    }


    /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= HERE IS WHERE THE MAGIC HAPPENS!  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=  */


    esp_rom_printf("[%s] Turned on because (1= POWERON_RESET) (Other= Probably forced by launcher)--> %d\n", TAG, esp_rom_get_reset_reason(0));

    if(esp_rom_get_reset_reason(0)==1) { //Verifica se foi ligado (poweron_reset==1) ou reset por aplicação
        ESP_LOGE(TAG, "## ESP turned on manually, as expected.");
        boot_index = TEST_APP_INDEX;
    } 


    /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=   HERE IS WHERE THE MAGIC ENDs!   =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=  */


    
    // 3. Load the app image for booting
    bootloader_utility_load_boot_image(&bs, boot_index);
}

// Select the number of boot partition
static int select_partition_number(bootloader_state_t *bs)
{
    // 1. Load partition table
    if (!bootloader_utility_load_partition_table(bs)) {
        ESP_LOGE(TAG, "load partition table error!");
        return INVALID_INDEX;
    }

    // 2. Select the number of boot partition
    return bootloader_utility_get_selected_boot_partition(bs);
}

// Return global reent struct if any newlib functions are linked to bootloader
struct _reent *__getreent(void)
{
    return _GLOBAL_REENT;
}
