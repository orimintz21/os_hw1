#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <time.h>
#include <iostream>
using std::cerr;
using std::endl;
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
  virtual ~Command() = default;
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
private:
  const char *_cmd_line;
  string _cmd;
  vector<string> _args;
  bool _is_background;

public:
  ExternalCommand(const char *cmd_line, string cmd, vector<string> args, bool is_background);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command
{
  string _cmd;
  vector<string> _args1;
  string _cmd1;
  vector<string> _args2;
  string _cmd2;
  bool _is_err;

public:
  PipeCommand(string &cmd, vector<string> &args1, string &cmd1, vector<string> args2, string &cmd2, bool is_err);
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
  vector<string> _args1;
  string _cmd1;
  vector<string> _args2;
  bool _append;
  string _file_name;

public:
  explicit RedirectionCommand(string &cmd, vector<string> &args1, string &cmd1, vector<string> &args2, bool append);
  virtual ~RedirectionCommand() {}
  void execute() override;
  // void prepare() override;
  // void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand
{
  // TODO: Add your data members public:
  string _dir;
  vector<string> _args;

public:
  ChangeDirCommand(string cmd_line, vector<string> &args);
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
  bool _kill;

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
    Command *_cmd;

  public:
    JobEntry(int job_id, pid_t pid, const string command, bool isStopped, Command *cmd);
    int getJobId() const { return _job_id; }
    pid_t getPid() const { return _pid; }
    string getCommand() const { return _command; }
    Command *getCmd() { return _cmd; }
    void setCmd(Command *cmd) { _cmd = cmd; }
    bool isStopped() const { return _isStopped; }
    void setStopped(bool isStopped) { _isStopped = isStopped; }
    time_t getStartTime() const { return _start_time; }
  };
  // TODO: Add your data members
  map<int, JobEntry> _jobs;

public:
  JobsList() = default;
  ~JobsList() = default;
  void addJob(Command *cmd, bool isStopped = false);
  void addJobWithId(Command *cmd, int id, bool isStopped = false);
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
  string _cmd;
  vector<string> _args;
  string _file_name;
  string _source;
  string _destination;

public:
  FareCommand(string &cmd, vector<string> &args);
  virtual ~FareCommand() {}
  void execute() override;
};

class SetcoreCommand : public BuiltInCommand
{
  /* Optional */
  // TODO: Add your data members
  string _cmd;
  vector<string> _args;
  int _core_num;
  int _job_id;

public:
  SetcoreCommand(string &cmd, vector<string> &args);
  virtual ~SetcoreCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand
{
  /* Bonus */
  // TODO: Add your data members
  string _cmd;
  int _sig_num;
  int _job_id;

public:
  KillCommand(string &cmd, vector<string> &args);
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
  pid_t _current_cmd_pid;
  Command *_current_cmd;
  int _current_job_id;
  SmallShell();

public:
  Command *CreateCommand(const char *cmd_line);
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
  void setPreDir(string &dir) { _preDir = dir; }
  void setCurrentDir(string &dir);
  void setPrompt(string newPrompt) { _prompt = newPrompt; }
  vector<string> convertToVector(const char *cmd_line);
  void goToDir(string &dir);
  void printJobsList() { _jobsList.printJobsList(); }
  bool jobsListIsEmpty() const { return _jobsList._jobs.empty(); }
  JobsList::JobEntry *getLastJobId(int *job_id);
  JobsList::JobEntry *getJobById(int job_id);
  JobsList::JobEntry *getLastStoppedJob(int *job_id) { return _jobsList.getLastStoppedJob(job_id); }
  void removeJobById(int job_id) { _jobsList.removeJobById(job_id); }
  void killAllJobs() { _jobsList.killAllJobs(); }
  void addJob(Command *cmd, bool isStopped = false) { _jobsList.addJob(cmd, isStopped); }
  void addJobWihId(Command *cmd, int id, bool isStopped = false) { _jobsList.addJobWithId(cmd, id, isStopped); }
  string setFullCmd(string &cmd);
  void sendControlC();
  int getCurrentCmdPid() const { return _current_cmd_pid; }
  void setCurrentCmdPid(int cmd_pid) { _current_cmd_pid = cmd_pid; }
  Command *getCurrentCmd() const { return _current_cmd; }
  void setCurrentJobId(int job_id) { _current_job_id = job_id; }
  int getCurrentJobId() const { return _current_job_id; }
  void setCurrentCmd(Command *cmd) { _current_cmd = cmd; }
  bool isRedirectionCommand(string &cmd, int &index, bool &is_append);
  bool isPipeCommand(string &cmd, int &index, bool &is_err);
  void splitRedirectionCommand(string &cmd, vector<string> &args1, vector<string> &args2, int index, bool is_append);
};

class CommandException : public std::exception
{
};
class TooManyArguments : public CommandException
{
private:
  string _cmd_line;

public:
  TooManyArguments(string &cmd) : _cmd_line(cmd)
  {
    cerr << "smash error: " + _cmd_line + ": too many arguments" << endl;
  }
};

class OldPwdNotSet : public CommandException
{
private:
  string _cmd_line;

public:
  OldPwdNotSet(string &cmd) : _cmd_line(cmd)
  {
    cerr << "smash error: " + _cmd_line + ": OLDPWD not set" << endl;
  }
};

class DefaultError : public CommandException
{
private:
  string _cmd_line;

public:
  DefaultError(string &cmd) : _cmd_line(cmd)
  {
    cerr << "smash error: " + _cmd_line << endl;
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
  JobsListIsEmpty(string &cmd) : _cmd_line(cmd)
  {
    cerr << "smash error: " + _cmd_line + ": jobs list is empty" << endl;
  }
};

class InvalidArguments : public CommandException
{
private:
  string _cmd_line;

public:
  InvalidArguments(string &cmd) : _cmd_line(cmd)
  {
    cerr << "smash error: " + _cmd_line + ": invalid arguments" << endl;
  }
};

class JobDoesNotExist : public CommandException
{
private:
  string _cmd_line;
  int _job_id;

public:
  JobDoesNotExist(string cmd, int job_id) : _cmd_line(cmd), _job_id(job_id)
  {
    cerr << "smash error: " + _cmd_line + ": job-id " + to_string(_job_id) + " does not exist" << endl;
  }
};

class AlreadyRunningInBackground : public CommandException
{
private:
  string _cmd_line;
  int _job_id;

public:
  AlreadyRunningInBackground(string cmd, int job_id) : _cmd_line(cmd), _job_id(job_id)
  {
    cerr << "smash error: " + _cmd_line + ": job-id " + to_string(_job_id) + " is already running in the background" << endl;
  }
};

class NoStopedJobs : public CommandException
{
private:
  string _cmd_line;

public:
  NoStopedJobs(string &cmd) : _cmd_line(cmd)
  {
    cerr << "smash error: " + _cmd_line + ": there is no stoped jobs to resume" << endl;
  }
};

class InvalidCoreNumber : public CommandException
{
private:
  string _cmd_line;

public:
  InvalidCoreNumber(string &cmd) : _cmd_line(cmd)
  {
    cerr << "smash error: " + _cmd_line + ": invalid core number" << endl;
  }
};
#endif // SMASH_COMMAND_H_
