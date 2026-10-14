#pragma once
#include "app_network.h"

// Compatibility wrapper for app_wifi functions using app_network
#define app_wifi_init app_network_init
#define app_wifi_start app_network_start
