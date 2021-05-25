/*
 *  logexecve.h - Header file for execve logger
 *
 *  Copyright (c) 2004-2021 Claes M. Nyberg <cmn@signedness.org>
 *  All rights reserved, all wrongs reversed.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 *  AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 *  THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 *  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * $Id: logexecve.h,v 1.1 2005-12-29 22:41:00 cmn Exp $
 */
#ifndef _LOGEXECVE_H
#define _LOGEXECVE_H

#include <sys/types.h>

/*
 * Debug macro
 */
#ifdef DEBUG

    /* Only log if verbose level is high enough */
    #define debug(level, fmt, args...) \
        if (DEBUG >= level) { \
            printf("logexecve DEBUG: " fmt, ##args); }
#else
    #define debug(level, fmt, args...)
#endif    /* DEBUG */


/* Maximum number of UIDS to keep track of */
#define LOG_MAX_UIDS    128

struct execve_logger_args {
        pid_t pid;  /* PID */
    pid_t ppid; /* Parent PID */
        
        uid_t uid;  /* Real UID */
        uid_t euid; /* Effective UID */
        uid_t suid; /* Saved effective UID */
        
        gid_t gid;  /* Real GID */
        gid_t egid; /* Effective GID */
        gid_t sgid; /* Saved effective GID */
    
        char *file; /* File to execute */
    char **argv; 
    char **envp;
};

/* Option flags */
#define LOG_OFF                 0x00000001      /* Disable logging */
#define LOG_ENVP                0x00000002  /* Log environment */
#define LOG_EUID                0x00000004  /* Log Effective UID */
#define LOG_SUID                0x00000008  /* Log Saved Effective UID */
#define LOG_EGID                0x00000010  /* Log Effective GID */
#define LOG_SGID                0x00000020  /* Log Saved Effective GID */
#define LOG_TSTE                0x00000040  /* Log call if effective UID is part of list */
#ifdef ENABLE_FILEHIDE
#define LOG_HIDE                0x00000080  /* Hide logfile */
#endif

struct log_options {
        u_int flags;            /* Option flags */
        u_long uids[LOG_MAX_UIDS];
        size_t uids_num;                /* Number of UID's in uids */
};

/* Syscall flags */
#define LOGOPT_GETOPT   0x01
#define LOGOPT_SETOPT   0x02
#ifdef ENABLE_FILEHIDE
#define LOGOPT_GETFILE  0x04
#define LOGOPT_SETFILE  0x08
#endif

#endif /* _LOGEXECVE_H */

