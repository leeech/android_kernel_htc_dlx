/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "kgsl.h"
#include "adreno.h"
#include "kgsl_snapshot.h"

#define DEBUG_SECTION_SZ(_dwords) (((_dwords) * sizeof(unsigned int)) \
		+ sizeof(struct kgsl_snapshot_debug))


#define SXDEBUG_COUNT 0x1B

static int a2xx_snapshot_sxdebug(struct kgsl_device *device, void *snapshot,
	int remain, void *priv)
{
	struct kgsl_snapshot_debug *header = snapshot;
	unsigned int *data = snapshot + sizeof(*header);
	int i;

	if (remain < DEBUG_SECTION_SZ(SXDEBUG_COUNT)) {
		SNAPSHOT_ERR_NOMEM(device, "SX DEBUG");
		return 0;
	}

	header->type = SNAPSHOT_DEBUG_SX;
	header->size = SXDEBUG_COUNT;

	for (i = 0; i < SXDEBUG_COUNT; i++) {
		adreno_regwrite(device, REG_RBBM_DEBUG_CNTL, 0x1B00 | i);
		adreno_regread(device, REG_RBBM_DEBUG_OUT, &data[i]);
	}

	adreno_regwrite(device, REG_RBBM_DEBUG_CNTL, 0);

	return DEBUG_SECTION_SZ(SXDEBUG_COUNT);
}

#define CPDEBUG_COUNT 0x20

static int a2xx_snapshot_cpdebug(struct kgsl_device *device, void *snapshot,
	int remain, void *priv)
{
	struct kgsl_snapshot_debug *header = snapshot;
	unsigned int *data = snapshot + sizeof(*header);
	int i;

	if (remain < DEBUG_SECTION_SZ(CPDEBUG_COUNT)) {
		SNAPSHOT_ERR_NOMEM(device, "CP DEBUG");
		return 0;
	}

	header->type = SNAPSHOT_DEBUG_CP;
	header->size = CPDEBUG_COUNT;

	for (i = 0; i < CPDEBUG_COUNT; i++) {
		adreno_regwrite(device, REG_RBBM_DEBUG_CNTL, 0x1628);
		adreno_regread(device, REG_RBBM_DEBUG_OUT, &data[i]);
	}

	adreno_regwrite(device, REG_RBBM_DEBUG_CNTL, 0);

	return DEBUG_SECTION_SZ(CPDEBUG_COUNT);
}


#define SQ_DEBUG_WRITE(_device, _reg, _data, _offset) \
	do { _data[(_offset)++] = (_reg); \
	adreno_regread(_device, (_reg), &_data[(_offset)++]); } while (0)

#define SQ_DEBUG_BANK_SIZE 23

static int a2xx_snapshot_sqdebug(struct kgsl_device *device, void *snapshot,
	int remain, void *priv)
{
	struct kgsl_snapshot_debug *header = snapshot;
	unsigned int *data = snapshot + sizeof(*header);
	int i, offset = 0;
	int size = SQ_DEBUG_BANK_SIZE * 2 * 2;

	if (remain < DEBUG_SECTION_SZ(size)) {
		SNAPSHOT_ERR_NOMEM(device, "SQ Debug");
		return 0;
	}

	header->type = SNAPSHOT_DEBUG_SQ;
	header->size = size;

	for (i = 0; i < 2; i++) {
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_CONST_MGR_FSM+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_EXP_ALLOC+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_FSM_ALU_0+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_FSM_ALU_1+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_GPR_PIX+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_GPR_VTX+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_INPUT_FSM+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_MISC+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_MISC_0+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_MISC_1+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_PIX_TB_0+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_PIX_TB_STATE_MEM+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device,
			REG_SQ_DEBUG_PIX_TB_STATUS_REG_0+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device,
			REG_SQ_DEBUG_PIX_TB_STATUS_REG_1+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device,
			REG_SQ_DEBUG_PIX_TB_STATUS_REG_2+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device,
			REG_SQ_DEBUG_PIX_TB_STATUS_REG_3+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_PTR_BUFF+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_TB_STATUS_SEL+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_TP_FSM+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_VTX_TB_0+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_VTX_TB_1+i*0x1000,
			data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_VTX_TB_STATE_MEM+i*0x1000,
			data, offset);
	}

	return DEBUG_SECTION_SZ(size);
}

#define SQ_DEBUG_THREAD_SIZE 7

static int a2xx_snapshot_sqthreaddebug(struct kgsl_device *device,
	void *snapshot, int remain, void *priv)
{
	struct kgsl_snapshot_debug *header = snapshot;
	unsigned int *data = snapshot + sizeof(*header);
	int i, offset = 0;
	int size = SQ_DEBUG_THREAD_SIZE * 2 * 16;

	if (remain < DEBUG_SECTION_SZ(size)) {
		SNAPSHOT_ERR_NOMEM(device, "SQ THREAD DEBUG");
		return 0;
	}

	header->type = SNAPSHOT_DEBUG_SQTHREAD;
	header->size = size;

	for (i = 0; i < 16; i++) {
		adreno_regwrite(device, REG_SQ_DEBUG_TB_STATUS_SEL,
				i | (6<<4) | (i<<7) | (1<<11) | (1<<12)
				| (i<<16) | (6<<20) | (i<<23));
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_VTX_TB_STATE_MEM,
			 data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_VTX_TB_STATUS_REG,
			 data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_PIX_TB_STATE_MEM,
			 data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_PIX_TB_STATUS_REG_0,
			 data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_PIX_TB_STATUS_REG_1,
			 data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_PIX_TB_STATUS_REG_2,
			 data, offset);
		SQ_DEBUG_WRITE(device, REG_SQ_DEBUG_PIX_TB_STATUS_REG_3,
			 data, offset);
	}

	return DEBUG_SECTION_SZ(size);
}

