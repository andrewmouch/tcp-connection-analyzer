#include "mongoose.h"
#include <pthread.h>

static void ev_handler(struct mg_connection* c, int ev, void* ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_match(hm->uri, mg_str("/hey"), NULL)) {
            mg_http_reply(c, 200, "", "Hey\n");
        }
    }
}

int main() {
    struct mg_mgr mgr;

    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:5000", ev_handler, NULL);

    printf("HTTP server started on port 8000\n");

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
    return 0;
}