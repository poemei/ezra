#ifndef EZRA_ROUTE_H
#define EZRA_ROUTE_H

/* Transport-neutral Core ingress. Existing ABI 1.5 host layout stays unchanged. */
#define EZRA_ROUTER_DISPATCH_SERVICE "digit.router.dispatch"
#define EZRA_ROUTE_TEXT_MAX 2048
#define EZRA_ROUTE_REPLY_MAX 512

typedef struct {
    char sender[128];
    char account[128];
    char text[EZRA_ROUTE_TEXT_MAX];
} ezra_route_request_t;

typedef struct {
    char text[EZRA_ROUTE_REPLY_MAX];
    int replied;
} ezra_route_reply_t;

#endif
