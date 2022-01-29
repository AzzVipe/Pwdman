#include <stdio.h>
#include <string.h>

#include <command.h>
#include <str.h>
#include <pwdman_request.h>
#include <pwdman_response.h>
#include <validator.h>
 
#define ADD_CMD_ARGS    5
#define DEL_CMD_ARGS    3
#define UPDATE_CMD_ARGS 6
#define PRINT_CMD_ARGS  2
#define FIND_CMD_ARGS   4

#define FIND_TYPES 3

const struct command commands[] = {
	{"add"    , "/add"    , command_handle_add    , pwdman_request_handle_add    , pwdman_response_handle_cud },
	{"delete" , "/del"    , command_handle_del    , pwdman_request_handle_del    , pwdman_response_handle_cud },
	{"update" , "/update" , command_handle_update , pwdman_request_handle_update , pwdman_response_handle_cud },
	{"find"   , "/find"   , command_handle_find   , pwdman_request_handle_find   , pwdman_response_handle_print },
	{"print"  , "/print"  , command_handle_print  , pwdman_request_handle_print  , pwdman_response_handle_print },
	{NULL     , NULL      , NULL                  , NULL                         , NULL }
};

const char finds[FIND_TYPES][32] = {
	"id",
	"site", 
	"email"
};

static bool command_is_valid(const char *command);
static int command_validate_find_value(char *value, int mode);

int command_handle(struct request *req, const char **argv, int argc, char *buf)
{
	int index, buflen;
	char req_buf[BUFFSIZE];

	if (argc < 2 ) {
		fprintf(stderr, "Usage : ./client <command> <arguments>\n");
		return -1;
	}

	// check the uri and invoke the corresponding cmd handler
	if ((index = command_get_index(argv[1], BY_CMD)) == -1) {
		fprintf(stderr, "Invaild Command : %s\n", argv[1]);
		return -1;
	}
	request_type_set(req, REQ_TYPE_GET);
	request_header_set(req, "content-len", "10");
	request_uri_set(req, commands[index].uri);

	// call handler
	if(commands[index].cmd_handle(req, argv, argc) == -1)
		return -1;

	return 0;

}

int command_handle_add(struct request *req, const char **argv, int argc)
{
	int err_count = 0;

	if (argc != ADD_CMD_ARGS) {
		fprintf(stderr, "Invaild Arguments \nUsage : ./client add <site> <email> <password>\n");
		return -1;
	}

	if (req == NULL)
		return -1;

	char *site, *email, *password;

	site = strdup(argv[2]);

	if (!validate_site(site)) {
		fprintf(stderr, "Invaild Site \n");
		err_count++;
	}

	email = strdup(argv[3]);

	if (!validate_email(email)) {
		fprintf(stderr, "Invaild Email \n");
		err_count++;
	}

	password = strdup(argv[4]);

	// if (!validate_password(password)) {
		
	// }
	if (err_count != 0) 
		goto cleanup;

	request_param_set(req, "site", site);
	request_param_set(req, "email", email);
	request_param_set(req, "password", password);

	return 0;

cleanup:

	free(site);
	free(email);
	free(password);

	return -1;
}

int command_handle_del(struct request *req, const char **argv, int argc)
{
	if (argc != DEL_CMD_ARGS) {
		fprintf(stderr, "Invaild Parameters\n");
		return -1;
	}

	if (req == NULL) 
		return -1;

	char *id;

	id = strdup(argv[2]);

	if (!validate_number(id)) {
		fprintf(stderr, "Invaild ID\n");
		free(id);

		return -1;
	}

	request_param_set(req, "id", id);

	return 0;
}


int command_handle_update(struct request *req, const char **argv, int argc)
{
	int err_count = 0;

	if (argc != UPDATE_CMD_ARGS) {
		fprintf(stderr, "Invaild Arguments \n");
		return -1;
	}

	if (req == NULL)
		return -1;

	char *id, *site, *email, *password;

	id = strdup(argv[2]);
	puts(id);

	if (!validate_number(id)) {
		fprintf(stderr, "Invaild ID \n");
		err_count++;
	}

	site = strdup(argv[3]);

	if (!validate_site(site)) {
		fprintf(stderr, "Invaild Site \n");
		err_count++;
	}

	email = strdup(argv[4]);

	if (!validate_email(email)) {
		fprintf(stderr, "Invaild Email \n");
		err_count++;
	}

	password = strdup(argv[5]);

	// if (!validate_password(email)) {
	// 	fprintf(stderr, "Invaild Password \n");
	// 	err_count++;
	// }

	if (err_count != 0) 
		goto cleanup;

	request_param_set(req, "id", id);
	request_param_set(req, "site", site);
	request_param_set(req, "email", email);
	request_param_set(req, "password", password);

	return 0;

cleanup:

	free(id);
	free(site);
	free(email);
	free(password);

	return -1;
}

int command_handle_find(struct request *req, const char **argv, int argc)
{
	// TODO
	if (argc != FIND_CMD_ARGS) {
		fprintf(stderr, "Invaild Arguments \n");
		return -1;
	}

	if (req == NULL)
		return -1;

	char *param = strdup(argv[2]);
	char *value = strdup(argv[3]);

	for (int i = 0; i < FIND_TYPES; ++i)
	{
		if (strcmp(param, finds[i]) == 0) {
			if (command_validate_find_value(value, i) == -1) {
				fprintf(stderr, "Invaild Parameters\n");
				return -1;
			}
			request_param_set(req, "type", param);	
			request_param_set(req, param, value);	

			return 0;
		}
	}

	return -1;

}

int command_handle_print(struct request *req, const char **argv, int argc)
{
	if (argc == PRINT_CMD_ARGS && req != NULL)
		return 0;
	else
		return -1;
}

int command_handle_init(struct request *req, const char **argv, int argc)
{
	return 0;
}

int command_get_index(const char *cmd, int flags)
{
	switch(flags)
	{
		case BY_CMD :
			for (int i = 0; commands[i].cmd; ++i) {
				if (strcmp(cmd, commands[i].cmd) == 0)
					return i;
			}

			return -1;

		case BY_URI :
			for (int i = 0; commands[i].uri; ++i) {
				if (strcmp(cmd, commands[i].uri) == 0)
					return i;
			}

			return -1;

		default :

			return -1;
	}


}

static bool command_is_valid(const char *command)
{
	char *cmd_cp, *cmd_cpp;
	if ((cmd_cp = strdup(command)) == NULL) {
		fprintf(stderr, "[!] command error\n");
		return false;
	}
	str_ltrim(cmd_cp);
	cmd_cpp = cmd_cp;

	if (command[0] != '/') {
		fprintf(stderr, "[!] unknown command\n");
		goto cleanup;
	}

	for (int i = 0; commands[i].uri; ++i) {
		if (strcmp(command, commands[i].uri) != 0)
			goto cleanup;
	}

	return true;

cleanup:
	free(cmd_cp);
	free(cmd_cpp);

	return false;
}

static int command_validate_find_value(char *value, int mode)
{
	switch (mode)
	{
		case 0 :
			if (!validate_number(value))
				return -1;
				break;

		case 1 :
			if (!validate_site(value))
				return -1;
				break;

		case 2 :
			if (!validate_email(value))
				return -1;
				break;

		return 0;
	}
}