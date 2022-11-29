#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num)
{
  // TODO: Add your implementation
  cout << "smash: got ctrl-Z" << endl;
  pid_t pid = SmallShell::getInstance().getCurrentCmdPid();
  if (pid != -1)
  {
    kill(pid, SIGSTOP);
    Command *cmd = SmallShell::getInstance().getCurrentCmd();
    int job_id = SmallShell::getInstance().getCurrentJobId();
    if (job_id == -1)
    {
      SmallShell::getInstance().addJob(cmd, true);
    }
    else
    {
      SmallShell::getInstance().addJobWihId(cmd, job_id, true);
    }
    SmallShell::getInstance().setCurrentCmdPid(-1);
    SmallShell::getInstance().setCurrentCmd(nullptr);

    cout << "smash: process " << int(pid) << " was stopped" << endl;
  }
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
    cout << "smash: process " << int(pid) << " was killed" << endl;
  }
}

// void alarmHandler(int sig_num)
// {
//   // TODO: Add your implementation
// }
