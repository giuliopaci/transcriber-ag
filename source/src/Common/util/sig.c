/**
 **  Title:
 **	sig.c:	system signals management module
 **
 **  SccsId:
 **	%W%	%E%	
 **
 **
 **  Purpose:
 **	initialize signals handlers to cleanly exit from a program if any
 **	system signal is received.  The user-defined function 'exit_func',
 **	passed when sig_init function is called, is then activated.
 **	This function should always be terminated by the exit() function,
 **	else the program will never stop except if KILL signal is sent.
 **
 **
 **  Author:
 **	P. Lecuyer
 **
 **/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/param.h>

#include <execinfo.h>
#include <linux/ptrace.h>
#include <asm/ucontext.h>

#include "sig.h"

static exit_func_t exit_funct;

static int sig_count=0;

inline void display_callstack(void) {
  void *btinfo[100];
  char **strings;
  size_t btsize;
  unsigned int i;

  btsize = backtrace(btinfo, 100);
  strings = backtrace_symbols(btinfo, btsize);

  printf(" --=== CALLSTACK ===--\n");
  for(i = 0; i < btsize; i++)
    printf(" #%d : %s\n", i, strings[i]);
  printf(" --=== CALLSTACK END ===--\n");

  free(strings);
}


/*
 * signals handlers
 */

static void sig_sighup(int num)	/* terminal hangup */
{
	printf("-- SIGNAL %d RECU: terminal hangup\n", num);
	sig_count++;
	if ( sig_count == 1 ) exit_funct();
	else exit(2);
}

static void sig_sigill(int num)	/* illegal instruction */
{
	printf("-- SIGNAL %d RECU: illegal instruction (often floating point error)\n", num);
	sig_count++;
	if ( sig_count == 1 ) exit_funct();
	else exit(2);
}

static void sig_sigtrap(int num)	/* trap flag */
{
	printf("-- SIGNAL %d RECU: system trap flag\n", num);
	sig_count++;
	if ( sig_count == 1 ) exit_funct();
	else exit(2);
}

static void sig_sigfpe(int num)	/* floating point exception */
{
	printf("-- SIGNAL %d RECU: floating point exception\n", num);
	printf("-- SIGNAL %d RECU: (divide by zero or overflow or floating point illegal format)\n", num);
	sig_count++;
	if ( sig_count == 1 ) exit_funct();
	else exit(2);
}

static void sig_sigbus(int num)	/* bus error */
{
	printf("-- SIGNAL %d RECU: bus error\n", num);
	sig_count++;
	if ( sig_count == 1 ) exit_funct();
	else exit(2);
}

/*static void sig_sigsegv(int num)*/	/* segmentation violation */
static void  sig_sigsegv(int n, siginfo_t *ist, void *extra) {
  static struct ucontext *puc;
  static void *btinfo[100];
  static char **strings;
  static size_t btsize;
  static int i, fd;
  static unsigned int old_eip, old_ebp;
  char buffer[512];

  puc = (struct ucontext *)extra;
  sprintf(buffer, "/tmp/%s.backtrace.%d", "TransAG", getpid());
  fprintf(stderr,
	  "SEGFAULT : acces à adresse 0x%08x invalide en 0x%08x\n"
	  "SEGFAULT : voir backtrace dans %s\n",
	  (uint32_t)ist->si_addr,
	  (uint32_t)puc->uc_mcontext.eip,
	  buffer
	  /*, puc->uc_mcontext.ebp */);

  if((fd = open(buffer, O_CREAT | O_EXCL | O_WRONLY, 0666)) == -1) {
    perror("open");
    return;
  }

  i = snprintf(buffer, 512, "SEGFAULT : acces à adresse 0x%08x invalide en 0x%08x\n",
	       (uint32_t)ist->si_addr,
	       (uint32_t)puc->uc_mcontext.eip
	       /*, puc->uc_mcontext.ebp */);
  if(write(fd, buffer, i) == -1) {
    perror("write");
    return;
  }

  old_eip = *(unsigned int*)((void*)&n-4);
  old_ebp = *(unsigned int*)((void*)&n-8);
  *(unsigned int*)((void*)&n-4) = puc->uc_mcontext.eip;
  *(unsigned int*)((void*)&n-8) = puc->uc_mcontext.ebp;

  btsize = backtrace(btinfo, 100);
  strings = backtrace_symbols(btinfo, btsize);

  for(i = 1;          /* on ignore la fonction dans laquelle on se trouve */
      i < btsize - 2; /* ne descend pas en dessous du main */
      i++)
    write(fd, buffer, snprintf(buffer, 512, " #%d : %s\n", i-1, strings[i]));
  close(fd);

  free(strings);

  *(unsigned int*)((void*)&n-4) = old_eip;
  *(unsigned int*)((void*)&n-8) = old_ebp;
}

