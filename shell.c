#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "parse.c"

// max characters per command a user can input
#define MAX_BUFFER 80

// handles Ctrl+C, quits all processes
void sigint_handler(int sig){
  if(sig == SIGINT){
    printf("\nMINI SHELL TERMINATED\n");
    exit(0);
  }
}

// reads what the user inputs
void get_user_input(char buffer[]){
  fgets(buffer, MAX_BUFFER, stdin);
}

// built in cd
int cd(char* path){
  chdir(path);
  return 0;
}

// built in help
int help(){
  printf("Built-in commands in my Mini Shell:\n");
  printf("exit:      Exits the mini shell.\n");
  printf("cd:        Navigate through directories.\n");
  printf("help:      Displays how to use the built in commands.\n");
  printf("history:   Displays all of the commands the user has inputted.\n");
  printf("**NOTE: all of the /bin commands can be called**\n");
  return 0;
}

// gets the valid commands that the user has entered
int history(char** commands){
  int i = 0;
  int i_visual = 1;
  while(commands[i] != NULL){
    printf("%d: %s\n", i_visual, commands[i]);
    i++;
    i_visual++;
  }
  return 0;
}

// executes a non-piped command
int execute(char** tokens, char** commands, int num_commands){
  if(tokens[0] == NULL){
    printf("Nothing was entered.\n");
  }else if(strcmp(tokens[0],"exit")==0){
    printf("Mini shell exited.\n");
    free(commands);
    exit(1);
  }else if(strcmp(tokens[0],"cd")==0){
    if(tokens[1]==NULL){
      printf("Directory required.\n");
      return -1;
    }else if(tokens[2] != NULL){
      printf("There shouldn't be more than 1 argument.\n");
      return -1;
    }else{
      return cd(tokens[1]);
    }
  }else if(strcmp(tokens[0],"help")==0){
    if(tokens[1] != NULL){
      printf("There shouldn't be any arguments.\n");
      return -1;
    }else{
      return help();
    }
  }else if(strcmp(tokens[0],"history")==0){
    if(tokens[1] != NULL){
      printf("There shouldn't be any arguments.\n");
      return -1;
    }else{
      return history(commands);
    }
  } 
  if(fork() == 0){
    execvp(tokens[0],tokens);
    printf("Command not found.\n");
    exit(1);
  }else{wait(NULL);}
  return -1;
}

// returns the index of the pipe in the command, 0 if none
int has_pipe(char** tokens){
  int i = 1;
  while(tokens[i] != NULL){
    if(strcmp(tokens[i],"|")==0){
      return i;
    }else{
      i++;
    }
  }
  return 0;
}

// executes a command with a pipe
int execute_pipe(char** tokens, int index){
  // args before the pipe
  char** argv1 = (char**)malloc(MAX_BUFFER * sizeof(char*));
  // args after the pipe
  char** argv2 = (char**)malloc(MAX_BUFFER * sizeof(char*));
  // i is the index of the tokens, i1 is the index of argv1, i2 is the index of argv2
  int i = 0;
  int i1 = 0;
  int i2 = 0;
  // build the separate args for before and after the pipe
  while(tokens[i] != NULL){
    if(i < index){
      argv1[i1] = (char*)malloc(MAX_BUFFER * sizeof(char));
      strcpy(argv1[i1],tokens[i]);
      i1++;
    }else if(i > index){
      argv2[i2] = (char*)malloc(MAX_BUFFER * sizeof(char));
      strcpy(argv2[i2], tokens[i]);
      i2++;
    }
    i++;
  }
  int pipefd[2];
  pipe(pipefd);
  if(fork() == 0){
    close(pipefd[0]);
    dup2(pipefd[1], 1);
    close(pipefd[1]);
    execvp(argv1[0],argv1);
  }else{
    close(pipefd[1]);
    dup2(pipefd[0], 0);
    close(pipefd[0]);
    execvp(argv2[0],argv2);
  }
  printf("Command not found.\n");
  return -1;
}

int main(int argc, char** argv){

  char** commands = (char**)malloc(1000 * sizeof(char*));
  int num_commands = 0;
  
  while(1){
    //print the shell prompter
    printf("mini-shell>");
    // call signal handler
    signal(SIGINT, sigint_handler);
    // read command here
    char user_input[MAX_BUFFER];
    get_user_input(user_input);
    // add commands to history
    commands[num_commands] = strdup(user_input);
    num_commands++;
    // parse command, comes from part 1 of hw
    char* tokens[MAX_BUFFER];
    lexer_parser(user_input, tokens);
    // check for pipe
    if(has_pipe(tokens)>0){
      if(fork()==0){
        execute_pipe(tokens, has_pipe(tokens));
        exit(1);
      }else{wait(NULL);}
    }else{
      execute(tokens, commands, num_commands);
    }
  }
  
  return 0;
}
