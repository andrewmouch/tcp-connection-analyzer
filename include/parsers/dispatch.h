#pragma once

#include "parsers/context.h"
#include "parsers/types.h"

parse_status_t dispatch_link_layer(packet_ctx_t* ctx);

parse_status_t dispatch_network_layer(packet_ctx_t* ctx);

parse_status_t dispatch_transport_layer(packet_ctx_t* ctx);