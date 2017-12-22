#include "main.h"
#include "secret.h"

#include "oauth.h"

void tweet_test(void) {
	const char *test_call_uri = "https://api.twitter.com/1.1/statuses/update.json";
	
	const char *owner = "pocsagbot";
	const char *text = "OAuth test";

	char *req_url = NULL;
	char *postarg = NULL;

	printf("Tweet test...\n");

	int argc = 2;
	char argv_wark[512];
	char **argv = (char**) malloc(sizeof(char*) * 2);
	argv[0] = strdup(test_call_uri);
	snprintf(argv_wark, sizeof(argv_wark), "status=%s", text);
	argv[1] = strdup(argv_wark);
	req_url = oauth_sign_array2(&argc, &argv, &postarg, OA_HMAC, NULL,
		consumer_key, consumer_secret, access_token, access_token_secret);
	free(argv[1]);
	free(argv[0]);
	free(argv);
	char* res = oauth_http_post2(req_url, postarg, "Expect: \r\n");
	
	if (res != NULL)
		puts(res);
	else
		printf("Null :(\n");
}
