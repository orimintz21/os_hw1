#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <memory>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <signal.h>

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY() \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args)
{
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for (std::string s; iss >> s;)
  {
    args[i] = (char *)malloc(s.length() + 1);
    memset(args[i], 0, s.length() + 1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line)
{
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos)
  {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&')
  {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// almog
// bool isPipeCommand(vector<string> args, int *index)
// {
//   for (int i = 0; i < args.size(); i++)
//   {
//     if (args[i] == ">>" || args[i] == ">" || args[i] == "|&" || args[i] == "|")
//     {
//       *index = i;
//       return true;
//     }
//   }
//   *index = -1;
//   return false;
// }
// almog
// void splitPipeCommand(vector<string> args, vector<string> &args1, vector<string> &args2, int index)
// {
//   for (int i = 0; i < index; i++)
//   {
//     args1.push_back(args[i]);
//   }
//   for (int i = index + 1; i < args.size(); i--)
//   {
//     args2.push_back(args[i]);
//   }
// }

FUNC_ENTRY()
// TODO: Add your implementation for classes in Commands.h
Command::Command(string _cmd_line) : _pid(-1), _cmd_line(_cmd_line)
{
}
Command::Command(pid_t pid, string cmd_line) : _pid(pid), _cmd_line(cmd_line)
{
}
pid_t Command::getPid() { return _pid; }
string Command::getCmdLine() { return _cmd_line; }

BuiltInCommand::BuiltInCommand(string cmd_line) : Command(cmd_line)
{
}

JobsList::JobEntry::JobEntry(int job_id, pid_t pid, const string cmd_line, bool isStopped, Command *cmd) : _job_id(job_id),
                                                                                                           _command(cmd_line),
                                                                                                           _pid(pid),
                                                                                                           _isStopped(isStopped),
                                                                                                           _start_time(time(NULL)),
                                                                                                           _cmd(cmd)
{
}

void JobsList::addJob(Command *cmd, bool isStopped)
{
  removeFinishedJobs();
  int job_id = 1;
  if (!_jobs.empty())
  {
    job_id = (--_jobs.end())->first + 1;
  }
  JobEntry newJobEntry = JobEntry(job_id, cmd->getPid(), cmd->getCmdLine(), isStopped, cmd);
  _jobs.insert(pair<int, JobEntry>(job_id, newJobEntry));
}

void JobsList::printJobsList()
{
  removeFinishedJobs();
  for (auto cm : _jobs)
  {
    time_t now = time(NULL);
    double seconds = difftime(now, cm.second.getStartTime());
    cout << "[" << cm.first << "] " << cm.second.getCommand() << " : " << cm.second.getPid() << " " << int(seconds) << " secs";
    if (cm.second.isStopped())
    {
      cout << "stopped";
    }
    cout << endl;
  }
}

void JobsList::killAllJobs()
{
  removeFinishedJobs();
  cout << "smash: sending SIGKILL signal to " << _jobs.size() << " jobs:" << endl;
  for (auto cm : _jobs)
  {
    cout << cm.second.getPid() << ": " << cm.second.getCommand() << endl;
    kill(cm.second.getPid(), SIGKILL);
  }
  removeFinishedJobs();
}

void JobsList::removeFinishedJobs()
{
  for (auto it = _jobs.begin(); it != _jobs.end();)
  {
    if (waitpid(it->second.getPid(), NULL, WNOHANG) != 0)
    {
      it = _jobs.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

JobsList::JobEntry *JobsList::getJobById(int jobId)
{
  removeFinishedJobs();
  auto it = _jobs.find(jobId);
  if (it != _jobs.end())
  {
    return &(it->second);
  }
  return nullptr;
}

void JobsList::removeJobById(int jobId)
{
  auto it = _jobs.find(jobId);
  if (it != _jobs.end())
  {
    _jobs.erase(it);
    return;
  }
}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId)
{
  if (_jobs.empty())
  {
    return nullptr;
  }
  *lastJobId = (--_jobs.end())->first;
  return &((--_jobs.end())->second);
}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId)
{
  if (_jobs.empty())
  {
    return nullptr;
  }
  for (auto it = _jobs.rbegin(); it != _jobs.rend(); ++it)
  {
    if (it->second.isStopped())
    {
      *jobId = it->first;
      return &(it->second);
    }
  }
  return nullptr;
}

SmallShell::SmallShell() : _prompt("smash"), _preDir(""), _currentDir(""), _jobsList(), _current_cmd_pid(-1), _current_cmd(nullptr)
{
  // TODO: add your implementation
  char *path = getcwd(NULL, 0);

  _currentDir = string(path);
  free(path);
}

SmallShell::~SmallShell()
{
  // TODO: add your implementation
}

/**
 * Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 */
Command *SmallShell::CreateCommand(const char *cmd_line)
{

  bool inBackground = _isBackgroundComamnd(cmd_line);
  shared_ptr<char> cmd_cpy(new char[strlen(cmd_line) + 1]);
  strcpy(cmd_cpy.get(), cmd_line);
  _removeBackgroundSign(cmd_cpy.get());
  vector<string> args = convertToVector(cmd_cpy.get());
  string cmd = cmd_cpy.get();
  cmd = setFullCmd(cmd);
  // almog
  // int *index;
  // if (isPipeCommand(args, index))
  // {
  //   vector<string> args1;
  //   vector<string> args2;
  //   splitPipeCommand(args, args1, args2, *index);
  //   // split cmd maybe?
  //   return make_shared<PipeCommand>(cmd_line, cmd, args, args1, args2);
  // }
  if (args.size() == 0)
  {
    return nullptr;
  }
  string commend = args[0];
  if (commend == "chprompt")
  {
    return new ChpromptCommand(cmd, args);
  }
  else if (commend == "showpid")
  {
    return new ShowPidCommand(cmd, args);
  }
  else if (commend == "pwd")
  {
    return new GetCurrDirCommand(cmd, args);
  }
  else if (commend == "cd")
  {
    return new ChangeDirCommand(cmd, args);
  }
  else if (commend == "jobs")
  {
    return new JobsCommand(cmd, args);
  }
  else if (commend == "fg")
  {
    return new ForegroundCommand(cmd, args);
  }
  else if (commend == "bg")
  {
    return new BackgroundCommand(cmd, args);
  }
  else if (commend == "quit")
  {
    return new QuitCommand(cmd, args);
  }
  // else if (commend == "kill")
  // {
  //   return make_shared<KillCommand>(cmd, args);
  // }
  else
  {
    if (inBackground)
    {
      cmd += "&";
    }
    return new ExternalCommand(cmd_line, cmd, args, inBackground);
  }

  // For example:
  /*
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (firstWord.compare("pwd") == 0) {
      return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0) {
      return new ShowPidCommand(cmd_line);
    }
    else if ...
    .....
    else {
      return new ExternalCommand(cmd_line);
    }
    */
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line)
{
  // TODO: Add your implementation here
  // for example:
  Command *cmd = CreateCommand(cmd_line);
  cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

string &SmallShell::getPrompt()
{
  return _prompt;
}

vector<string> SmallShell::convertToVector(const char *cmd_line)
{
  vector<string> vec;
  string str = cmd_line;
  stringstream ss(str);
  string word;
  while (ss >> word)
  {
    vec.push_back(word);
  }

  return vec;
}

void SmallShell::goToDir(string &dir)
{
  _preDir = _currentDir;
  _currentDir = dir;
}

string &SmallShell::getCurrentDir()
{
  return _currentDir;
}

string &SmallShell::getPreDir()
{
  return _preDir;
}

void SmallShell::setCurrentDir(string &dir)
{
  _currentDir = dir;
}

string SmallShell::setFullCmd(string &cmd)
{
  string ans;
  stringstream ss(cmd);
  string word;
  while (ss >> word)
  {
    ans += word;
    ans += " ";
  }
  ans.pop_back();
  return ans;
}

ChpromptCommand::ChpromptCommand(string cmd_line, vector<string> &args) : BuiltInCommand(cmd_line)
{
  if (args.size() == 1)
  {
    _newPrompt = "smash";
  }
  else
  {
    _newPrompt = args[1];
  }
}

JobsList::JobEntry *SmallShell::getLastJobId(int *job_id)
{
  return _jobsList.getLastJob(job_id);
}

JobsList::JobEntry *SmallShell::getJobById(int job_id)
{
  return _jobsList.getJobById(job_id);
}

void ChpromptCommand::execute()
{
  SmallShell::getInstance().setPrompt(_newPrompt);
}

ShowPidCommand::ShowPidCommand(string cmd_line, vector<string> &args) : BuiltInCommand(cmd_line)
{
  _newPid = getpid();
}
void ShowPidCommand::execute()
{
  std::cout << int(_newPid) << endl;
}

GetCurrDirCommand::GetCurrDirCommand(string cmd_line, vector<string> &args) : BuiltInCommand(cmd_line)
{
  char *pwd = getcwd(NULL, 0);
  _currentDir = pwd;
  free(pwd);
}
void GetCurrDirCommand::execute()
{
  std::cout << _currentDir << std::endl;
}

ChangeDirCommand::ChangeDirCommand(string cmd_line, vector<string> &args) : BuiltInCommand(cmd_line), _dir("")
{
  if (args.size() == 1)
  {
    throw InvalidArguments(cmd_line);
  }
  else if (args.size() == 2)
  {
    string temp;
    if (args[1] == "-")
    {
      if (SmallShell::getInstance().getPreDir() == "")
      {
        throw OldPwdNotSet(args[0]);
      }
      else
      {
        temp = SmallShell::getInstance().getPreDir();
      }
    }
    else
    {
      temp = args[1];
    }
    _dir = temp;
  }
  else
  {
    throw TooManyArguments(args[0]);
  }
}
void ChangeDirCommand::execute()
{
  if (chdir(_dir.c_str()) == 0)
  {
    SmallShell::getInstance().goToDir(_dir);
  }
  else
  {
    throw DirDoesNotExist();
  }
}

JobsCommand::JobsCommand(string cmd_line, vector<string> &args) : BuiltInCommand(cmd_line) {}

void JobsCommand::execute()
{
  SmallShell::getInstance().printJobsList();
}

ForegroundCommand::ForegroundCommand(string cmd_line, vector<string> &args) : BuiltInCommand(cmd_line), _job(nullptr), _job_id(0)
{
  if (args.size() == 1)
  {
    if (SmallShell::getInstance().jobsListIsEmpty())
    {
      throw JobsListIsEmpty(args[0]);
    }
    _job = SmallShell::getInstance().getLastJobId(&_job_id);
  }
  else if (args.size() > 2)
  {
    throw InvalidArguments(cmd_line);
  }
  else
  {
    for (int i = 0; i < args[1].size(); i++)
    {
      if (!isdigit(args[1][i]))
      {
        throw InvalidArguments(cmd_line);
      }
      _job_id = stoi(args[1]);
    }
    _job = SmallShell::getInstance().getJobById(_job_id);
  }
}

void ForegroundCommand::execute()
{
  if (_job == nullptr)
  {
    throw JobDoesNotExist(string("fg"), _job_id);
  }
  else
  {
    if (_job->isStopped())
    {
      _job->setStopped(false);
      kill(_job->getPid(), SIGCONT);
    }
    cout << _job->getCommand() << " : " << _job->getPid() << endl;
    SmallShell::getInstance().setCurrentCmd(_job->getCmd());
    SmallShell::getInstance().setCurrentCmdPid(_job->getPid());
    SmallShell::getInstance().removeJobById(_job_id);
    waitpid(_job->getPid(), NULL, WUNTRACED);
    SmallShell::getInstance().setCurrentCmdPid(-1);
    SmallShell::getInstance().setCurrentCmd(nullptr);
  }
}

BackgroundCommand::BackgroundCommand(string cmd_line, vector<string> &args) : BuiltInCommand(cmd_line)
{

  if (args.size() == 1)
  {
    _job = SmallShell::getInstance().getLastStoppedJob(&(_job_id));
    if (!_job)
    {
      throw NoStopedJobs(args[0]);
    }
  }
  else if (args.size() == 2)
  {
    for (int i = 0; i < args[1].size(); i++)
    {
      if (!isdigit(args[1][i]))
      {
        throw InvalidArguments(cmd_line);
      }
    }
    _job_id = stoi(args[1]);
    _job = SmallShell::getInstance().getJobById(_job_id);
    if (!_job)
    {
      throw JobDoesNotExist(args[0], _job_id);
    }
    else if (!_job->isStopped())
    {
      throw AlreadyRunningInBackground(args[0], _job_id);
    }
  }
  if (args.size() > 2)
  {
    throw DefaultError(cmd_line);
  }
}

void BackgroundCommand::execute()
{
  std::cout << _cmd_line << ":" << to_string(_job->getPid()) << endl;
  _job->setStopped(false);
  kill(_job->getPid(), SIGCONT);
}

QuitCommand::QuitCommand(string cmd_line, vector<string> &args) : BuiltInCommand(cmd_line), _kill(false)
{
  if (args.size() > 1)
  {
    if (args[1] == "kill")
    {
      _kill = true;
    }
  }
}

void QuitCommand::execute()
{
  if (_kill)
  {
    SmallShell::getInstance().killAllJobs();
  }
  exit(0);
}

ExternalCommand::ExternalCommand(const char *cmd_line, string cmd, vector<string> args, bool is_background) : Command(cmd),
                                                                                                              _cmd_line(cmd_line),
                                                                                                              _cmd(cmd), _args(args),
                                                                                                              _is_background(is_background) {}

void ExternalCommand::execute()
{
  pid_t pid = fork();
  if (pid == 0)
  {
    setpgrp();
    bool has_wild_card = false;
    for (int i = 0; i < _args.size(); ++i)
    {
      if (_args[i].find('*') != string::npos || _args[i].find('?') != string::npos)
      {
        has_wild_card = true;
        break;
      }
    }
    if (has_wild_card)
    {
      // Todo: implement wild card
      string bash_cmd = "/bin/bash";
      if (execl(bash_cmd.c_str(), bash_cmd.c_str(), "-c", _cmd.c_str(), NULL) == -1)
      {
        perror("smash error: execv failed");
        exit(1);
      }
    }
    else
    {
      char **args = new char *[_args.size() + 1];
      string cmd_name = _args[0].substr(_args[0].find_last_of('/') + 1);
      args[0] = new char[cmd_name.size() + 1];
      strcpy(args[0], cmd_name.c_str());
      for (int i = 1; i < _args.size(); ++i)
      {
        args[i] = new char[_args[i].size() + 1];
        strcpy(args[i], _args[i].c_str());
      }
      args[_args.size()] = NULL;
      execv(_args[0].c_str(), args);

      string bin_cmd = "/bin/" + cmd_name;
      execv(bin_cmd.c_str(), args);

      perror("smash error: >");
      exit(1);
    }
  }

  else
  {
    if (pid == -1)
    {
      perror("smash error: fork failed");
      return;
    }
    this->_pid = pid;
    if (_is_background)
    {
      SmallShell::getInstance().addJob(this, false);
    }
    else
    {
      SmallShell::getInstance().setCurrentCmd(this);
      SmallShell::getInstance().setCurrentCmdPid(this->getPid());
      waitpid(pid, NULL, WUNTRACED);
      SmallShell::getInstance().setCurrentCmd(nullptr);
      SmallShell::getInstance().setCurrentCmdPid(-1);
    }
  }
}

// PipeCommand::PipeCommand(const char *cmd_line, string cmd, vector<string> args ,vector<string> args1 ,vector<string> args2): Command(cmd),
//                                                                                                                             _cmd_line(cmd_line),
//                                                                                                                             _cmd(cmd),
//                                                                                                                             _args(args),
//                                                                                                                             _args1(args),
//                                                                                                                             _args2(args){}

// void PipeCommand::execute()
// {
//   int fd[2];

//   pipe(fd);
//   if (fork() == 0)
//   {
//     close(fd[0]);
//   }
//   close(fd[1]);
// }
