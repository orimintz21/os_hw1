#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <time.h>
using std::map;
using std::shared_ptr;
using std::string;
using std::to_string;
using std::vector;

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command
{
protected:
  // TODO: Add your data members
  pid_t _pid;
  string _cmd_line;

public:
  Command(pid_t pid, string cmd_line);
  Command(string _cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  virtual pid_t getPid();
  virtual string getCmdLine();
  // virtual void prepare();
  // virtual void cleanup();
  //  TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command
{
public:
  BuiltInCommand(string cmd_line);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command
{
public:
  ExternalCommand(const char *cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command
{
  // TODO: Add your data members
public:
  PipeCommand(const char *cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class ChpromptCommand : public BuiltInCommand
{
private:
  string _newPrompt;

public:
  ChpromptCommand(string cmd_line, vector<string> &args);
  explicit ChpromptCommand(vector<string> args);
  virtual ~ChpromptCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command
{
  // TODO: Add your data members
public:
  explicit RedirectionCommand(const char *cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  // void prepare() override;
  // void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand
{
  // TODO: Add your data members public:
  string _dir;
  ChangeDirCommand(string cmd_line, vector<string> &args);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand
{
private:
  string _currentDir;

public:
  GetCurrDirCommand(string cmd_line, vector<string> &args);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand
{
public:
  pid_t _newPid;
  ShowPidCommand(string cmd_line, vector<string> &args);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand
{
  // TODO: Add your data members
  string _cmd_line;
  bool killAll;

public:
  QuitCommand(string cmd_line, vector<string> &args);
  virtual ~QuitCommand() {}
  void execute() override;
};

class JobsList
{
public:
  class JobEntry
  {
    // TODO: Add your data members
    int _job_id;
    pid_t _pid;
    string _command;
    bool _isStopped;
    time_t _start_time;

  public:
    JobEntry(int job_id, pid_t pid, const string command, bool isStopped);
    int getJobId() const { return _job_id; }
    pid_t getPid() const { return _pid; }
    string getCommand() const { return _command; }
    bool isStopped() const { return _isStopped; }
    time_t getStartTime() const { return _start_time; }
  };
  // TODO: Add your data members
  map<int, JobEntry> _jobs;

public:
  JobsList();
  ~JobsList();
  void addJob(Command *cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry *getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry *getLastJob(int *lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand
{
  // TODO: Add your data members
public:
  JobsCommand(string cmd_line, vector<string> &args);
  virtual ~JobsCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand
{
  // TODO: Add your data members
  JobsList::JobEntry *_job;
  int _job_id;

public:
  ForegroundCommand(string cmd_line, vector<string> &args);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand
{
  JobsList::JobEntry *_job;
  int _job_id;
  // TODO: Add your data members
public:
  BackgroundCommand(string cmd_line, vector<string> &args);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class TimeoutCommand : public BuiltInCommand
{
  /* Optional */
  // TODO: Add your data members
public:
  explicit TimeoutCommand(const char *cmd_line);
  virtual ~TimeoutCommand() {}
  void execute() override;
};

class FareCommand : public BuiltInCommand
{
  /* Optional */
  // TODO: Add your data members
public:
  FareCommand(const char *cmd_line);
  virtual ~FareCommand() {}
  void execute() override;
};

class SetcoreCommand : public BuiltInCommand
{
  /* Optional */
  // TODO: Add your data members
public:
  SetcoreCommand(const char *cmd_line);
  virtual ~SetcoreCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand
{
  /* Bonus */
  // TODO: Add your data members
public:
  KillCommand(const char *cmd_line, JobsList *jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class SmallShell
{
private:
  // TODO: Add your data members
  string _prompt;
  string _preDir;
  string _currentDir;
  JobsList _jobsList;
  SmallShell();

public:
  shared_ptr<Command> CreateCommand(const char *cmd_line);
  SmallShell(SmallShell const &) = delete;     // disable copy ctor
  void operator=(SmallShell const &) = delete; // disable = operator
  static SmallShell &getInstance()             // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char *cmd_line);
  // TODO: add extra methods as needed
  string &getPrompt();
  string &getPreDir();
  string &getCurrentDir();
  void setPreDir(string &dir);
  void setCurrentDir(string &dir);
  void setPrompt(string newPrompt);
  vector<string> convertToVector(const char *cmd_line);
  void goToDir(string &dir);
  void printJobsList() { _jobsList.printJobsList(); }
  bool jobsListIsEmpty() const { return _jobsList._jobs.empty(); }
  JobsList::JobEntry *getLastJobId(int *job_id);
  JobsList::JobEntry *getJobById(int job_id);
  void removeJobById(int job_id) { _jobsList.removeJobById(job_id); }
  void killAllJobs() { _jobsList.killAllJobs(); }
};

class CommandException : public std::exception
{
};
class TooManyArguments : public CommandException
{
private:
  string _cmd_line;

public:
  TooManyArguments(string &cmd) : _cmd_line(cmd) {}

  const char *what() const noexcept
  {
    string ans = "smash error: " + _cmd_line + ": too many arguments";
    return ans.c_str();
  }
};

class TooFewArguments : public CommandException
{
private:
  string _cmd_line;

public:
  TooFewArguments(string &cmd) : _cmd_line(cmd) {}
  const char *what() const noexcept
  {
    string ans = "smash error:> " + _cmd_line;
    return ans.c_str();
  }
};

class OldPwdNotSet : public CommandException
{
private:
  string _cmd_line;

public:
  OldPwdNotSet(string &cmd) : _cmd_line(cmd) {}
  const char *what() const noexcept
  {
    string ans = "smash error: " + _cmd_line + ": OLDPWD not set";
    return ans.c_str();
  }
};

class DirDoesNotExist : public CommandException
{
};

class JobsListIsEmpty : public CommandException
{
private:
  string _cmd_line;

public:
  JobsListIsEmpty(string &cmd) : _cmd_line(cmd) {}
  const char *what() const noexcept
  {
    string ans = "smash error: " + _cmd_line + ": jobs list is empty";
    return ans.c_str();
  }
};

class InvalidArguments : public CommandException
{
private:
  string _cmd_line;

public:
  InvalidArguments(string &cmd) : _cmd_line(cmd) {}
  const char *what() const noexcept
  {
    string ans = "smash error: " + _cmd_line + ": invalid arguments";
    return ans.c_str();
  }
};

class JobDoesNotExist : public CommandException
{
private:
  string _cmd_line;
  int _job_id;

public:
  JobDoesNotExist(string cmd, int job_id) : _cmd_line(cmd), _job_id(job_id) {}
  const char *what() const noexcept
  {
    string ans = "smash error: " + _cmd_line + ": job-id " + to_string(_job_id) + " does not exist";
    return ans.c_str();
  }
};

class AlreadyRunningInBackground : public CommandException
{
private:
  string _cmd_line;
  int _job_id;

public:
  AlreadyRunningInBackground(string cmd, int job_id) : _cmd_line(cmd), _job_id(job_id) {}

  const char *what() const noexcept
  {
    string ans = "smash error: " + _cmd_line + ": job-id " + to_string(_job_id) + " is already running in the background";
    return ans.c_str();
  }
};

#endif // SMASH_COMMAND_H_
