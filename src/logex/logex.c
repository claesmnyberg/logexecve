/*
 * Copyright (C) 2004-2021 Claes M Nyberg
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Claes M Nyberg.
 * 4. The name Claes M Nyberg may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/param.h>
#include <err.h>
#include <pwd.h>
#include <sys/syscall.h>

#include "../sys/kern/logexecve/logexecve.h"

/* Default kernel, OpenBSD 3.5 */
#define LOGEXECVE_SYSCALL_NUM 331

int
get_logopts(struct log_options *lopts)
{
	int opt = LOGOPT_GETOPT;
	if (syscall(LOGEXECVE_SYSCALL_NUM, opt, lopts, sizeof(struct log_options)) < 0) {
		perror("syscall:");
		return(-1);
	}
	return(0);
}

int
set_logopts(struct log_options *lopts)
{
    int opt = LOGOPT_SETOPT;
    if (syscall(LOGEXECVE_SYSCALL_NUM, opt, lopts, sizeof(struct log_options)) < 0) {
        perror("syscall:");
        return(-1);
    }
    return(0);
}


int
print_logopts(struct log_options *lopts, int uname)
{
	struct passwd *pwd;

	printf("   on: %s\n", lopts->flags & LOG_OFF ? "no" : "yes");
	printf("  env: %s\n", lopts->flags & LOG_ENVP ? "yes" : "no");
	printf(" euid: %s\n", lopts->flags & LOG_EUID ? "yes" : "no");
	printf(" suid: %s\n", lopts->flags & LOG_SUID ? "yes" : "no");
	printf(" egid: %s\n", lopts->flags & LOG_EGID ? "yes" : "no");
	printf(" sgid: %s\n", lopts->flags & LOG_SGID ? "yes" : "no");
	printf(" tste: %s\n", lopts->flags & LOG_TSTE ? "yes" : "no");

	if (lopts->uids_num) {
		int i;

		printf("users: ");
		for (i=0; i<lopts->uids_num; i++) {
			printf("%u", (u_int)lopts->uids[i]);
			
			if (uname != 0) {
				if ( (pwd = getpwuid((uid_t)lopts->uids[i])) != NULL)
					printf("(%s)", pwd->pw_name);
			}
			printf(" ");
		}
		printf("\n");
	}
	else
		printf(" ** No users set to log!\n");
	return(0);
}


/*     
 * Check if string is a numeric value.
 * Zero is returned if string contains base-illegal
 * characters, 1 otherwise. 
 * Binary value should have the prefix '0b'
 * Octal value should have the prefix '0'.
 * Hexadecimal value should have the prefix '0x'.
 * A string with any digit as prefix except '0' is
 * interpreted as decimal.
 * If val in not NULL, the converted value is stored
 * at that address.
 */ 
int 
strisnum(u_char *str, u_long *val)
{
    int base = 0;
    char *endpt;
 
    if (str == NULL)
        return(0);

    while (isspace((int)*str))
        str++;

    /* Binary value */
    if (!strncmp(str, "0b", 2) || !strncmp(str, "0B", 2)) {
        str += 2;
        base = 2;
    }

    if (*str == '\0')
        return(0);

    if (val == NULL)
        strtoul(str, &endpt, base);
    else
        *val = strtoul(str, &endpt, base); 
 
    return((int)*endpt == '\0');
}


/*
 * Parse the list of users.
 * Returns -1 on error, 0 otherwise.
 */
int
get_uids(char *str, u_long *list, size_t *savenum, size_t list_len)
{
	char *ep;
	char *bp;

	bp = str;
	*savenum = 0;
	while (bp != NULL) {

		if ( (ep = strchr(bp, ',')) != NULL)
			*ep = '\0';
	
		if (*savenum >= list_len) {
			fprintf(stderr, "** Error: To many UID for "
				"list length (%lu)\n", *savenum -1);
			return(-1);
		}
	
		if (!strisnum(bp, &list[*savenum])) {
			struct passwd *pwd;

			if ( (pwd = getpwnam(bp)) == NULL) {
				fprintf(stderr, "** Error: Can not resolve '%s' as UID  "
					"or user name\n", bp);
				return(-1);
			}
			list[*savenum] = pwd->pw_uid;
		}

		(*savenum)++;
		if (ep != NULL)
			*ep++ = ',';
		bp = ep;
		
		/* List ends with ',' */
		if ((bp != NULL) && *bp == '\0')
			break;
	}

	return(0);
}

