#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <memory>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

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

JobsList::JobEntry::JobEntry(int job_id, pid_t pid, const string cmd_line, bool isStopped) : _job_id(job_id),
                                                                                             _command(cmd_line),
                                                                                             _pid(pid),
                                                                                             _isStopped(isStopped),
                                                                                             _start_time(time(NULL))
{
}

void JobsList::addJob(Command *cmd, bool isStopped)
{
  removeFinishedJobs();
  int job_id = 1;
  if (!_jobs.empty())
  {
    job_id = _jobs.end()->first + 1;
  }
  JobEntry newJobEntry = JobEntry(job_id, cmd->getPid(), cmd->getCmdLine(), isStopped);
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
  *lastJobId = _jobs.end()->first;
  return &(_jobs.end()->second);
}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId)
{
  if (_jobs.empty())
  {
    return nullptr;
  }
  for (auto it = _jobs.end(); it != _jobs.begin();)
  {
    if (it->second.isStopped())
    {
      *jobId = it->first;
      return &(it->second);
    }
  }
}

SmallShell::SmallShell() : _prompt("smash"), _preDir(""), _currentDir("")
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
shared_ptr<Command> SmallShell::CreateCommand(const char *cmd_line)
{
  bool inBackground = _isBackgroundComamnd(cmd_line);
  shared_ptr<char> cmd_cpy(new char[strlen(cmd_line) + 1]);
  strcpy(cmd_cpy.get(), cmd_line);
  _removeBackgroundSign(cmd_cpy.get());
  vector<string> args = convertToVector(cmd_cpy.get());
  string cmd = cmd_cpy.get();
  if (args.size() == 0)
  {
    return nullptr;
  }
  string commend = args[0];
  if (commend == "chprompt")
  {
    return make_shared<ChpromptCommand>(cmd, args);
  }
  else if (commend == "showpid")
  {
    return make_shared<ShowPidCommand>(cmd, args);
  }
  else if (commend == "pwd")
  {
    return make_shared<GetCurrDirCommand>(cmd, args);
  }
  else if (commend == "cd")
  {
    return make_shared<ChangeDirCommand>(cmd, args);
  }
  else if (commend == "jobs")
  {
    return make_shared<JobsCommand>(cmd, args);
  }
  else if (commend == "fg")
  {
    return make_shared<ForegroundCommand>(cmd, args);
  }
  else if (commend == "bg")
  {
    return make_shared<BackgroundCommand>(cmd, args);
  }
  else if (commend == "quit")
  {
    return make_shared<QuitCommand>(cmd, args);
  }
  else if (commend == "kill")
  {
    return make_shared<KillCommand>(cmd, args);
  }
  // else
  // {
  //   return make_shared(ExternalCommand(cmd_cpy));
  // }

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
  shared_ptr<Command> cmd = CreateCommand(cmd_line);
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
  std::cout << int(_newPid);
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
    throw TooFewArguments(cmd_line);
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
    throw InvalidArguments(args[0]);
  }
  else
  {
    for (int i = 0; i < args[1].size(); i++)
    {
      if (!isdigit(args[1][i]))
      {
        throw InvalidArguments(args[0]);
      }
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
    waitpid(_job->getPid(), NULL, SIGCONT);
    SmallShell::getInstance().removeJobById(_job_id);
  }
  cout << _job->getCommand() << " : " << _job->getPid() << endl;
}

BackgroundCommand::BackgroundCommand(string cmd_line, vector<string> &args) : BuiltInCommand(cmd_line)
{

  if (args.size() == 1)
  {
    _job = SmallShell::getInstance().getLastStoppedJob();
    _job_id = SmallShell::getInstance().getLastStoppedJob()->first;
    if (!_job)
    {
      throw // there is no stop jobs to resume
    }
  }
  else if (args.size() == 2)
  {
    for (int i = 0; i < args[1].size(); i++)
    {
      if (!isdigit(args[1][i]))
      {
        throw InvalidArguments(args[0]);
      }
    }
    _job_id = stoi(args[1]);
    _job = SmallShell::getInstance().removeJobById(_job_id);
    if (!_job)
    {
      throw JobDoesNotExist(args[0], _job_id)
    }
    else if (!_job->isStopped)
    {
      throw AlreadyRunningInBackground(args[0], _job_id);
    }
  }
  if (args.size() > 2)
  {
    throw InvalidArguments(args[0]);
  }
}

void BackgroundCommand::execute()
{
  std::cout << _cmd_line << ":" << to_string(_job->getPid()) << endl;
  kill(_job->getPid(), SIGCONT);
}

quitCommand::quitCommand(string cmd_line, vector<string> &args) : BuiltInCommand(cmd_line), _kill(false)
{
  if (args.size() > 1)
  {
    if (args[1] == "kill")
    {
      _kill = true;
    }
  }
}

void quitCommand::execute()
{
  if (_kill)
  {
    SmallShell::getInstance().killAllJobs();
  }
  exit(0);
}
