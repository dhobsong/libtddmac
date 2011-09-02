/**
   Copyright (C) 2011 Renesas Solutions Corp.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
   02110-1301 USA
*/

#include <stdint.h>
#include <string.h>
#include "tddmac_regs.h"
#include <tddmac/tddmac.h>
#include <uiomux/uiomux.h>


static UIOMux *uiomux;
#define UIOMUX_TDDMAC	(1 << 0)

struct TDDMAC{
	UIOMux *uiomux;
	void *reg_base;
	char dmac_locks[TOTAL_DMA_CHANNELS];
};

struct tddmac_format_info {
	tddmac_format_t	fmt;
	uint32_t	fmt_bits;
	uint32_t	src_swap;
	uint32_t	dst_swap;
};

static const struct tddmac_format_info tddmac_fmts[] = {
	{TDDMAC_Y, FMT_Y, SWAP_ILS | SWAP_IWS | SWAP_IBS,
		SWAP_OLS | SWAP_OWS | SWAP_OBS},
	{TDDMAC_CbCr420, FMT_CbCr_12, SWAP_ILS | SWAP_IWS | SWAP_IBS,
		SWAP_OLS | SWAP_OWS | SWAP_OBS},
	{TDDMAC_CbCr422, FMT_CbCr_16, SWAP_ILS | SWAP_IWS | SWAP_IBS,
		SWAP_OLS | SWAP_OWS | SWAP_OBS},
};

static const struct tddmac_format_info *fmt_info(tddmac_format_t fmt)
{
	int i, fmts;
	fmts = sizeof(tddmac_fmts) / sizeof (tddmac_fmts[0]);
	for (i = 0; i < fmts; i++)
		if (tddmac_fmts[i].fmt == fmt)
			return &tddmac_fmts[i];
	return NULL;
}

static void dmac_write32(void *base, uint32_t val, dmac_id_t dmac, int offset)
{
	volatile uint32_t *reg = (uint32_t *) ((uint8_t *) base +
						DMAC_OFFSET(dmac, offset));

	*reg = val;
}

static uint32_t dmac_read32(void *base, dmac_id_t dmac, int offset)
{
	volatile uint32_t *reg = (uint32_t *) ((uint8_t *) base +
						DMAC_OFFSET(dmac, offset));

	return *reg;
}

static int check_done_bit(void *base, dmac_id_t dmac_id)
{
	volatile uint32_t *reg = (uint32_t *) ((uint8_t *) base + CHSTCLR);
	return (*reg) & (1 << dmac_id);
}

static void clear_done_bit(void *base, dmac_id_t dmac_id)
{
	volatile uint32_t *reg = (uint32_t *) ((uint8_t *) base + CHSTCLR);
	uint32_t tmp = (1 << dmac_id);
	*reg = tmp;
}

TDDMAC *tddmac_open()
{
	TDDMAC *tddmac;
	const char *uio_names[2] = {"2DDMAC", NULL};
	int i;
	uiomux = uiomux_open_named(uio_names);

	if (!uiomux) {
		fprintf(stderr, "Could not initialize TDDMAC UIO\n");
		return NULL;
	}

	tddmac = malloc (sizeof (*tddmac));
	if (!tddmac) {
		fprintf(stderr, "%s: Out of memory\n", __FUNCTION__);
		return NULL;
	}

	memset(tddmac, 0, sizeof(*tddmac));

	tddmac->uiomux = uiomux;
	uiomux_get_mmio (tddmac->uiomux, UIOMUX_TDDMAC, NULL, NULL,
		&tddmac->reg_base);
	return tddmac;
}

static dmac_id_t get_available_dmac_id(TDDMAC *tddmac) {
	int i;
	uiomux_lock(tddmac->uiomux, UIOMUX_TDDMAC);
	for (i = 0; i < TOTAL_DMA_CHANNELS; i++) {
		if(!tddmac->dmac_locks[i]) {
			tddmac->dmac_locks[i] = 1;
			uiomux_unlock(tddmac->uiomux, UIOMUX_TDDMAC);
			return (dmac_id_t) i;
		}
	}
	uiomux_unlock(tddmac->uiomux, UIOMUX_TDDMAC);
	return (dmac_id_t) -1;
}

dmac_id_t tddmac_setup(TDDMAC *tddmac,
	     struct tddmac_buffer *src,
	     struct tddmac_buffer *dst)
{
	unsigned long sfmt_reg;
	unsigned long dfmt_reg;
	dmac_id_t dmac_id;
	const struct tddmac_format_info *sfmt, *dfmt;

	/* cannot convert from luma info to anything else */
	if ((src->fmt == TDDMAC_Y) != (dst->fmt == TDDMAC_Y))
		return -1;

	/* Cannot convert between chroma and RGB formats */
	if (is_yuv_chroma(src->fmt) != is_yuv_chroma(dst->fmt))
		return -1;

	/*get usable DMACs */
	dmac_id = get_available_dmac_id(tddmac);
	if (dmac_id < 0)
		return -1;

	/*Enable interupts */
	dmac_write32(tddmac->reg_base, CTRL_TIE, dmac_id, CHnCTRL);
	
	/* set format information */
	if ((sfmt = fmt_info(src->fmt)) == NULL)
		goto err;
	
	sfmt_reg = sfmt->fmt_bits;

	if ((dfmt = fmt_info(dst->fmt)) == NULL)
		goto err;

	dfmt_reg = dfmt->fmt_bits;

	/* set swap information */
	dmac_write32(tddmac->reg_base, dfmt->dst_swap | sfmt->src_swap,
		dmac_id, CHnSWAP);

	sfmt_reg |= (src->pitch << HBYTES_OFFSET);
	dfmt_reg |= (dst->pitch << HBYTES_OFFSET);
	dmac_write32(tddmac->reg_base, sfmt_reg, dmac_id, CHnSFMT);
	dmac_write32(tddmac->reg_base, dfmt_reg, dmac_id, CHnDFMT);

	dmac_write32(tddmac->reg_base, (dst->w << 16) | dst->h, dmac_id,
		CHnDPXL);
	return dmac_id;

err:
	return -1;
}

void tddmac_close(TDDMAC *tddmac)
{
	uiomux_close(uiomux);
}

int tddmac_start(TDDMAC *tddmac,
	dmac_id_t dmac_id,
	unsigned long from,
	unsigned long to)
{

	/* set buffer */
	dmac_write32(tddmac->reg_base, from, dmac_id, CHnSAR);
	dmac_write32(tddmac->reg_base, to, dmac_id, CHnDAR);

	/* Start transfer */
	dmac_write32(tddmac->reg_base,
		dmac_read32(tddmac->reg_base, dmac_id, CHnCTRL) |
		CTRL_DMAEN, dmac_id, CHnCTRL);

	return 0;
}

int tddmac_wait(TDDMAC *tddmac, dmac_id_t dmac_id)
{
	/* Wait for done */
	while (!check_done_bit(tddmac->reg_base, dmac_id)) {
		uiomux_sleep(tddmac->uiomux, UIOMUX_TDDMAC);
	}
	clear_done_bit(tddmac->reg_base, dmac_id);
	return 0;	
}
