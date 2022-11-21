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

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell() : _prompt("smash")
{
  // TODO: add your implementation
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
  vector<string> args = convertToVector(cmd_line);
  if (args.size() == 0)
  {
    return nullptr;
  }
  string commend = args[0];

  if (commend == "chprompt")
  {
    return make_shared<ChpromptCommand>(args);
  }
  else if (commend == "showpid")
  {
    return make_shared<ShowPidCommand>(args);
  }
  else if (commend == "pwd")
  {
    return make_shared<PwdCommand>(args);
  }
  else if (commend == "cd")
  {
    return make_shared<CdCommand>(args);
  }
  else if (commend == "jobs")
  {
    return make_shared<JobsCommand>(args);
  }
  else if (commend == "fg")
  {
    return make_shared<FgCommand>(args);
  }
  else if (commend == "bg")
  {
    return make_shared<BackgroundCommand>(args);
  }
  else if (commend == "quit")
  {
    return make_shared<QuitCommand>(args);
  }
  else if (commend == "kill")
  {
    return make_shared<KillCommand>(args);
  }
  else
  {
    return make_shared(ExternalCommand(cmd_line));
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

ChpromptCommand::ChpromptCommand(vector<string> args)
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

ChpromptCommand::execute()
{
  SmallShell::getInstance().setPrompt(_newPrompt);
}
ShowPidCommand::ShowPidCommand(vector<string> args)
{
  _newPid = getpid();
}
ShowPidCommand::execute()
{
  std::cout << int(_newPid);
}
