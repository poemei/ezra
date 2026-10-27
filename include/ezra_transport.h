#ifndef EZRA_TRANSPORT_H
#define EZRA_TRANSPORT_H

#define EZRA_TRANSPORT_CHANNEL_SEND_SERVICE "digit.transport.channel.send"
#define EZRA_TRANSPORT_PRIVATE_SEND_SERVICE "digit.transport.private.send"

#define EZRA_TRANSPORT_TARGET_MAX 128
#define EZRA_TRANSPORT_MESSAGE_MAX 512

typedef struct {
    char message[EZRA_TRANSPORT_MESSAGE_MAX];
} ezra_transport_channel_request_t;

typedef struct {
    char target[EZRA_TRANSPORT_TARGET_MAX];
    char message[EZRA_TRANSPORT_MESSAGE_MAX];
} ezra_transport_private_request_t;

#endif
