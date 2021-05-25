# OpenBSD Kernel modification for logging execve calls
## By Claes M Nyberg, Spring 2021

## How this was implemented
1. Edited /src/sys/kern_exec.c and added new syscall and wrapper function for sys_execve
2. Appended sys_logexecve to syscalls.master 
3. Executed 'make syscalls' in /sys/kern
4. Edited logex.c and set the syscall number

## How to build
1. Merge source into /usr/src/sys
2. Compile and install new kernel
3. Compile logex by running make

## How to install
1. Copy kernel to /bsd
2. Copy logex binary to /bin


## Usage
The logging configuration is kept in kernel memory only and 
is not preserved during reboot. Unless logex is executed
during boot nothing is logged. We get the current configuration
by simply running logex without arguments:

```
obsd69# logex                                                                                 
	  on: yes
	 env: no
	euid: no
	suid: no
	egid: no
	sgid: no
	tste: no
	** No users set to log!
```

We can also get a usage description with the -h option:

```
obsd69# logex -h

Configure logging of calls to execve(2)
Usage: logex [on|off] [Option(s)]

Options:
  -d log_opt[,log_opt,...]  Disable log option(s)
  -e log_opt[,log_opt,...]  Enable log option(s)
  -h                        This help
  -u user1[,user2,...]      Set list of users to log

Log options:
  env     Log environment strings
  euid    Log effective UID
  suid    Log saved effective UID
  egid    Log effective GID
  sgid    Log saved effective GID
  tste    Log calls where effective UID is in list of users
```

So, to see all execve(2) calls made by root, run logex:

```
obsd69# logex -u root
```

And check that the configuration was installed:

```
obsd69# logex                                                                                 
   on: yes
  env: no
 euid: no
 suid: no
 egid: no
 sgid: no
 tste: no
users: 0(root) 
```

Now, lines will be logged to /var/log/messages:

```
May 25 15:49:07 obsd69 /bsd: pid=42566 ppid=85134 uid=0 argv={"logex"} 
May 25 15:49:23 obsd69 /bsd: pid=34040 ppid=85134 uid=0 argv={"ls"} 
```
