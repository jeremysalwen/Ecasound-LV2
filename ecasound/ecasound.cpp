// ------------------------------------------------------------------------
// ecasound.cpp: Console mode user interface to ecasound.
// Copyright (C) 2002 Kai Vehmanen (kai.vehmanen@wakkanet.fi)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
// ------------------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <string>

#include <pthread.h>   /* POSIX: pthread_create() */
#include <signal.h>    /* POSIX: sigaction(), sigwait(), sig_atomic_t */
#include <unistd.h>    /* POSIX: sleep() */

#include <kvu_dbc.h>
#include <kvu_com_line.h>

#include <eca-control.h>
#include <eca-error.h>
#include <eca-logger.h>
#include <eca-session.h>
#include <eca-version.h>

#include "eca-comhelp.h"
#include "eca-console.h"
#include "eca-curses.h"
#include "eca-plaintext.h"
#include "textdebug.h"

/**
 * Type definitions
 */

struct ecasound_state {
  ECA_CONSOLE* console;
  ECA_CONTROL* control;
  ECA_LOGGER_INTERFACE* logger;
  ECA_SESSION* session;
  sig_atomic_t exit_request;
  int retval;
  bool daemon_mode;
  bool interactive_mode;
  bool quiet_mode;
};

/**
 * Static/private function definitions
 */

static void ecasound_atexit_cleanup(void);
static void ecasound_create_eca_objects(struct ecasound_state* state, COMMAND_LINE& cline);
static void ecasound_launch_daemon(struct ecasound_state* state);
static void ecasound_main_loop(struct ecasound_state* state);
void ecasound_parse_command_line(struct ecasound_state* state, 
				 const COMMAND_LINE& clinein,
				 COMMAND_LINE* clineout);
static void ecasound_print_usage(void);
static void ecasound_print_version_banner(void);
static void ecasound_setup_signals(struct ecasound_state* state);
static void* ecasound_watchdog_thread(void* arg);

static struct ecasound_state ecasound_state_global = 
  { 0,        /* ECA_CONSOLE* */
    0,        /* ECA_CONTROL */
    0,        /* ECA_LOGGER_INTERFACE */
    0,        /* ECA_SESSION */
    0,        /* sig_wait_t */
    0,        /* return value */
    false,    /* daemon mode */
    false,    /* interactive mode */
    false     /* quiet mode */
  };

/**
 * Namespace imports
 */
using namespace std;

/**
 * Function definitions
 */
int main(int argc, char *argv[])
{
  /* FIXME: store a global pointer for cleanup */
  struct ecasound_state* state = &ecasound_state_global;

  /* 1. setup signals and watchdog thread */
  ecasound_setup_signals(state);

  /* 2. parse command-line args */
  COMMAND_LINE* cline = new COMMAND_LINE(argc, argv);
  COMMAND_LINE* clineout = new COMMAND_LINE();
  ecasound_parse_command_line(state, *cline, clineout); 
  delete cline; cline = 0;

  /* 3. create console interface */
  if (state->retval == 0) {
   
#if defined(ECA_USE_NCURSES) || defined (ECA_USE_TERMCAP)
    if (state->quiet_mode != true) {
      state->console = new ECA_CURSES();
      state->logger = new TEXTDEBUG();
      ECA_LOGGER::attach_logger(state->logger);
    }
    else 
#endif
      {
	state->console = new ECA_PLAIN_TEXT();
      }
    
    /* 4. print banner */
    state->console->print_banner();

    /* 5. set default debug levels */
    ECA_LOGGER::instance().set_log_level(ECA_LOGGER::errors, true);
    ECA_LOGGER::instance().set_log_level(ECA_LOGGER::info, true);
    ECA_LOGGER::instance().set_log_level(ECA_LOGGER::subsystems, true);
    
    /* 6. create eca objects */
    ecasound_create_eca_objects(state, *clineout);
    delete clineout; clineout = 0;

    /* 7. start ecasound daemon */
    if (state->retval == 0) {
      if (state->daemon_mode == true) {
	ecasound_launch_daemon(state);
      }
    }
     
    /* 8. start processing */
    if (state->retval == 0) {
      ecasound_main_loop(state);
    }
  }

  // cerr << endl << "ecasound: main() exiting..." << endl;
  
  return(state->retval);
}

/**
 * Cleanup routine that is run after either exit()
 * is called or ecasound returns from its main().
 */
void ecasound_atexit_cleanup(void)
{
  struct ecasound_state* state = &ecasound_state_global;

  // cerr << endl << "ecasound: atexit cleanup" << endl;

  if (state != 0 && 
      state->control != 0) {

    if (state->control->is_running() == true) {
      state->control->stop_on_condition();
    }
    
    if (state->control->is_connected() == true) {
      state->control->disconnect_chainsetup();
    }
  }

  if (state != 0) {
    if (state->control != 0) { delete state->control; state->control = 0; }
    if (state->session != 0) { delete state->session; state->session = 0; }
    if (state->console != 0) { delete state->console; state->console = 0; }
  }    

  // cerr << "ecasound: atexit cleanup done." << endl << endl;
}

/**
 * Enters the main processing loop.
 */
void ecasound_create_eca_objects(struct ecasound_state* state, 
				 COMMAND_LINE& cline)
{
  DBC_REQUIRE(state != 0);
  DBC_REQUIRE(state->console != 0);

  try {
    state->session = new ECA_SESSION(cline);
    state->control = new ECA_CONTROL(state->session);

    DBC_ENSURE(state->session != 0);
    DBC_ENSURE(state->control != 0);
  }
  catch(ECA_ERROR& e) {
    state->console->print("---\necasound: ERROR: [" + e.error_section() + "] : \"" + e.error_message() + "\"\n");
    state->retval = -1;
  }
}

