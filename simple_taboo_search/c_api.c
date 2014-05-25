#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <curl/curl.h>
#include "jsmn.h"

#define JSON_TOKENS 256
#define UTC (0)

struct JSONDataStruct{
    char *dataString;
    size_t size;
};
struct GraphStruct{
    int *graphArray;
    size_t size;
};

const char *HOST = "http://sindrus.net/cloud/slave/new?api_key=F237E8FB2657FFFE5878AC972CA67";

typedef enum { START, KEY, GRAPH, SIZE, ID, TABOO, SKIP, STOP } parse_state;

static size_t WriteMemoryCallback( void *contents, size_t size, size_t nmemb, void *userp );
void fetch_new_graph( int **graph, int *graphSize, long *graphId );



static size_t WriteMemoryCallback( void *contents, size_t size, size_t nmemb, void *userp ){
    size_t realsize = size * nmemb;
    struct JSONDataStruct *json = (struct JSONDataStruct *)userp;

    json->dataString = realloc( json->dataString, json->size + realsize + 1 );
    if( json->dataString == NULL ){
        /* Out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }
    memcpy( &( json->dataString[ json->size ] ), contents, realsize );
    json->size += realsize;
    json->dataString[ json->size ] = 0;

    return realsize;
}

char *substring( char *string, int position, int length ){
    char *pointer = malloc( length + 1 );
    if ( pointer == NULL ){
        printf( "Unable to allocate memory.\n" );
        exit( EXIT_FAILURE );
    }
    int c;
    for( c = 0; c < position - 1; c++ ){
        string++;
    }
    for( c = 0; c < length ; c++ ){
        *(pointer+c) = *string;
        string++;
    }
    *(pointer+c) = '\0';
    return pointer;
}

/**
 *
 * These are the functions to connect to the master,
 * get a new graph object as a json object, parse the json
 * and send the graph to the algorithm.
 * 
 * void fetch_data( struct JSONDataStruct *json);
 * void *json_tokenizer( char *js, size_t size );
 * parse_state get_next_state( char *instr );
 * void convert_graph( int **graph, char *source, size_t size );
 * void fetch_new_graph( int **graph, int *graphSize, long *graphId );
 *
**/
void fetch_data( struct JSONDataStruct *json){
    
    CURL *curl_handle;
    CURLcode res;
    curl_global_init( CURL_GLOBAL_ALL );
    curl_handle = curl_easy_init();
    curl_easy_setopt( curl_handle, CURLOPT_URL, HOST );
    curl_easy_setopt( curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback );
    curl_easy_setopt( curl_handle, CURLOPT_WRITEDATA, (void *)json );
    curl_easy_setopt( curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    res = curl_easy_perform( curl_handle );
    if ( res != CURLE_OK ){
        fprintf( stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror( res ) );
    }
    curl_easy_cleanup( curl_handle );
    curl_global_cleanup();
}

void *json_tokenizer( char *js, size_t size ){
    jsmn_parser parser;
    jsmn_init( &parser );
    unsigned int n = JSON_TOKENS;
    jsmntok_t *tokens = malloc( sizeof( jsmntok_t ) * n );

    int ret = jsmn_parse( &parser, js, size, tokens, n );

    while ( ret == JSMN_ERROR_NOMEM ){
        n = n * 2 + 1;
        tokens = realloc( tokens, sizeof( jsmntok_t ) * n );
        ret = jsmn_parse( &parser, js, size, tokens, n );
    }
    return tokens;
}

parse_state get_next_state( char *instr ){
    if( strcmp( instr, "graph" ) == 0 )
        return GRAPH;
    else if( strcmp( instr, "matrix_size" ) == 0 )
        return SIZE;
    else if( strcmp( instr, "graph_id" ) == 0 )
        return ID;
    else if( strcmp( instr, "taboos" ) == 0 )
        return TABOO;
    return SKIP;
}

void convert_graph( int **graph, char *source, size_t size ){

    (*graph) = realloc( (*graph), size*sizeof( int ) );
    for( int i = 0; i < (int) size ; i++ ){
        (*graph)[ i ] = source[ i ] - '0';
    }
}

void fetch_new_graph( int **graph, int *graphSize, long *graphId ){
    struct JSONDataStruct json;
    json.dataString = malloc( 1 );
    json.size = 0;
    fetch_data( &json );
    jsmntok_t *tokens = json_tokenizer( json.dataString, json.size );
    parse_state state = START;
    size_t object_tokens = 0;
    char *strtolDrain;

    for( size_t i=0, j=1; j>0; i++, j-- ){
        jsmntok_t *t = &tokens[ i ];
        if (t->type == JSMN_ARRAY || t->type == JSMN_OBJECT )
            j += t->size;

        switch (state){
            case START:

                if( t->type != JSMN_OBJECT )
                    printf( "%s\n", "Root element must be an object." );
                
                state = KEY;
                object_tokens = t->size;
                
                if( object_tokens == 0 )
                    state = STOP;
                
                if ( object_tokens % 2 != 0)
                    printf( "%s\n", "Objects must have even number of children." );
                break;

            case KEY:
                object_tokens--;
                
                if( t->type != JSMN_STRING )
                    printf( "%s\n", "Objects keys must be strings." );
                
                state = get_next_state( substring( 
                        json.dataString, t->start+1, t->end - t->start ) );
                break;

            case GRAPH:
                object_tokens--;
                state = KEY;
                char *gg = substring( json.dataString, t->start+1, t->end - t->start );
                convert_graph( graph, gg, t->end - t->start );
                free( gg );
                if( object_tokens == 0 )
                    state = STOP;
                break;

            case SIZE:
                object_tokens--;
                state = KEY;

                *graphSize = strtol( substring( 
                        json.dataString, t->start+1, t->end - t->start ), &strtolDrain, 10 );

                if( object_tokens == 0 )
                    state = STOP;
                break;

            case ID:
                object_tokens--;
                state = KEY;
                
                *graphId = strtol( substring( 
                        json.dataString, t->start+1, t->end - t->start ), &strtolDrain, 10 );

                if( object_tokens == 0 )
                    state = STOP;
                break;

            case TABOO:
                object_tokens--;
                state = KEY;
                
                if( object_tokens == 0 )
                    state = STOP;
                break;

            case SKIP:
                object_tokens--;
                state = KEY;
                
                if( object_tokens == 0 )
                    state = STOP;
                break;

            case STOP:
                break;

            default:
                break;
        }
    }
    free( json.dataString );
}

/**
 *
 * functions that save a graph to the master
 * void save_graph( long graphId, int graphSize, int *graph, int isSolution )
 * 
**/

void graph_to_string( char *strGraph, int *graph, int size ){
    for( int i = 0; i < size*size; i++ )
        strGraph[i] = graph[i]+'0';
}

void submit( char *data ){
    printf( "submitting\n" );
    CURL *curl;
    CURLcode res;
        
    curl_global_init(CURL_GLOBAL_ALL);
             
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://sindrus.net/cloud/slave/save");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
                                                      
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
                                                                                 
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    printf( "done submitting\n" );
}

void save_graph( long graphId, 
                 int graphSize, 
                 int *graph, 
                 int isSolution ){
    
    printf( "\nEnd graph\n" );

    int jsonlength = 160+( graphSize*graphSize );
    int inProgress = isSolution; 
    char *jsonStr = malloc( jsonlength );
    char *graphStr = malloc( graphSize*graphSize );
    graph_to_string( graphStr, graph, graphSize );
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);
    printf( "saving\n" );

    //printf( "graph:\n%s\n", graphStr );

    sprintf( jsonStr, "{ \"graph\" : \"%s\", "
                      "\"in_progress\" : %d, "
                      "\"matrix_size\" : %d, "
                      "\"graph_id\" : %ld, "
                      "\"taboos\" : [], "
                      "\"is_solution\" : %d, "
                      "\"last_updated\" : \"%d-%d-%d %d:%d:%d\" }",
                      graphStr, inProgress, graphSize, graphId, isSolution,
                      tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
                      tm.tm_min, tm.tm_sec );
    printf( "json: \n%s\n", jsonStr );
    submit( jsonStr );
    free( jsonStr );
    free( graphStr );
}

/**
 *
 * Functions to test and debug the library
 *
**/
/*void new_graph(){
    int *graph = malloc( 1*sizeof( int ) );
    int graphSize;
    long graphId;

    fetch_new_graph( &graph, &graphSize, &graphId );
}

int main(){
//    new_graph();
    long graphId = 2333601399676259;
    int graphSize = 2;
    int graph[ 4 ] = { 1, 1, 0, 1 };
    int isSolution = 0;
    save_graph( graphId, graphSize, graph, isSolution );
    return 0;
}*/
