#ifndef W_KERNEL_WIFI_H
#define W_KERNEL_WIFI_H

#include "kernel.h"
#include "terminal.h"

#include "pico/cyw43_arch.h"

static void wifi_init(void)
{
	if(cyw43_arch_init())
		panic("wifi init failed\n");

	cyw43_arch_enable_sta_mode();
	cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
}

static int print_scan(__unused void *env, const cyw43_ev_scan_result_t *result)
{
	if (result) {
		// printk("ssid: %s rssi: %d chan: %d mac: %x:%x:%x:%x:%x:%x sec: %u\n",
		//     result->ssid, result->rssi, result->channel,
		//     result->bssid[0], result->bssid[1], result->bssid[2], result->bssid[3], result->bssid[4], result->bssid[5],
		//     result->auth_mode);
		printk("ssid: %s\n", result->ssid);
	}
	return 0;
}

static void wifi_scan(void)
{
	// do not allow to rescan to avoid deadlocking
	if (cyw43_wifi_scan_active(&cyw43_state))
		return;

	cyw43_wifi_scan_options_t scan_options = {0};
	int rv = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, print_scan);
	if (!rv && !cyw43_wifi_scan_active(&cyw43_state)) {
		printk("wifi scan failed\n");
	}
}

#endif