static void sig_sigint(int num)	/* process interruption */
{
	printf("-- SIGNAL %d RECU: process interruption on user request\n", num);
	sig_count++;
	if ( sig_count == 1 ) exit_funct();
	else exit(2);
}

static void sig_sigquit(int num)	/* quit process */
{
	printf("-- SIGNAL %d RECU: quit process on user request\n", num);
	sig_count++;
	if ( sig_count == 1 ) exit_funct();
	else exit(2);
}

static void sig_sigterm(int num)	/* software termination */
{
	printf("-- SIGNAL %d RECU: software termination\n", num);
	sig_count++;
	if ( sig_count == 1 ) exit_funct();
	else exit(2);
}

static void sig_sigabrt(int num)	/* abort process */
{
	printf("-- SIGNAL %d RECU: abort signal received\n", num);
	
	display_callstack();

	sig_count++;
	if ( sig_count == 1 ) exit_funct();
	else exit(2);
}

#ifdef _NOT_USED

static void sig_sigemt(int num)	/* emt instruction */
{
	printf("-- SIGNAL %d RECU: EMT instruction\n", num);
	sig_count++;
	if ( sig_count == 1 ) exit_funct();
	else exit(2);
}

static void sig_sigsys(int num)	/* bad argument to system call */
{
	printf("-- SIGNAL %d RECU: bad argument to system call\n", num);
	sig_count++;
	if ( sig_count == 1 ) exit_funct();
	else exit(2);
}
#endif


static void sig_sigpipe(int num)	/* write on a pipe with no one to read it */
{
	printf("-- SIGNAL %d RECU: write on a pipe with no one to read it\n", num);
	sig_count++;
	if ( sig_count == 1 ) exit_funct();
	else exit(2);
}

static void sig_sigurg(int num)	/* urgent condition on IO channel */
{
	printf("-- SIGNAL %d RECU: urgent condition on IO channel\n", num);
	sig_count++;
	if ( sig_count == 1 ) exit_funct();
	else exit(2);
}

#ifdef _NOT_USED
static void sig_sigchld(int num)	/* a child process has terminated */
{
  static int wait_status;
  /* waiting for terminated child to clear it from processes table */
  (void)wait(&wait_status);	
}
#endif

/* 
 *  sig_init :  signals handlers initialization
 */
void sig_init(exit_func_t func)
{
  struct sigaction sigst;
  
  exit_funct = (func ? func : main_exit);

  signal(SIGHUP, sig_sighup);
  signal(SIGILL, sig_sigill);
  signal(SIGTRAP, sig_sigtrap);
  signal(SIGFPE, sig_sigfpe);
  signal(SIGBUS, sig_sigbus);
 // signal(SIGSEGV, sig_sigsegv);
//  signal(SIGINT, sig_sigint);
//  signal(SIGQUIT, sig_sigquit);
//  signal(SIGTERM, sig_sigterm);
  signal(SIGABRT, sig_sigabrt);
  //	signal(SIGEMT, sig_sigemt);
  //	signal(SIGSYS, sig_sigsys);
//  signal(SIGPIPE, sig_sigpipe);
//  signal(SIGURG, sig_sigurg);
  //	signal(SIGCHLD, sig_sigchld);

  sigst.sa_sigaction = sig_sigsegv;
  sigemptyset(&sigst.sa_mask);
  sigst.sa_flags = SA_SIGINFO | SA_ONESHOT;

  if(sigaction(SIGSEGV, &sigst, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }
}

/*
 *	positionnement d'un handler popur signaux SIGCHLD
 */
void sig_sigchld_handler(sig_handler_t handler)
{
  signal(SIGCHLD, (handler != NULL  ? handler : SIG_DFL));
}



void main_exit()
{
  (void)printf("Exiting...\n");
  exit(1);
}


/*
 *  TEST MODULE
 */
#ifdef TEST
main()
{
	sig_init(NULL);
	(void)printf("\n\n          signals handling - test module\n\n");
	(void)printf("This program is going to loop until a signal is received.\n");
	(void)printf("To perform this test, you can either run this program in background\n");
	(void)printf("or send signals from another terminal.\n");
	(void)printf("To send a signal, use KILL command with the appropriate signal name:\n");
	(void)printf("one of HUP, ILL, TRAP, FPE, BUS, SEGV, INT, QUIT, TERM, ABRT, EMT,\n");
	(void)printf(" SYS, PIPE, URG \n");
	(void)printf("\nprocess id is : %d\n\n", getpid());
	(void)printf("waiting for signal...\n");
	for(;;) sleep(1);
}
#endif	/* TEST */
