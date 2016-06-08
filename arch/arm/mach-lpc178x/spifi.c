/*
 * linux/arch/arm/mach-lpc18xx/spifi.c
 *
 * Copyright (C) 2014-2015 Emcraft Systems
 *
 * Pavel Boldin <paboldin@emcraft.com>
 * Vladimir Khusainov <vlad@emcraft.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/init.h>
#include <linux/list.h>
#include <linux/mtd/partitions.h>
#include <linux/spi/flash.h>
#include <linux/platform_device.h>

#include <mach/lpc178x.h>
#include <mach/power.h>
#include <mach/platform.h>

/*
 * LPC40XX SPIFI I/O resources
 */
#define LPC40XX_SPIFI_BASE	0x20094000

static struct resource flash_resources[] = {
	{
		.start	= LPC40XX_SPIFI_BASE,
		.end	= LPC40XX_SPIFI_BASE + 0x400,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= 0x28000000,
		.end	= 0x28000000 + 8 * 1024 * 1024,
		.flags	= IORESOURCE_MEM,
	},
};

/*
 * SPIFI Flash partitioning information
 * Winbond W25Q64FV device, size: 8 MB
 * part0: kernel - 2 MB
 * part1: JFFS2  - 6 MB
 */
#define FLASH_KERNEL_OFFSET	(0)
#define FLASH_KERNEL_SIZE	(2*1024*1024)
#define FLASH_JFFS2_OFFSET	FLASH_KERNEL_SIZE
#define FLASH_JFFS2_SIZE	(6*1024*1024)

static struct mtd_partition flash_partitions[] = {
	{
		.name	= "kernel",
		.offset = FLASH_KERNEL_OFFSET,
		.size	= FLASH_KERNEL_SIZE,
	},
	{
		.name	= "jffs2",
		.offset = FLASH_JFFS2_OFFSET,
		.size	= FLASH_JFFS2_SIZE,
	},
};

static struct flash_platform_data flash_data = {
	.nr_parts	= ARRAY_SIZE(flash_partitions),
	.parts		= flash_partitions,
	.type		= "w25q64",
};

/*
 * SPIFI platform info
 */
static struct platform_device flash_dev = {
	.name           = "m25p80_spifi",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(flash_resources),
	.resource       = flash_resources,
	.dev		= {
		.platform_data = &flash_data,
	},
};

/* SPIFI Clock Selection register 
 * SPIFI interface is available only
 * on LPC40XX and LPC1773 devices 
 */
#define LPC40XX_SCC_SPIFICLKSEL_SPIFIDIV_MSK	4
#define LPC40XX_SCC_SPIFICLKSEL_SPIFISEL_MSK	(1 << 8)

void __init lpc40xx_spifi_init(void)
{
	int p = lpc178x_platform_get();

	if (p == PLATFORM_LPC40XX_UCS_LPC4088) {
		/* Setup SPIFI clock */
		LPC178X_SCC->spificlksel = (
			LPC40XX_SCC_SPIFICLKSEL_SPIFISEL_MSK |
			LPC40XX_SCC_SPIFICLKSEL_SPIFIDIV_MSK);
		/* Switch on SPIFI interface module */
		lpc178x_periph_enable(LPC178X_SCC_PCONP_PCSPIFI_MSK, 1);
		/* Register device */
		platform_device_register(&flash_dev);
	}
}
