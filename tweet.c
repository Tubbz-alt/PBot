#include "main.h"
#include "secret.h"

#include "oauth.h"

void tweet(const char *text) {
	int argc = 2;
	char argv_wark[512];
	char *req_url = NULL;
	char *postarg = NULL;
	
	const char *test_call_uri = "https://api.twitter.com/1.1/statuses/update.json";

	printf("Tweeting... ");
	
	char **argv = (char**)malloc(sizeof(char*) * 2);
	
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
		if (strstr(res, "error") == NULL)
			printf("Success :D\n");
		else
			printf("Error :(\n");
	else
		printf("Failed :(\n");
}


void send_dm(const char *text) {
	int argc = 3;
	char argv_wark[512];
	char *req_url = NULL;
	char *postarg = NULL;
	
	const char *test_call_uri = "https://api.twitter.com/1.1/direct_messages/new.json";

	printf("Sending DM... ");
	
	char **argv = (char**)malloc(sizeof(char*) * 3);
	
	argv[0] = strdup(test_call_uri);
	snprintf(argv_wark, sizeof(argv_wark), "text=%s", text);
	argv[1] = strdup(argv_wark);
	snprintf(argv_wark, sizeof(argv_wark), "screen_name=furrtek", text);
	argv[2] = strdup(argv_wark);
	
	req_url = oauth_sign_array2(&argc, &argv, &postarg, OA_HMAC, NULL,
		consumer_key, consumer_secret, access_token, access_token_secret);
	
	free(argv[2]);
	free(argv[1]);
	free(argv[0]);
	free(argv);
	
	char* res = oauth_http_post2(req_url, postarg, "Expect: \r\n");
	
	if (res != NULL)
		if (strstr(res, "error") == NULL)
			printf("Success :D\n");
		else
			printf("Error :(\n");
	else
		printf("Failed :(\n");
}
