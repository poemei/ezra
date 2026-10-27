#ifndef EZRA_OLLAMA_INTERNAL_H
#define EZRA_OLLAMA_INTERNAL_H

#include <stddef.h>

#define EZRA_OLLAMA_ENDPOINT "http://127.0.0.1:11434/api/generate"
#define EZRA_OLLAMA_MODEL "llama3.2:1b"
#define EZRA_OLLAMA_HTTP_MAX (256U * 1024U)

int ezra_ollama_build_request(const char *prompt,
                                char *output, size_t output_size);
int ezra_ollama_extract_response(const char *json,
                                   char *output, size_t output_size);
int ezra_ollama_run_selftests(unsigned int *executed,
                                unsigned int *passed,
                                int *negative_executed,
                                int *negative_passed);

#endif
