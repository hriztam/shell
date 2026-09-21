#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>

std::vector<std::string> tokenize(const std::string &command)
{
  std::vector<std::string> tokens;
  std::string token;

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

std::string findExecutable(const std::string &command)
{
  const char *pathEnvironment = getenv("PATH");
  if (pathEnvironment == nullptr)
    return "";

  std::string path = pathEnvironment;
  std::size_t start = 0;

  while (start <= path.size())
  {
    std::size_t end = path.find(':', start);
    std::string directory = path.substr(start, end == std::string::npos ? end : end - start);

    if (directory.empty())
      directory = ".";

    std::string candidate = directory + '/' + command;
    if (access(candidate.c_str(), X_OK) == 0)
      return candidate;

    if (end == std::string::npos)
      break;

    start = end + 1;
  }

  return "";
}

void echo(const std::string &command)
{
  std::cout << command.substr(5) << std::endl;
}

void pwd()
{
  char currentDirectory[PATH_MAX];

  if (getcwd(currentDirectory, sizeof(currentDirectory)) == nullptr)
  {
    perror("pwd");
    return;
  }

  std::cout << currentDirectory << std::endl;
  
}

void type(const std::vector<std::string> &tokens, const std::unordered_set<std::string> &builtins)
{
  if (tokens.size() == 1)
  {
    std::cout << "invalid syntax for this command\n";
    return;
  }

  const std::string &command = tokens[1];
  if (builtins.find(command) != builtins.end())
  {
    std::cout << command << " is a shell builtin\n";
    return;
  }

  std::string executable = findExecutable(command);
  if (!executable.empty())
  {
    std::cout << command << " is " << executable << std::endl;
    return;
  }

  std::cout << command << ": not found\n";
}

void runExternalCommand(
  const std::string &executable,
  std::vector<std::string> &tokens)
{
  pid_t child = fork();

  if (child == 0)
  {
    std::vector<char *> arguments;

    for (std::string &token : tokens)
      arguments.push_back(&token[0]);

    arguments.push_back(nullptr);

    execv(executable.c_str(), arguments.data());

    perror("execv");
    exit(1);
  }

  if (child > 0)
  {
    waitpid(child, nullptr, 0);
  }
  else
  {
    perror("fork");
  }

}

int main(int argc, char *argv[])
{
  // Flush after every std::cout / std:cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;
  const std::unordered_set<std::string> builtins = {"exit", "echo", "type", "pwd"};

  while (true)
  {
    std::cout << "$ ";
    std::string command;
    getline(std::cin, command);
    if (command == "exit")
      break;
    std::vector<std::string> tokens = tokenize(command);
    std::string first_token;
    if (!tokens.empty())
      first_token = tokens[0];

    if (first_token == "echo")
      echo(command);
    else if (first_token == "pwd")
      pwd();
    else if (first_token == "type")
      type(tokens, builtins);
    else
    {
      std::string executable = findExecutable(first_token);

      if (executable.empty())
      {
        std::cout << command << ": command not found" << std::endl;
        continue;
      }

      runExternalCommand(executable, tokens);
      
      
    }
  }
  return 0;
}
