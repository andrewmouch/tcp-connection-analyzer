#include "parsers/context.h"

int jsonify_packet_ctx(packet_ctx_t* ctx, cJSON* out) {
    if (!cJSON_AddStringToObject(out, "src_ip", ctx->ipv4.source_ip_address_dotted_quad)) {
        return -1;
    }
    if (!cJSON_AddStringToObject(out, "dst_ip", ctx->ipv4.dest_ip_address_dotted_quad)) {
        return -1;
    }
    return 0;
}