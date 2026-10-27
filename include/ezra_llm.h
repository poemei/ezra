#ifndef EZRA_LLM_H
#define EZRA_LLM_H

#define EZRA_LLM_GENERATE_SERVICE "digit.llm.generate"

#define EZRA_LLM_PROMPT_MAX 2048
#define EZRA_LLM_RESPONSE_MAX 512

typedef struct {
    char prompt[EZRA_LLM_PROMPT_MAX];
} ezra_llm_request_t;

typedef struct {
    char text[EZRA_LLM_RESPONSE_MAX];
} ezra_llm_response_t;

#endif
