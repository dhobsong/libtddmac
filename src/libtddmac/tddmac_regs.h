#define TOTAL_DMA_CHANNELS	8

#define CHSTCLR		0x10
#define CHnCTRL		0x20
#define CTRL_DMAEN	(1 << 0)
#define CTRL_TE		(1 << 4)
#define CTRL_TIE	(1 << 5)
#define CHnSWAP		0x30
#define SWAP_OLS	(1 << 6)
#define SWAP_OWS	(1 << 5)
#define SWAP_OBS	(1 << 4)
#define SWAP_ILS	(1 << 2)
#define SWAP_IWS	(1 << 1)
#define SWAP_IBS	(1 << 0)
#define CHnSAR		0x80
#define CHnDAR		0x84
#define CHnDPXL		0x88
#define CHnSFMT		0x8C
#define HBYTES_OFFSET	16
#define HBYTES_MASK 	(0xFFFF << HBYTES_OFFSET)
#define FMT_RGB		(0 << 5)
#define FMT_Y		(1 << 5)
#define FMT_CbCr_12	(2 << 5)
#define FMT_CbCr_16	(3 << 5)
#define FMT_MASK	(3 << 5)
#define CHnDFMT		0x90
#define CHnSARE		0x94
#define CHnDARE		0x98
#define CHnDPXLE	0x9C

#define VALIGN(x) ((x + 15) & ~15)

#define DMAC_OFFSET(dmac, offset)	\
	((offset == CHnCTRL || offset == CHnSWAP) ? ((dmac & 4) << 6) | \
		((dmac << 2) & 0xf) | offset : offset + (dmac << 5))
