#include <stdio.h>
#include <curl/curl.h>
#include <string.h>

/* Compile with:
gcc api_commands.c -lcurl

Example found here, in the file called simple.c:
http://curl.haxx.se/libcurl/c/example.html
*/

char *serverAddress = "http://sindrus.net/cloud/slave/new?api_key=F237E8FB2657FFFE5878AC972CA67";

int main(void)
{
  CURL *curl;
  CURLcode res;
 
  curl = curl_easy_init();
  if(curl) {
    /* Set properties */
    curl_easy_setopt(curl, CURLOPT_URL, serverAddress);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    /* Fetch data from api */
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
  return 0;
}