#ifndef __REQUEST_H
#define __REQUEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#define REQ_MAX_PARAM_LEN 	10
#define REQ_TYPE_GET		 1
#define REQ_TYPE_POST		 2
#define REQ_TYPE_PATCH		 3
#define REQ_TYPE_DELETE		 4
#define REQ_MAX_URI_LEN		256

#define REQ_HEADER_DELIM	':'
#define REQ_PARAM_DELIM		'='


struct request {
	int   	type;
	char 	*uri;
	char 	*header_keys[REQ_MAX_PARAM_LEN];
	char 	*header_values[REQ_MAX_PARAM_LEN];

	char 	*param_keys[REQ_MAX_PARAM_LEN];
	char 	*param_values[REQ_MAX_PARAM_LEN];
	char 	*body;
	size_t 	bodylen;
	int		iparam;
	int		iheader;
};

const char *request_type_str_get(struct request *req);

int request_type_get(struct request *req);
int request_type_set(struct request *req, int req_type);

bool request_parse(const char *reqbuf, size_t rbdatalen, struct request *req);
void request_header_set(struct request *req, const char *key, const char *value);
void request_param_set(struct request *req, const char *key, const char *value);
char *request_uri_get(struct request *req);
void request_uri_set(struct request *req, const char *uri);
size_t request_header_size(struct request *req);
size_t request_param_size(struct request *req);

/**
 * Returns value of parameter of the request body key in the given
 * request req
 *
 * @return char*  param value of the given key if key found otherwise NULL
 */
char *request_param_get(struct request *req, const char *key);

/**
 * Finds and returns the value of the header key present in the 
 * request req
 *
 * @return char*  header value of the given key if key found otherwise NULL
 */
char *request_header_get(struct request *req, const char *key);

/**
 *
 *
 */
ssize_t request_prepare(struct request *req, char *buf, ssize_t buflen);

/**
 * Dumps the request req in json format. Helpful in debuging
 *
 */
void request_dump(struct request *req);

#endif