/**
 * Launches a background daemon that allows NetECI 
 * clients to connect to the current ecasound
 * session.
 */
void ecasound_launch_daemon(struct ecasound_state* state)
{
  DBC_REQUIRE(state != 0);
  DBC_REQUIRE(state->console != 0);

  state->console->print("ecasound: starting the NetECI server.");

  state->console->print("ecasound: NetECI server exiting");
}

/**
 * The main processing loop.
 */
void ecasound_main_loop(struct ecasound_state* state)
{
  DBC_REQUIRE(state != 0);
  DBC_REQUIRE(state->console != 0);

  ECA_CONTROL* ctrl = state->control;

  if (state->interactive_mode == true) {
    while(state->exit_request == 0) {
      state->console->read_command("ecasound ('h' for help)> ");
      const string& cmd = state->console->last_command();
      if (cmd.size() > 0 && state->exit_request == 0) {
	ctrl->command(cmd);
	ctrl->print_last_error();
	ctrl->print_last_value();
	if (cmd == "quit" || cmd == "q") {
	  state->console->print("---\necasound: Exiting...");
	  break;
	}
      }
    }
  }
  else {
    ctrl->connect_chainsetup();
    if (ctrl->is_connected() == true) {
      ctrl->run();
    }
    else {
      ctrl->print_last_error();
    }
  }
  // cerr << endl << "ecasound: mainloop exiting..." << endl;
}

/**
 * Parses the command lines options in 'cline'.
 */
void ecasound_parse_command_line(struct ecasound_state* state, 
				 const COMMAND_LINE& cline,
				 COMMAND_LINE* clineout)
{
  if (cline.size() < 2) {
    ecasound_print_usage();
    state->retval = -1;
  }
  else {
   cline.begin();
    while(cline.end() != true) {
      if (cline.current() == "-o:stdout" ||
	  cline.current() == "stdout" ||
	  cline.current() == "-d:0" ||
	  cline.current() == "-q") {

	state->quiet_mode = true;
	/* pass option to libecasound */
	clineout->push_back(cline.current());
      } 

      else if (cline.current() == "-c") {
	state->interactive_mode = true;
      }

      else if (cline.current() == "-C") {
	state->interactive_mode = true;
      }

      else if (cline.current() == "-D") {
	/* FIXME: todo */
	// state->cerr_output_only = true;
      }
      
      else if (cline.current() == "--daemon") {
	state->daemon_mode = true;
      }

      else if (cline.current() == "-h" ||
	       cline.current() == "--help") {
	ecasound_print_usage();
	state->retval = -1;
	break;
      }

      else if (cline.current() == "--version") {
	ecasound_print_version_banner();
	state->retval = -1;
	break;
      }
      
      else {
	/* pass rest of the options to libecasound */
	clineout->push_back(cline.current());
      }

      cline.next();
    }
  }
}

void ecasound_print_usage(void)
{
  cout << ecasound_parameter_help();
}

void ecasound_print_version_banner(void)
{
  cout << "ecasound v" << ecasound_library_version << endl;
  cout << "Copyright (C) 1997-2002 Kai Vehmanen" << endl;
  cout << "Ecasound comes with ABSOLUTELY NO WARRANTY." << endl;
  cout << "You may redistribute copies of ecasound under the terms of the GNU" << endl;
  cout << "General Public License. For more information about these matters, see" << endl; 
  cout << "the file named COPYING." << endl;
}

/**
 * Sets up a signal mask with sigaction() that blocks 
 * all common signals, and then launces an watchdog
 * thread that waits on the blocked signals using
 * sigwait().
 */
void ecasound_setup_signals(struct ecasound_state* state)
{
  pthread_t watchdog;

  /* man pthread_sigmask:
   *  "...signal actions and signal handlers, as set with
   *   sigaction(2), are shared between all threads"
   */

  struct sigaction blockaction;
  blockaction.sa_handler = SIG_IGN;
  sigemptyset(&blockaction.sa_mask);
  blockaction.sa_flags = 0;

  /* ignore the following signals */
  sigaction(SIGTERM, &blockaction, 0);
  sigaction(SIGINT, &blockaction, 0);
  sigaction(SIGHUP, &blockaction, 0);
  sigaction(SIGPIPE, &blockaction, 0);

  int res = pthread_create(&watchdog, 
			   NULL, 
			   ecasound_watchdog_thread, 
			   reinterpret_cast<void*>(state));
  if (res != 0) {
    cerr << "ecasound: Warning! Unable to create watchdog thread." << endl;
  }
}

void* ecasound_watchdog_thread(void* arg)
{
  sigset_t signalset;
  struct ecasound_state* state = reinterpret_cast<struct ecasound_state*>(arg);
  int signalno;

  /* register cleanup routine */
  atexit(&ecasound_atexit_cleanup);

  // cerr << "Watchdog-thread created, pid=" << getpid() << "." << endl;

  sigemptyset(&signalset);

  /* handle the following signals explicitly */
  sigaddset(&signalset, SIGTERM);
  sigaddset(&signalset, SIGINT);
  sigaddset(&signalset, SIGHUP);
  sigaddset(&signalset, SIGPIPE);

  /* block until a signal received */
  sigwait(&signalset, &signalno);

  state->exit_request = 1;
  
  // cerr << endl << "ecasound: watchdog-thread received signal " << signalno << ". Cleaning up..." << endl;

  /* give time for the console interface thread to die gracefully */
  sleep(1);

  // cerr << endl << "ecasound: watchdog-thread exiting..." << endl;

  exit(state->retval);
}
