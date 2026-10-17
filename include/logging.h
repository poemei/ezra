#ifndef EZRA_LOGGING_H
#define EZRA_LOGGING_H

#define EZRA_INSTALL_PATH "/opt/ezra"
#define EZRA_LOG_DIRECTORY "./logs"
#define EZRA_LOG_PATH "./logs/ezra.log"
#define EZRA_MODULE_LOG_PATH "./logs/modules.log"

int ezra_log_open(void);
int ezra_log_open_path(const char *absolute_path); /* tests only */
int ezra_log_event(const char *component, const char *event, const char *detail);
int ezra_log_sync(void);
void ezra_log_close(void);
int ezra_log_is_open(void);

#endif
