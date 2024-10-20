#include <curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <float.h>

enum MallocPlusAIError {
	MALLOC_AI_SUCCESS,
	MALLOC_AI_ALREADY_INIT,
	MALLOC_AI_CURL_INIT_FAILED,

	MALLOC_AI_NOT_INIT,
};

struct _MallocPlusAI {
	CURL *curl;
	char *address;
	char *model_name;
	bool verbose;
	char *bearer_auth;
	float temperature;
};

struct _MallocPlusAI *INST = NULL;

// Requires a server with no authentication and an OpenAI-like API.
enum MallocPlusAIError initMallocPlusAI (const char *server_address, const char *model_name) {
	if (INST) {
		return MALLOC_AI_ALREADY_INIT;
	}

	// Unfortunately, the power of AI cannot be harnessed as of this allocation.
	struct _MallocPlusAI *inst = malloc(sizeof(struct _MallocPlusAI));

	char *addr = malloc(strlen(server_address));
	memcpy(addr, server_address, strlen(server_address));

	char *mod_name = malloc(strlen(model_name));
	memcpy(mod_name, model_name, strlen(model_name));

	inst->address = addr;
	inst->model_name = mod_name;
	inst->verbose = false;
	inst->bearer_auth = NULL;
	inst->temperature = 0.01f;

	inst->curl = curl_easy_init();
	if (!inst->curl) {
		return MALLOC_AI_CURL_INIT_FAILED;
	}

	INST = inst;
	return MALLOC_AI_SUCCESS;
}

void freeMallocPlusAI (void) {
	if (!INST) {
		return;
	}
	curl_easy_cleanup(INST->curl);
	free(INST->address);
	free(INST->model_name);
	free(INST->bearer_auth);

	INST = NULL;
}

enum MallocPlusAIError setMallocPlusAIVerbose (bool verbose) {
	if (!INST) {
		return MALLOC_AI_NOT_INIT;
	}
	INST->verbose = verbose;
	if (verbose) {
		puts("mallocWithAI: verbose = true");
	}
	return MALLOC_AI_SUCCESS;
}

enum MallocPlusAIError setMallocPlusAITemperature (float temperature) {
	if (!INST) {
		return MALLOC_AI_NOT_INIT;
	}
	INST->temperature = temperature;
	return MALLOC_AI_SUCCESS;
}

enum MallocPlusAIError setMallocPlusAIBearerAuth (const char *token) {
	if (!INST) {
		return MALLOC_AI_NOT_INIT;
	}
	if (INST->bearer_auth) {
		free(INST->bearer_auth);
	}

	char *tok = malloc(strlen(token));
	memcpy(tok, token, strlen(token));

	INST->bearer_auth = tok;
	return MALLOC_AI_SUCCESS;
}

struct _CurlReader {
	char *text;
	size_t length;
};

size_t CurlRead (void *data, size_t size, size_t count, void *userdata) {
	struct _CurlReader *buf = (struct _CurlReader*) userdata; // convert userdata back to our own struct
	const size_t added_bytes = size * count; // curl gives us the sizeof a character and the number of characters
	buf->text = realloc(buf->text, buf->length + added_bytes + 1); // +1 for null byte
memcpy(&(buf->text[buf->length]), data, added_bytes); // copy text from curl into our buffer, leaving an allocated byte at the end
	buf->length += added_bytes;
	buf->text[buf->length] = 0; // make sure null byte is sustained at the end of the buffer for every callback call
	return added_bytes;
}

void* mallocPlusAI (const char *request) {
	if (!INST) {
		// initMallocPlusAI needs to be called beforehand
		return NULL;
	}
	struct _CurlReader out = {0};
	out.text = malloc(1);
	*out.text = 0;
	out.length = 0;

	char url[64] = { 0 };
	snprintf(url, 64, "%s/v1/chat/completions", INST->address);
	curl_easy_setopt(INST->curl, CURLOPT_URL, &url);
	curl_easy_setopt(INST->curl, CURLOPT_WRITEFUNCTION, &CurlRead);
	curl_easy_setopt(INST->curl, CURLOPT_WRITEDATA, &out);

	struct curl_slist *hs=NULL;
	hs = curl_slist_append(hs, "Content-Type: application/json");
	if (INST->bearer_auth) {
		char bearer[300] = { 0 };
		snprintf(bearer, 300, "Authorization: Bearer %s", INST->bearer_auth); 	
		hs = curl_slist_append(hs, bearer);
	}
	curl_easy_setopt(INST->curl, CURLOPT_HTTPHEADER, hs);

	char json[512] = { 0 };
	snprintf(json, 512, "{\"temperature\":%.3f,\"model\":\"%s\",\"messages\":[{\"role\":\"system\",\"content\":\"You are a memory allocator, and you need to tell me how many bytes of memory I would need to accomplish a certain task. ONLY respond with the whole number of bytes, DO NOT provide ANY other text\"},{\"role\":\"user\",\"content\":\"%s\"}]}", INST->temperature, INST->model_name, request);
	curl_easy_setopt(INST->curl, CURLOPT_POSTFIELDS, &json);
	curl_easy_perform(INST->curl);

	if (INST->verbose) {
		printf("mallocPlusAI (raw response): %s\n", out.text);
	}
	
	const char *needle = "\"content\": \"";
	char *content = strstr(out.text, needle);
	if (!content) {
		return NULL;
	}

	char* begin_resp = content + sizeof(needle) + 4; // no idea why i need to add an extra 4 here

	size_t resp_len = 0;
	while (isdigit(begin_resp[resp_len])) {
		++resp_len;
	}
	begin_resp[resp_len] = 0;

	int num_bytes = atoi(begin_resp);

	if (INST->verbose) {
		printf("mallocPlusAI: malloc(%d)\n", num_bytes); 
	}
	free(out.text);
	return malloc(num_bytes);
}




