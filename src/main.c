#include <pthread.h>
#include "mongoose.h"
#include "capture.h"
#include "cJSON.h"

struct app_state_t {
    pthread_t capture_tid;
    volatile bool should_capture;
    packet_queue_t *queue;
}; 

static void ev_handler(struct mg_connection* c, int ev, void* ev_data) {
    struct app_state_t* state = (struct app_state_t*) c->fn_data;
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_match(hm->uri, mg_str("/"), NULL)) {
            struct mg_http_serve_opts opts = {0};
            mg_http_serve_file(c, hm, "web/index.html", &opts); 
        }
        else if (mg_match(hm->uri, mg_str("/api/capture/start"), NULL)) {
            if (!state->should_capture){
                state->should_capture = true;
                char* if_string = malloc(10);
                strcpy(if_string, "wlp0s20f3");
                struct packet_capture_thread_args* args = malloc(sizeof(*args));
                args->packet_queue = state->queue;
                args->should_capture = &state->should_capture; 
                args->interface = if_string;
                if (pthread_create(&state->capture_tid, NULL, capture_packets, (void *)args) != 0) {
                    mg_http_reply(c, 500, "", "Failed to start listener\n");
                }

                mg_http_reply(c, 200, "", "Successfully started listener\n");
            } else {
                mg_http_reply(c, 400, "", "Already running\n");
            }
        } else if (mg_match(hm->uri, mg_str("/api/capture/stop"), NULL)) {
            if (state->should_capture) {
                state->should_capture = false; 
                pthread_join(state->capture_tid, NULL);
                mg_http_reply(c, 200, "", "Sniffer stopped\n");
            } else {
                mg_http_reply(c, 400, "", "Packet listener not currently running");
            }
        } else if (mg_match(hm->uri, mg_str("/api/capture/stream"), NULL)) {
            mg_printf(c, "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/event-stream\r\n"
                "Cache-Control: no-cache\r\n"
                "Connection: keep-alive\r\n\r\n"
            );
            
            c->data[0] = 'S';
        } else {
            mg_http_reply(c, 404, "", "Not found\n");
        }
    } else if (ev == MG_EV_POLL && c->data[0] == 'S') {
        while (true) {
            char *json = packet_queue_try_pop(state->queue); 
            if (!json) break;

            mg_printf(c, "data: %s\n\n", json);
        }
    }
}

int main() {
    struct mg_mgr mgr;

    mg_mgr_init(&mgr);
    struct app_state_t app_state = {0};

    app_state.queue = malloc(sizeof *app_state.queue);
    packet_queue_init(app_state.queue);

    mg_http_listen(&mgr, "http://0.0.0.0:5000", ev_handler, (void*) &app_state);
    mg_wakeup_init(&mgr);

    printf("HTTP server started on port 5000\n");

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
    return 0;
}