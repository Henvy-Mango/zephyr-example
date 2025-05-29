/*
 * Copyright (c) 2018 Savoir-Faire Linux.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

int main(void)
{
	printf("Hello, World!\n");
	const struct device *const dev = DEVICE_DT_GET_ANY(mpr121);
	
	if(!device_is_ready(dev)) {
		LOG_ERR("Device %s is not ready", dev->name);
		return -ENODEV;
	}

	// if (!dev) {
	// 	LOG_ERR("No devices with compatible nxp,mpr121 found");
	// 	return 0;
	// } else if (!device_is_ready(dev)) {
	// 	LOG_ERR("LED device %s is not ready", dev->name);
	// 	return 0;
	// } else {
	// 	LOG_INF("Found LED device %s", dev->name);
	// }

	LOG_INF("Testing","");
	return 0;
}
