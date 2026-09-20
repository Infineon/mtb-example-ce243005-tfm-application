/******************************************************************************
 * File Name        : main.c
 *
 * Description      : This is the source code for Main CM33 non-secure application
 *
 * Related Document : See README.md
 *
 *******************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
 *******************************************************************************/

/*******************************************************************************
 * Header Files
 ********************************************************************************/

#include "cy_pdl.h"
#include "cybsp.h"
#include "tfm_ns_interface.h"
#include "os_wrapper/common.h"
#include "psa/internal_trusted_storage.h"
#include "ifx_platform_api.h"
#include <stdio.h>

/*******************************************************************************
 * Macros
 ********************************************************************************/

/* Internal Trusted Storage UID */
#define ITS_UID                     (1U)

/* Buffer size for Internal Trusted Storage */
#define ITS_BUFF_SIZE               (20U)

/* These are the flash addresses where the PPCA core0 and core1 images are located. */
#define CORE0_IMAGE_ADDRESS (CYMEM_CM33_0_m33ns_ppca0_nvm_START + MCUBOOT_HEADER_SIZE)
#define CORE1_IMAGE_ADDRESS (CYMEM_CM33_0_m33ns_ppca1_nvm_START + MCUBOOT_HEADER_SIZE)

#define CORE0_IMAGE_SIZE (CYMEM_CM33_0_m33ns_ppca0_nvm_SIZE - MCUBOOT_HEADER_SIZE)
#define CORE1_IMAGE_SIZE (CYMEM_CM33_0_m33ns_ppca1_nvm_SIZE - MCUBOOT_HEADER_SIZE)

/* LED Toggle interval */
#define LED_TOGGLE_INTERVAL_MS 1000U

/* Buffer size available to send message to SPE */
#define LOG_BUFFER_SIZE (256)



/*******************************************************************************
 * Global Variables
 ********************************************************************************/

/* Log buffer */
char out_buf[256];

/*******************************************************************************
 * Function Prototypes
 ********************************************************************************/


 /*******************************************************************************
 * Function Name: main
 ********************************************************************************
 * Summary:
 *  This is the main function for CM33 CPU. It does...
 *    1. LED 3 Blink
 *    2. Read shared memory updated from PPCA cores and print values using NSC
 *
 * Parameters:
 *  void
 *
 * Return:
 *  int
 *
 *******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    uint32_t rslt;
    char set_data[] = "Hello World";
    char get_data[ITS_BUFF_SIZE] = {0};
    size_t get_len = 0;
    psa_status_t status;
    int buf_size;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        /* Board init failed. Stop program execution */
        CY_ASSERT(0);
    }

    /* Enable interrupts */
    __enable_irq();

    /* Initialize TF-M interface */
    rslt = tfm_ns_interface_init();
    if(rslt != OS_WRAPPER_SUCCESS)
    {
        CY_ASSERT(0);
    }


    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    buf_size = sprintf((char*)out_buf, "\x1b[2J\x1b[;H"
                "******* "
                "PSOC Control C3M8: Basic Trusted Firmware-M (TF-M) based Application "
                "******* \r\n\n");
    ifx_platform_log_msg(out_buf, buf_size);


    buf_size = sprintf((char*)out_buf, "*** TF-M Internal Trusted Storage (ITS) service ***\r\n\n");
    ifx_platform_log_msg(out_buf, buf_size);

    buf_size = sprintf((char*)out_buf, "ITS Storage data: %s\r\n", set_data);
    ifx_platform_log_msg(out_buf, buf_size);

    buf_size = sprintf((char*)out_buf, "Storing data in ITS...\r\n\n");
    ifx_platform_log_msg(out_buf, buf_size);

    /* Start of Internal Trusted Storage code.
     * Internal Trusted Storage can store upto 10 assets. The maximum size of asset
     * can be upto 512 bytes.
     */
    status = psa_its_set(ITS_UID, sizeof(set_data), set_data, PSA_STORAGE_FLAG_NONE);
    if(status != PSA_SUCCESS)
    {
        CY_ASSERT(0);
    }

    buf_size = sprintf((char*)out_buf, "Retrieving data from ITS...\r\n");
    ifx_platform_log_msg(out_buf, buf_size);

    status = psa_its_get(ITS_UID, 0, sizeof(set_data), get_data, &get_len);
    if(status != PSA_SUCCESS)
    {
        CY_ASSERT(0);
    }

    buf_size = sprintf((char*)out_buf, "Retrieved data: %s\r\n\n", get_data);
    ifx_platform_log_msg(out_buf, buf_size);

    /* Start PPCA Cores */
    buf_size = sprintf((char*)out_buf, "Starting PPCA CM33 cores\r\n");
    ifx_platform_log_msg(out_buf, buf_size);

    Cy_System_Init_CPU0((void *)CORE0_IMAGE_ADDRESS, CORE0_IMAGE_SIZE);
    Cy_System_Init_CPU1((void *)CORE1_IMAGE_ADDRESS, CORE1_IMAGE_SIZE);

    for (;;)
    {
        /* Toggle LED */
        Cy_GPIO_Inv(CYBSP_USER_LED3_PORT, CYBSP_USER_LED3_PIN);
        Cy_SysLib_Delay(LED_TOGGLE_INTERVAL_MS);
    }
}
