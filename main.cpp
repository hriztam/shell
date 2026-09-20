#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <cstdlib>
#include <unistd.h>

using namespace std;

vector<string> tokenize(const string &command)
{
  vector<string> tokens;
  string token;

  for (char character : command)
  {
    if (character == ' ')
    {
      if (!token.empty())
      {
        tokens.push_back(token);
        token.clear();
      }
    }
    else
    {
      token += character;
    }
  }

  if (!token.empty())
    tokens.push_back(token);

  return tokens;
}

string findExecutable(const string &command)
{
  const char *pathEnvironment = getenv("PATH");
  if (pathEnvironment == nullptr)
    return "";

  string path = pathEnvironment;
  size_t start = 0;

  while (start <= path.size())
  {
    size_t end = path.find(':', start);
    string directory = path.substr(start, end == string::npos ? end : end - start);

    if (directory.empty())
      directory = ".";

    string candidate = directory + '/' + command;
    if (access(candidate.c_str(), X_OK) == 0)
      return candidate;

    if (end == string::npos)
      break;

    start = end + 1;
  }

  return "";
}

void echo(const string &command)
{
  cout << command.substr(5) << endl;
}

void type(const vector<string> &tokens, const unordered_set<string> &builtins)
{
  if (tokens.size() == 1)
  {
    cout << "invalid syntax for this command\n";
    return;
  }

  const string &command = tokens[1];
  if (builtins.find(command) != builtins.end())
  {
    cout << command << " is a shell builtin\n";
    return;
  }

  string executable = findExecutable(command);
  if (!executable.empty())
  {
    cout << command << " is " << executable << endl;
    return;
  }

  cout << command << ": not found\n";
}

int main(int argc, char *argv[])
{
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  const unordered_set<string> builtins = {"exit", "echo", "type"};

  while (true)
  {
    std::cout << "$ ";
    string command;
    getline(std::cin, command);
    if (command == "exit")
      break;
    vector<string> tokens = tokenize(command);
    string first_token;
    if (!tokens.empty())
      first_token = tokens[0];

    if (first_token == "echo")
      echo(command);
    else if (first_token == "type")
      type(tokens, builtins);
    else
    {
      cout << command << ": command not found" << endl;
    }
  }
  return 0;
}
