#include "../inc/terminal.hpp"

int Terminal::terminal_descriptor = -1;


    static termios terminal_original;
    static termios terminal_settings;

/* Restore terminal to original settings */
void Terminal::terminalDone() {
 if (terminal_descriptor != -1)
    tcsetattr(terminal_descriptor, TCSANOW, &terminal_original);
}
 

/* "Default" signal handler: restore terminal, then exit.  */
void Terminal::terminalSignal(int signum) {
  cout << "Terminal_signal(" << signum << ")" << endl;
  if (terminal_descriptor != -1)
    tcsetattr(terminal_descriptor, TCSANOW, &terminal_original);

  /* exit() is not async-signal safe, but _exit() is.
   * Use the common idiom of 128 + signal number for signal exits.
   * Alternative approach is to reset the signal to default handler,
   * and immediately raise() it. */
  _exit(128 + signum);
}
 

/* 
 * Initialize terminal for non-canonical, non-echo mode,
 * that should be compatible with standard C I/O.
 * Returns 0 if success, nonzero errno otherwise.
*/
int Terminal::terminalInit() {

  /* Already initialized? */
  if (terminal_descriptor != -1)
    return errno = 0;

  /* Which standard stream is connected to our TTY? */
  if (isatty(STDERR_FILENO))
    terminal_descriptor = STDERR_FILENO;
  else
  if (isatty(STDIN_FILENO))
    terminal_descriptor = STDIN_FILENO;
  else
  if (isatty(STDOUT_FILENO))
    terminal_descriptor = STDOUT_FILENO;
  else
    return errno = ENOTTY;

  /* Obtain terminal settings. */
  if (tcgetattr(terminal_descriptor, &terminal_original) ||
      tcgetattr(terminal_descriptor, &terminal_settings))
  {
    return errno = ENOTSUP;
  }

  // Disable buffering for terminal streams.
  if (isatty(STDIN_FILENO))
    setvbuf(stdin, NULL, _IONBF, 0);
  if (isatty(STDOUT_FILENO))
    setvbuf(stdout, NULL, _IONBF, 0);
  if (isatty(STDERR_FILENO))
    setvbuf(stderr, NULL, _IONBF, 0);

  /* At exit() or return from main(),
   * restore the original settings. */
  if (atexit(Terminal::terminalDone))
    return errno = ENOTSUP;

  /* Set new "default" handlers for typical signals,
   * so that if this process is killed by a signal,
   * the terminal settings will still be restored first. */
  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_handler = Terminal::terminalSignal;
  act.sa_flags = 0;
  if (sigaction(SIGHUP,  &act, NULL) ||
      sigaction(SIGINT,  &act, NULL) ||
      sigaction(SIGQUIT, &act, NULL) ||
      sigaction(SIGTERM, &act, NULL) ||
#ifdef SIGXCPU
      sigaction(SIGXCPU, &act, NULL) ||
#endif
#ifdef SIGXFSZ    
      sigaction(SIGXFSZ, &act, NULL) ||
#endif
#ifdef SIGIO
      sigaction(SIGIO,   &act, NULL) ||
#endif
      sigaction(SIGPIPE, &act, NULL) ||
      sigaction(SIGALRM, &act, NULL))
  {
    return errno = ENOTSUP;
  }

  terminal_settings.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN);
  terminal_settings.c_cflag &= ~(CSIZE | PARENB);
  terminal_settings.c_cflag |= CS8;
  terminal_settings.c_cc[VMIN] = 0;
  terminal_settings.c_cc[VTIME] = 0;

  /* Done. */
  return errno = 0;
}


void Terminal::setRerminalRawMode() {

  /* Set the new terminal settings.
   * Note that we don't actually check which ones were successfully
   * set and which not, because there isn't much we can do about it. */
  if (terminal_descriptor != -1)
    tcsetattr(terminal_descriptor, TCSANOW, &terminal_settings);
}


void Terminal::setTerminalOriginalMode() {
  /* Restore original terminal settings. */
  if (terminal_descriptor != -1)
    tcsetattr(terminal_descriptor, TCSANOW, &terminal_original);
}

Terminal::Terminal(CPU *cpu){
  this->cpu = cpu;
  this->terminalInit();
  this->setRerminalRawMode();
}

void Terminal::stdinput() {
   char in;
    if (read(STDIN_FILENO, &in, 1) == 1) {
        cpu->mem[term_in] = in;
        cpu->mem[term_in + 1] = 0;
        //cout << cpu->mem[term_in] << endl;
        cpu->int_ev |= 1 << 3;
    }
}

Terminal::~Terminal() {
  this->terminalDone();
}