/*
 * Parse the list of options.
 * Returns -1 on error, 0 otherwise.
 */
int
get_flags(char *str, u_int *flags)
{
    char *ep;
    char *bp;

	*flags = 0;
    bp = str;
    while (bp != NULL) {

        if ( (ep = strchr(bp, ',')) != NULL)
            *ep = '\0';
		
		while ((*bp != '\0') && isspace((int)*bp))
			bp++;

		if (!strncasecmp(bp, "env", 3))
			*flags |= LOG_ENVP;
		else if (!strncasecmp(bp, "euid", 4)) 
			*flags |= LOG_EUID;
		else if (!strncasecmp(bp, "suid", 4)) 
			*flags |= LOG_SUID;
		else if (!strncasecmp(bp, "egid", 4)) 
			*flags |= LOG_EGID;
		else if (!strncasecmp(bp, "sgid", 4)) 
			*flags |= LOG_SGID;
		else if (!strncasecmp(bp, "tste", 4)) 
			*flags |= LOG_TSTE;
#ifdef ENABLE_FILEHIDE
		else if (!strncasecmp(bp, "hide", 4)) 
			*flags |= LOG_HIDE;
#endif
		else {
			fprintf(stderr, "** Error: Unrecognized log option '%s'\n", bp);
			return(-1);
		}

        if (ep != NULL)
            *ep++ = ',';
        bp = ep;

        /* List ends with ',' */
        if ((bp != NULL) && *bp == '\0')
            break;
    }

    return(0);
}


void
usage(char *pname)
{
	printf("\nConfigure logging of calls to execve(2)\n");
    printf("Usage: %s [on|off] [Option(s)]\n", pname);
    printf("\nOptions:\n");
	printf("  -d log_opt[,log_opt,...]  Disable log option(s)\n");
	printf("  -e log_opt[,log_opt,...]  Enable log option(s)\n");
	printf("  -h                        This help\n");
    printf("  -u user1[,user2,...]      Set list of users to log\n");
	printf("\nLog options:\n");
	printf("  env     Log environment strings\n");
	printf("  euid    Log effective UID\n");
	printf("  suid    Log saved effective UID\n");
	printf("  egid    Log effective GID\n");
	printf("  sgid    Log saved effective GID\n");
	printf("  tste    Log calls where effective UID is in list of users\n");
    printf("\n");
}

int 
main(int argc, char **argv)
{
	struct log_options curr;
	char hidefile[MAXPATHLEN];
	int uname = 1;
	u_int flags;
	int i;

	memset(&curr, 0x00, sizeof(curr));
	memset(&hidefile, 0x00, sizeof(hidefile));

	if (get_logopts(&curr) < 0)
		exit(EXIT_FAILURE);

	if (argc <= 1) {
		if (print_logopts(&curr, uname) < 0)
			exit(EXIT_FAILURE);
		exit(EXIT_SUCCESS);
	}

	if (!strncasecmp(argv[1], "on", 2)) {
		curr.flags &= (~LOG_OFF);
		argc--;
		argv++;
	}
	else if (!strncasecmp(argv[1], "off", 3)) {
		curr.flags |= LOG_OFF;
		argc--;
		argv++;
	}

	while ( (i = getopt(argc, argv, "d:e:u:h")) != -1) {
		switch(i) {
			case 'd': 
				if (get_flags(optarg, &flags) < 0)
					exit(EXIT_FAILURE);
				curr.flags &= (~flags);
				break;
			case 'e': 
				if (get_flags(optarg, &flags) < 0)
					exit(EXIT_FAILURE);
				curr.flags |= flags;
				break;
			case 'h': 
				usage(argv[0]); 
				exit(EXIT_FAILURE);
				break;
			case 'u':
				if (get_uids(optarg, curr.uids, 
						&curr.uids_num, LOG_MAX_UIDS) < 0)
					exit(EXIT_FAILURE);
				break;
			default: exit(EXIT_FAILURE);
		}
	}

	if (set_logopts(&curr) < 0) 
		exit(EXIT_FAILURE);
	
	exit(EXIT_SUCCESS);
}
