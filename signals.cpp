#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num)
{
  // TODO: Add your implementation
}

void ctrlCHandler(int sig_num)
{
  //   // TODO: Add your implementation
  cout << "smash: got ctrl-C" << endl;
  pid_t pid = SmallShell::getInstance().getCurrentCmdPid();
  if (pid != -1)
  {
    kill(pid, SIGINT);
    SmallShell::getInstance().setCurrentCmdPid(-1);
    SmallShell::getInstance().setCurrentCmd(nullptr);
  }
}

void alarmHandler(int sig_num)
{
  // TODO: Add your implementation
}
