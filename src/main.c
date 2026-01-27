#include <pthread.h>
#include "mongoose.h"
#include "capture.h"
#include "cJSON.h"

static void ev_handler(struct mg_connection* c, int ev, void* ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_match(hm->uri, mg_str("/"), NULL)) {
            struct mg_http_serve_opts opts = {0};
            mg_http_serve_file(c, hm, "web/index.html", &opts); 
        }
        else if (mg_match(hm->uri, mg_str("/api/capture/start"), NULL)) {
            pthread_t thread_id = 1;
            char* interface = "wlp0s20f3";
            if (pthread_create(&thread_id, NULL, capture_packets, (void *)interface) == -1) {
                mg_http_reply(c, 400, "", "Failed to start listener\n");
                fprintf(stderr, "Failed to start listener");
            }

            mg_http_reply(c, 200, "", "Successfully started listener\n");
        } else {
            mg_http_reply(c, 404, "", "Not found\n");
        }
    }
}

int main() {
    struct mg_mgr mgr;

    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:5000", ev_handler, NULL);

    printf("HTTP server started on port 5000\n");

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
    return 0;
}