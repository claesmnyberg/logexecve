
/* Log options */
struct log_options logopt;

/* File descriptor of logfile */
static int logfd = -1;

/*
 * Scan through list of UID's to see if
 * the argument given is part of that list.
 * Return true if so, false otherwise.
 */
int
log_me(uid_t uid)
{
	u_int i; 
	
	for (i=0; i<logopt.uids_num; i++) {
		if (logopt.uids[i] == uid)
			return(1);
	}
	return(0);
}


/*
 * Logger function
 */
int
execve_logger(struct execve_logger_args *args)
{
	int i;
	
	debug(1, "Entered execve_logger\n");

	/* Logging disabled */
	if ((logopt.flags & LOG_OFF) || (logopt.uids_num == 0)) {
		debug(1, "Logging disabled\n");
		return(-1);
	}

	if (!log_me(args->uid)) {
		debug(1, "UID (%d) not part of user list\n", args->uid);
		/* Test effective UID as well */
		if ((logopt.flags & LOG_TSTE) && !log_me(args->euid)) {
			debug(1, "EUID (%d) not part of user list\n", args->euid);
			return(-1);
		}
		else
			return(-1);
	}

	printf("pid=%u ppid=%u uid=%u ", args->pid, args->ppid, args->uid);
	if (logopt.flags & LOG_EUID)
		printf("euid=%u ", args->euid);
	if (logopt.flags & LOG_SUID)
		printf("suid=%u ", args->suid);
	if (logopt.flags & LOG_EGID)
		printf("egid=%u ", args->egid);
	if (logopt.flags & LOG_EGID)
		printf("sgid=%u ", args->sgid);

	printf("argv={");
	for (i=0; args->argv[i] != NULL; i++) {
		printf("\"%s\"", args->argv[i]);

		if (args->argv[i+1] != NULL)
			printf(", ");
	}
	printf("} ");

	if (logopt.flags & LOG_ENVP) {
		printf("envp={");
		for (i=0; args->envp[i] != NULL; i++) {
			printf("\"%s\"", args->envp[i]);

			if (args->envp[i+1] != NULL)
				printf(", ");
		}
		printf("} ");
	}

	printf("\n");
	return(0);	
}


/*
 * Set logging options
 */
int
logopts(uid_t uid, u_int opt, void *data, size_t data_len)
{
	int error;

	/* Must be root */
	if (uid != 0) 
		return(EACCES);

	/* Send current settings to userland */
	if (opt == LOGOPT_GETOPT) {
		if (data_len != sizeof(struct log_options))
			return(EINVAL);

		error = copyout(&logopt, data, data_len);
	}

	/* Save new settings */
	else if (opt == LOGOPT_SETOPT) {
		if (data_len != sizeof(struct log_options))
			return(EINVAL);
		
		error = copyin(data, &logopt, sizeof(struct log_options));
	}
	else
		return(EINVAL);

	return(error);
}
