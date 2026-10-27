#ifndef EZRA_IRC_INTERNAL_H
#define EZRA_IRC_INTERNAL_H

#include <stddef.h>

#define EZRA_IRC_ENV_PATH "./auth/irc/irc.env"
#define EZRA_IRC_SERVER_MAX 256
#define EZRA_IRC_NICK_MAX 64
#define EZRA_IRC_CHANNEL_MAX 128
#define EZRA_IRC_ACCOUNT_MAX 128
#define EZRA_IRC_PASSWORD_MAX 256
#define EZRA_IRC_LINE_MAX 4096

typedef struct {
    char server[EZRA_IRC_SERVER_MAX];
    unsigned short port;
    char nick[EZRA_IRC_NICK_MAX];
    char channel[EZRA_IRC_CHANNEL_MAX];
    char account[EZRA_IRC_ACCOUNT_MAX];
    char password[EZRA_IRC_PASSWORD_MAX];
} ezra_irc_config_t;

int ezra_irc_config_load(const char *path, ezra_irc_config_t *config);
void ezra_irc_config_clear(ezra_irc_config_t *config);

int ezra_irc_line_command(const char *line, char *command, size_t command_size,
                            const char **params_out);
int ezra_irc_word_present(const char *text, const char *word);
int ezra_irc_build_plain(const char *account, const char *password,
                           char *output, size_t output_size);
int ezra_irc_parse_privmsg(const char *line,
                             char *sender, size_t sender_size,
                             char *target, size_t target_size,
                             char *message, size_t message_size);
int ezra_irc_run_selftests(unsigned int *executed, unsigned int *passed,
                             int *negative_executed, int *negative_passed);

#endif
