/**
   src/vpu5/shvpu5_common_2ddmac.h

   This component implements H.264 / MPEG-4 AVC video codec.
   The H.264 / MPEG-4 AVC video encoder/decoder is implemented
   on the Renesas's VPU5HG middleware library.

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
#ifndef TDDMAC_H
#define TDDMAC_H
typedef enum {
	TDDMAC_Y,
	TDDMAC_CbCr420,
	TDDMAC_CbCr422,
	TDDMAC_RGB565,
	TDDMAC_RGB24,
	TDDMAC_BGR24,
	TDDMAC_RGB32,
	TDDMAC_ARGB32,
} tddmac_format_t;

struct tddmac_buffer {
	int w;		/**< Buffer width in pixels (dest only)*/
	int h;		/**< Buffer height in pixels (dest only) */
	int pitch; 	/**< Buffer pitch in pixels */
	tddmac_format_t fmt;/**< Buffer format*/
};

static inline int is_yuv_chroma(tddmac_format_t fmt) {
	if (fmt == TDDMAC_CbCr420 || fmt == TDDMAC_CbCr422)
		return 1;
	return 0;
}
static inline int is_rgb_fmt(tddmac_format_t fmt) {
	if (fmt >= TDDMAC_RGB565 && fmt <= TDDMAC_ARGB32)
		return 1;
	return 0;
}

/**
 * An opaque handle to the TDDMAC
 */
struct TDDMAC;
typedef struct TDDMAC TDDMAC;

/**
 * A handle to a DMAC channel
 */
typedef int dmac_id_t;

/**
  * Open a TDDMAC device
  * \retval NULL Failure, otherwise VEU handle
  */
TDDMAC *tddmac_open();

/**
  * Close a TDDMAC device
  */
void tddmac_close(TDDMAC *tddmac);

/** Setup the TDDMAC for transfer.
  *
  * Color format conversion is possible between two RGB formats or
  * between NV12 and NV16.
  * \param tddmac	TDDMAC handle
  * \param src		Input buffer format
  * \param dst		Output buffer format
  * \retval <0		Failure
  * \retval >=0		DMAC channel handle
  */
dmac_id_t tddmac_setup(TDDMAC *tddmac,
	         struct tddmac_buffer *src,
	         struct tddmac_buffer *dst);
/** Start a DMA transfer
  *
  * \param tddmac 	TDDMAC handl
  * \param dmac_id	DMA Channel handle
  * \param from		Input buffer start address
  * \param to		Output buffer start address
  * \retval 0 Success, otherwise error
  */
int tddmac_start(TDDMAC *tddmac,
	dmac_id_t dmac_id,
	unsigned long from,
	unsigned long to);

/** Wait for a DMA transfer to complete
  * 
  * \param tddmac 	TDDMAC handl
  * \param dmac_id	DMA Channel handle
  * \retval 0 Success, otherwise error
  */
int tddmac_wait(TDDMAC *tddmac, dmac_id_t dmac_id);
#endif /* TDDMAC_H */
