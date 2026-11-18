/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "boothush-floppy-write.h"
#include "../../arch/i286/c16.h"
#if !defined(__WATCOMC__) || !defined(__I86__)
#error This provider requires the admitted Open Watcom native16 target.
#endif
CS_C16_ASSERT(cs_floppy_write_near_data,sizeof(void *)==2);
CS_C16_ASSERT(cs_floppy_write_near_code,sizeof(cs_block_read_one)==2);
static uint16_t native_ds(void);
#pragma aux native_ds = "mov ax,ds" value [ax] modify exact [ax];
/* Return original AH in the high byte and original carry in bit0. */
static uint16_t write_sector(uint16_t cx,uint16_t dx,uint8_t *buffer);
#pragma aux write_sector = \
    "pushf" "push ds" "push es" "push si" "push di" "push bp" \
    "push ds" "pop es" "mov ax,0301h" "int 13h" "mov al,0" "adc al,0" \
    "pop bp" "pop di" "pop si" "pop es" "pop ds" "popf" "cld" \
    parm [cx] [dx] [bx] value [ax] modify exact [ax bx cx dx];
static int range_fits(uint32_t first,uint32_t blocks)
{return blocks!=0 && first<CS_PC286_FLOPPY_BLOCKS && blocks<=(uint32_t)CS_PC286_FLOPPY_BLOCKS-first;}
int cs_pc286_floppy_write_bind(cs_pc286_floppy_writer *writer,uint32_t first,uint32_t blocks)
{
    uint16_t segment=native_ds();
    if(writer==NULL || !range_fits(first,blocks) ||
        !cs_pc286_floppy_buffer_fits(segment,(uint16_t)(uintptr_t)writer->bounce))return 0;
    writer->first_sector=(uint16_t)first;writer->blocks=(uint16_t)blocks;writer->data_segment=segment;
    return 1;
}
cs_pc286_write_result cs_pc286_floppy_write_one(cs_pc286_floppy_writer *writer,
    uint32_t lba,const uint8_t *source)
{
    uint16_t physical_sector,cylinder,head,sector,offset,status;
    if(writer==NULL || source==NULL || source==writer->bounce)return CS_PC286_WRITE_ARGUMENT;
    if(writer->data_segment!=native_ds() || !range_fits(writer->first_sector,writer->blocks) ||
        !cs_pc286_floppy_buffer_fits(writer->data_segment,(uint16_t)(uintptr_t)writer->bounce))return CS_PC286_WRITE_STATE;
    if(lba>=writer->blocks)return CS_PC286_WRITE_RANGE;
    physical_sector=(uint16_t)(writer->first_sector+(uint16_t)lba);
    cylinder=(uint16_t)(physical_sector/30u);
    head=(uint16_t)((physical_sector/15u)%2u);
    sector=(uint16_t)(physical_sector%15u+1u);
    for(offset=0;offset<CS_BLOCK_BYTES;++offset)writer->bounce[offset]=source[offset];
    status=write_sector((uint16_t)((cylinder<<8)|sector),(uint16_t)(head<<8),writer->bounce);
    if(status==0)return CS_PC286_WRITE_OK;
    if(status==0x0301u)return CS_PC286_WRITE_PROTECTED;
    return CS_PC286_WRITE_IO;
}