#define MIUDEBUG_COUNT 0x10

static int a2xx_snapshot_miudebug(struct kgsl_device *device, void *snapshot,
	int remain, void *priv)
{
	struct kgsl_snapshot_debug *header = snapshot;
	unsigned int *data = snapshot + sizeof(*header);
	int i;

	if (remain < DEBUG_SECTION_SZ(MIUDEBUG_COUNT)) {
		SNAPSHOT_ERR_NOMEM(device, "MIU DEBUG");
		return 0;
	}

	header->type = SNAPSHOT_DEBUG_MIU;
	header->size = MIUDEBUG_COUNT;

	for (i = 0; i < MIUDEBUG_COUNT; i++) {
		adreno_regwrite(device, REG_RBBM_DEBUG_CNTL, 0x1600 | i);
		adreno_regread(device, REG_RBBM_DEBUG_OUT, &data[i]);
	}

	adreno_regwrite(device, REG_RBBM_DEBUG_CNTL, 0);

	return DEBUG_SECTION_SZ(MIUDEBUG_COUNT);
}


void *a2xx_snapshot(struct adreno_device *adreno_dev, void *snapshot,
	int *remain, int hang)
{
	struct kgsl_device *device = &adreno_dev->dev;
	struct kgsl_snapshot_registers regs;
	unsigned int pmoverride;

	

	if (adreno_is_a20x(adreno_dev)) {
		regs.regs = (unsigned int *) a200_registers;
		regs.count = a200_registers_count;
	} else if (adreno_is_a220(adreno_dev)) {
		regs.regs = (unsigned int *) a220_registers;
		regs.count = a220_registers_count;
	} else if (adreno_is_a225(adreno_dev)) {
		regs.regs = (unsigned int *) a225_registers;
		regs.count = a225_registers_count;
	}

	
	snapshot = kgsl_snapshot_add_section(device,
		KGSL_SNAPSHOT_SECTION_REGS, snapshot, remain,
		kgsl_snapshot_dump_regs, &regs);

	
	snapshot = kgsl_snapshot_indexed_registers(device, snapshot,
			remain, REG_CP_STATE_DEBUG_INDEX,
			REG_CP_STATE_DEBUG_DATA, 0x0, 0x14);

	
	snapshot = kgsl_snapshot_indexed_registers(device, snapshot,
			remain, REG_CP_ME_CNTL, REG_CP_ME_STATUS,
			64, 44);


	adreno_regread(device, REG_RBBM_PM_OVERRIDE2, &pmoverride);
	adreno_regwrite(device, REG_RBBM_PM_OVERRIDE2, 0xFF);

	
	snapshot = kgsl_snapshot_add_section(device,
			KGSL_SNAPSHOT_SECTION_DEBUG, snapshot, remain,
			a2xx_snapshot_sxdebug, NULL);

	
	if (!adreno_is_a22x(adreno_dev))
		snapshot = kgsl_snapshot_indexed_registers(device, snapshot,
				remain, REG_PA_SU_DEBUG_CNTL,
				REG_PA_SU_DEBUG_DATA,
				0, 0x1B);

	
	snapshot = kgsl_snapshot_add_section(device,
			KGSL_SNAPSHOT_SECTION_DEBUG, snapshot, remain,
			a2xx_snapshot_cpdebug, NULL);

	
	snapshot = kgsl_snapshot_indexed_registers(device, snapshot,
			remain, MH_DEBUG_CTRL, MH_DEBUG_DATA, 0x0, 0x40);

	
	if (adreno_is_a22x(adreno_dev)) {
		
		snapshot = kgsl_snapshot_indexed_registers(device, snapshot,
			remain, REG_RB_DEBUG_CNTL, REG_RB_DEBUG_DATA, 0, 8);

		
		snapshot = kgsl_snapshot_indexed_registers(device, snapshot,
			remain, REG_RB_DEBUG_CNTL, REG_RB_DEBUG_DATA + 0x1000,
			0, 8);

		
		snapshot = kgsl_snapshot_indexed_registers(device, snapshot,
			remain, REG_PC_DEBUG_CNTL, REG_PC_DEBUG_DATA, 0, 8);

		
		snapshot = kgsl_snapshot_indexed_registers(device, snapshot,
			remain, REG_GRAS_DEBUG_CNTL, REG_GRAS_DEBUG_DATA, 0, 4);

		
		snapshot = kgsl_snapshot_add_section(device,
			KGSL_SNAPSHOT_SECTION_DEBUG, snapshot, remain,
			a2xx_snapshot_miudebug, NULL);

		
		snapshot = kgsl_snapshot_add_section(device,
			KGSL_SNAPSHOT_SECTION_DEBUG, snapshot, remain,
			a2xx_snapshot_sqdebug, NULL);


		if (hang) {
			
			snapshot = kgsl_snapshot_add_section(device,
				KGSL_SNAPSHOT_SECTION_DEBUG, snapshot, remain,
				a2xx_snapshot_sqthreaddebug, NULL);
		}
	}

	
	adreno_regwrite(device, REG_RBBM_PM_OVERRIDE2, pmoverride);

	return snapshot;
}
