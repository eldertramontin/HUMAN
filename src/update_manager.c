/*
 * update_manager.c
 *
 *  Created on: 3 de set de 2018
 *      Author: elder
 */
#include "update_manager.h"
#include "../device/flash_memory.h"
#include "../device/brave.h"
#include "../device/utmc.h"
#include "../misc/project_definitions.h"

uint32_t image_in_use;
uint32_t image_alternative;

#pragma PERSISTENT(FRAM_sector_data)
volatile uint8_t FRAM_sector_data[SECTOR_SIZE] = {0xFF};

void update_target(void)
{
    brave_power_on();

    utmc_power_on();
}

void handle_new_bitstream_segment(uint8_t *new_segment, uint16_t sequence_number)
{
    uint16_t i, position_in_sector;
    uint32_t sector_to_modify;

    sector_to_modify = (sequence_number * BITSTREAM_SEGMENT_SIZE) / SECTOR_SIZE;
    position_in_sector = (sequence_number * BITSTREAM_SEGMENT_SIZE) % SECTOR_SIZE;

    memory_read(image_in_use + sector_to_modify, (uint8_t *) FRAM_sector_data, SECTOR_SIZE);

    for(i = 0; (i < BITSTREAM_SEGMENT_SIZE) && ((i + position_in_sector) < SECTOR_SIZE); i++)
    {
        FRAM_sector_data[i + position_in_sector] = new_segment[i];
    }

    memory_sector_erase(image_in_use + sector_to_modify);
    memory_page_program(image_in_use + sector_to_modify, (uint8_t *) FRAM_sector_data, SECTOR_SIZE);

    if(i < BITSTREAM_SEGMENT_SIZE)
    {
        memory_read(image_in_use + sector_to_modify + 1, (uint8_t *) FRAM_sector_data, SECTOR_SIZE);

        for(; i < BITSTREAM_SEGMENT_SIZE; i++)
        {
            FRAM_sector_data[i + position_in_sector - SECTOR_SIZE] = new_segment[i];
        }

        memory_sector_erase(image_in_use + sector_to_modify + 1);
        memory_page_program(image_in_use + sector_to_modify + 1, (uint8_t *) FRAM_sector_data, SECTOR_SIZE);
    }

#ifdef DIAGNOSTIC_MODE
    memory_read(image_in_use + sector_to_modify, (uint8_t *) FRAM_sector_data, SECTOR_SIZE);
    for(i = 0; i < BITSTREAM_SEGMENT_SIZE; i++)
    {
        if(FRAM_sector_data[i + position_in_sector] != new_segment[i])
        {
            throw_error(FLASH_MEMORY_CORRUPTED_SECTOR);
        }
    }
#endif
}

void swap_image_in_use(uint8_t version)
{
    uint32_t current_sector_address;
    uint32_t image_to_swap;

    if(version == 1)
    {
        image_to_swap = IMAGE_A_COPY_1_ADDRESS;
    }
    else
    {
        image_to_swap = IMAGE_B_COPY_1_ADDRESS;
    }

    for(current_sector_address = 0; current_sector_address < IMAGE_MAX_SIZE; current_sector_address += SECTOR_SIZE)
    {
        memory_read(image_to_swap + current_sector_address, (uint8_t *) FRAM_sector_data, SECTOR_SIZE);

        memory_sector_erase(RUNNING_IMAGE_ADDRESS + current_sector_address);
        memory_page_program(RUNNING_IMAGE_ADDRESS + current_sector_address, (uint8_t *) FRAM_sector_data, SECTOR_SIZE);
    }
}
