#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include "jsmn.h"

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
        /* out of memory! */ 
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

char *substring(char *string, int position, int length) 
{
   char *pointer;
   int c;
 
   pointer = malloc(length+1);
 
   if (pointer == NULL)
   {
      printf("Unable to allocate memory.\n");
      exit(EXIT_FAILURE);
   }
 
   for (c = 0 ; c < position -1 ; c++) 
      string++; 
 
   for (c = 0 ; c < length ; c++)
   {
      *(pointer+c) = *string;      
      string++;   
   }
 
   *(pointer+c) = '\0';
 
   return pointer;
}

void initCurl(int *graph, int *graphSize, long *graphId)
{
    struct MemoryStruct chunk;
    
    chunk.memory = malloc(1);
    chunk.size = 0;

	CURL *curl_handle;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);

    curl_handle = curl_easy_init();
    
    curl_easy_setopt(curl_handle, CURLOPT_URL, "http://sindrus.net/cloud/slave/new?api_key=F237E8FB2657FFFE5878AC972CA67");
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
 
    res = curl_easy_perform(curl_handle);

    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    jsmn_parser parser;
    jsmn_init(&parser);
    
    unsigned int n = chunk.size;
    jsmntok_t *tokens = malloc(20);
    int ret = jsmn_parse(&parser, chunk.memory, chunk.size, tokens, n);

    typedef enum { START, KEY, GRAPH, SIZE, ID, TABOO, SKIP, STOP } parse_state;
    parse_state state = START;

    size_t object_tokens = 0;

    for (size_t i = 0, j = 1; j > 0; i++, j--)
    {
        jsmntok_t *t = &tokens[i];

        if (t-> type == JSMN_ARRAY || t->type == JSMN_OBJECT)
            j += t->size;

        switch (state)
        {
            case START:
                if (t->type != JSMN_OBJECT)
                    printf("%s\n","Root element must be an object.");

                state = KEY;
                object_tokens = t->size;

                if (object_tokens == 0)
                    state = STOP;

                if (object_tokens % 2 != 0)
                    printf("%s\n","Objects must have even number of children.");

                break;

            case KEY:
                object_tokens--;

                if(t->type != JSMN_STRING)
                    printf("%s\n","Objects keys must be strings.");

                state = SKIP;

                char *pointer = substring(chunk.memory, t->start+1, t->end - t->start);

                if (strcmp(pointer, "graph") == 0)
                    state = GRAPH;

                if (strcmp(pointer, "matrix_size") == 0)
                    state = SIZE;

                if (strcmp(pointer, "graph_id") == 0)
                    state = ID;

                if (strcmp(pointer, "taboos") == 0)
                    state = TABOO;

                free(pointer);

                break;

            case GRAPH:
                if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE)
                    printf("%s\n","Case graph - Objects values must be strings or primitive");

                /* TODO: How to copy char array temp_graph to integer graph. */
                char *temp_graph = substring(chunk.memory, t->start+1, t->end - t->start);
                sscanf(temp_graph, "%d", graph);
                free(temp_graph);
                    
                object_tokens--;
                state = KEY;
                    
                if (object_tokens == 0)
                    state = STOP;

                break;

            case SIZE:
                if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE)
                    printf("%s\n","Case size - Objects values must be strings or primitive");

                char *temp_size = substring(chunk.memory, t->start+1, t->end - t->start);
                sscanf(temp_size, "%d", graphSize);

                object_tokens--;
                state = KEY;
                    
                if (object_tokens == 0)
                    state = STOP;

                break;

            case ID:
                if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE)
                    printf("%s\n","Case id - Objects values must be strings or primitive");

                char *temp_id = substring(chunk.memory, t->start+1, t->end - t->start);
                sscanf(temp_id, "%ld", graphId);

                object_tokens--;
                state = KEY;
                    
                if (object_tokens == 0)
                    state = STOP;
                
                break;

            case TABOO:
                if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE)
                    printf("%s\n","Case taboo - Objects values must be strings or primitive");

                /* Need to do something here */
                    
                object_tokens--;
                state = KEY;
                    
                if (object_tokens == 0)
                    state = STOP;

                break;

            case SKIP:
                if (t->type != JSMN_STRING && t->type != JSMN_PRIMITIVE)
                    printf("%s\n","Case skip - Objects values must be string or primitives.");
                object_tokens --;
                state = KEY;

                if (object_tokens == 0)
                    state = STOP;

                break;

            case STOP:
                break;

            default:
                printf("Invalid state %u", state);
        }
    }

    if(chunk.memory)
        free(chunk.memory);
}

int main()
{
	int graph;
	int graphSize;
	long graphId;

	initCurl(&graph, &graphSize, &graphId);
    
    printf("%d\n", graph);
    printf("%d\n", graphSize);
    printf("%ld\n", graphId);

	return 0;
}