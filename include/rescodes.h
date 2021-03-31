#ifndef __RESCODES_
#define __RESCODES_ 

struct rescode {
	int code;
	char *desc;
	int (*handle)(struct collection *col);
};

const struct rescode errcodes[] = {
	{PWDMAN_ERROR            , "Operation Failed\n", NULL},
	{PWDMAN_ERRINVALIDID     , "Invalid Id\n", NULL},
	{PWDMAN_ERRINVALIDEMAIL  , "Invalid Email\n", NULL},
	{PWDMAN_ERRINVALIDSITE   , "Invalid Site\n", NULL},
	{PWDMAN_ERRINVALIDPARAMS , "Invlalid Parameters\n", NULL},
	{PWDMAN_ERRIDNOTFOUND    , "Id not found\n", NULL},
	{PWDMAN_ERRSITENOTFOUND  , "Site not found\n", NULL},
	{PWDMAN_ERREMAILNOTFOUND , "Email not found\n", NULL},
	{0                       , NULL }
};

const struct rescode rescodes[] = {
	{PWDMAN_SUCCESSADD    , "Added Successfully\n", NULL},
	{PWDMAN_SUCCESSUPDATE , "Updated Successfully\n", NULL},
	{PWDMAN_SUCCESSDELETE , "Deleted Successfully\n", NULL},
	{PWDMAN_SUCCESSFIND   , NULL, },
	{0                    , NULL                  }
};


#endif