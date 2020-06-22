#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "shell.h"

int counter = 0;

int shell( int argc, char **argv, char **envp ) {
  char *prompt = calloc(PROMPTMAX+1, sizeof(char));
  char *prompt_pre = calloc(PROMPTMAX+1, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd;
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  pathelement_t *pathlist;

  uid = getuid();
  password_entry = getpwuid(uid);               /* get passwd info */
  homedir = password_entry->pw_dir;             /* Home directory to start
                                                  out with*/
  if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
  {
    perror("getcwd");
    exit(2);
  }
  owd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(owd));

  /* Put PATH into a linked list */
  pathlist = get_path();

  while(go) {

    char **args = calloc(MAXARGS, sizeof(char*));

    prompt = getcwd(prompt, PROMPTMAX);
    
    printf("%s [%s]> ", prompt_pre, prompt);

    fgets(commandline, MAX_CANON, stdin);

    args = input_to_array(args, commandline);

    if(strlen(args[0]) <= 1){
      for(int i = 0; i < counter; i++){
        free(args[i]);
      }
      free(args);
      continue;
    }else if(strcmp(args[0], "which") == 0 || strcmp(args[0], "which\n") == 0){
      printf("Executing built-in [which]\n");
      char *x = which(args[1], pathlist);
      free(x);
    }else if(strcmp(args[0], "where") == 0 || strcmp(args[0], "where\n") == 0){
      printf("Executing built-in [where]\n");
      where(args[1], pathlist);
    }else if(strcmp(args[0], "prompt\n") == 0){
      printf("Executing built-in [prompt]\n");
      change_prompt(args, prompt_pre);
    }else if(strcmp(args[0], "prompt") == 0){
      printf("Executing built-in [prompt]\n");
      change_p(args, prompt_pre);
    }else if(strcmp(args[0], "pid\n") == 0){
      printf("Executing built-in [pid]\n");
      print_pid();
    }else if(strcmp(args[0], "list\n") == 0){
      printf("Executing built-in [list]\n");
      print_clist(args);
    }else if(strcmp(args[0], "list") == 0){
      printf("Executing built-in [list]\n");
      print_glist(args);
    }else if(strcmp(args[0], "pwd\n") == 0){
      printf("Executing built-in [pwd]\n");
      print_cwd();
    }else if(strcmp(args[0], "cd") == 0){
      printf("Executing built-in [cd]\n");
      change_dir(args, owd);
    }else if(strcmp(args[0], "cd\n") == 0){
      printf("Executing built-in [cd]\n");
      change_hdir(homedir);
    }else if(strcmp(args[0], "printenv\n") == 0){
      printf("Executing built-in [printenv]\n");
      print_env(envp);
    }else if(strcmp(args[0], "printenv") == 0){
      printf("Executing built-in [printenv]\n");
      print_env_v(args, envp);
    }else if(strcmp(args[0], "kill") == 0 && args[2] == NULL){
      printf("Executing built-in [kill]\n");
      kill_p(args);
    }else if(strcmp(args[0], "kill") == 0 && args[2] != NULL){
      kill_p2(args);
    }else if(strcmp(args[0], "exit\n") == 0){ /* free all allocated mem */
      printf("Executing built-in [exit]\n");
      for(int i = 0; i < counter; i++){
        free(args[i]);
      }
      free(args);
      free(prompt);
      free(prompt_pre);
      free(commandline);
      free(owd);
      pathelement_t *cur = NULL;
      while(pathlist != NULL){
        cur = pathlist;
        pathlist = pathlist->next;
        free(cur);
      }
      free(pathlist);
      free(pwd);
      break;
    }else{  // execve progrom
      if(args[0] != NULL && args[1] == NULL){
        int len = strlen(args[0]);
        args[0][len-1] = '\0';
      }
      if(args[1] != NULL && args[2] == NULL){
        int len = strlen(args[1]);
        args[1][len-1] = '\0';
      }
      if(args[2] != NULL && args[3] == NULL){
        int len = strlen(args[1]);
        args[2][len-1] = '\0';
      }
      if(args[3] != NULL && args[4] == NULL){
        int len = strlen(args[1]);
        args[3][len-1] = '\0';
      }
     
      char *ex = which(args[0], pathlist);
      if(args[0][0] == '/' && access(args[0], X_OK) == 0){
        printf("Executing [%s]\n", args[0]);
        shell_exec(args[0], args, envp, pathlist);
      }else if(ex != NULL){
	printf("Executing [%s]\n", args[0]);
        shell_exec(ex, args, envp, pathlist);
      }else{
        fprintf(stderr, "%s: Command not found.\n", args[0]);
      }
      free(ex);
    }

    for(int i = 0; i < counter; i++){
        free(args[i]);
    }
    free(args);
    continue;
  }
  return 0;
} /* sh() */


/* locates and prints the executable of a given command from the path, allocates and returns char* command */
char *which(char *cmd, pathelement_t *p) {
  if(cmd == NULL){
    printf("which: Too few arguments\n");
    return NULL;
  }
  int length = strlen(cmd);
  if(cmd[length-1] == '\r' || cmd[length-1] == '\n'){
    cmd[length-1] = '\0';
  }
  char *command = calloc(64, sizeof(char));
  pathelement_t *path = p;
  
  while (path != NULL) {
    sprintf(command, "%s/%s", path->element, cmd);
    if (access(command, X_OK) == 0) {
      printf("[%s]\n", command);
      return command;
    }
    path = path->next;
  }
  perror("which");
  free(command);
  return NULL;
} /* which() */

/* prints all instances of command in the path */
char *where(char *cmd, pathelement_t *p) {
  if(cmd == NULL){
    printf("where: Too few arguments\n");
    return NULL;
  }
  int length = strlen(cmd);
  int count = 0;
  char *cmd2;
  cmd2 = cmd;
  char command[64];
  pathelement_t *path = p;
  cmd2[length-1] = '\0';

  while (path) { 
    sprintf(command, "%s/%s", path->element, cmd2);
    if (access(command, F_OK) == 0) {
      printf("[%s]\n", command);
      count++;
    }
    path = path->next;
  }
 
  if(count == 0){
    perror("where");
  }
  return cmd;
} /* where() */

/* splits command line into a char**, allocates mem for **argv  */
char **input_to_array(char **args, char *input){
  if (strlen(input) == 0) return NULL;
  int count = 0;
  char buf[strlen(input)]; 
  strcpy(buf, input);
  char *t = strtok(buf, " ");

  while(strtok(NULL, " ")){
    count++;
  }
  count++;
  counter = count;
  
  strcpy(buf, input);
  t = strtok(buf, " ");
  count = 0;

  while(t){
    int len = strlen(t);
    args[count] = calloc(len+1, sizeof(char));
    strcpy(args[count], t);
    count++;
    t = strtok(NULL, " ");
  }
  return args;
} /* input_to_array() */


/* changes the prompt prefix based on given phrase */
void change_prompt(char** args, char *prompt_pre){
  char temp[PROMPTMAX];

  printf("input prompt prefix: ");
  scanf("%s", temp);
  strcpy(prompt_pre, temp);
}

/* changes prompt based on given arg */
void change_p(char **args, char *prompt_pre){
  int len = strlen(args[1]);
  args[1][len-1] = '\0';
  strcpy(prompt_pre, args[1]);
}

/* prints the pid of the shell */
void print_pid(){
  printf("%ld\n", (long)getpid());
}

/* prints the files in the current directory */
void print_clist(char **args){ 
  struct dirent *de;
  DIR *dr = opendir(".");

  if(dr == NULL){
    printf("Could not open directory\n");
    return;
  }
  while((de = readdir(dr)) != NULL){
    printf("%s\n", de->d_name);
  }
  closedir(dr);
}

/* prints the files in the given directory */
void print_glist(char **args){
  int len = strlen(args[1]);
  args[1][len-1] = '\0';
  struct dirent *de;
  DIR *dr = opendir(args[1]);

  if(dr == NULL){
    printf("Could not open directory\n");
    return;
  }
  printf("%s:\n", args[1]);
  while((de = readdir(dr)) != NULL){
    printf("%s\n", de->d_name);
  }
  closedir(dr);
}

/* prints the current working directory */
void print_cwd(){
  char cwd[128];

  if(getcwd(cwd, sizeof(cwd)) != NULL){
    printf("Current working directory: %s\n", cwd);
  }else{
    perror("pwd");
  }
}

/* change directory to given or last directory */
void change_dir(char **args, char *owd){
  char back[6] = "-";

  if(strncmp(args[1], back, 1) != 0){
    int len = strlen(args[1]);
    args[1][len-1] = '\0';
    if(chdir(args[1]) != 0){
      perror("cd: ");
    }   
  }else{ // change to previous directory
    if(chdir(owd) != 0){
      perror("cd");
    }
  }
}

/* change directory to the home directory */
void change_hdir(char *homedir){	
  if(chdir(homedir) != 0){
    perror("cd");
  }
}

/* prints all environment variables */
void print_env(char **envp){
  for(char **env = envp; *env != 0; env++){
    char *this_env = *env;
    printf("%s\n", this_env);
  }
}

/* prints the given environment variable */
void print_env_v(char **args, char **envp){
  if(args[2] != NULL){
    perror("printenv error: ");
  }
  int len = strlen(args[1]);
  args[1][len-1] = '\0';
  if(getenv(args[1]) != NULL){
    printf("%s= %s\n", args[1], getenv(args[1]));
  }else{
    if(getenv(args[1]) == NULL){
      printf("printenv: variable not found\n");
    }
  }
}

/* executes the given executable, cmd */
int shell_exec(char *cmd, char **args, char **envp, pathelement_t *pathlist){
  pid_t pid, wpid;
  int status;
  pid = fork();

  if(pid == 0){ // child
    if(execve(cmd, args, envp) == -1){//args <-> arg_arr
      perror("execve failed");
    }
  }else if(pid < 0){ // error
    perror("execve failed");
  }else{ // parent
    do{
      wpid = waitpid(pid, &status, WUNTRACED);
    }while(!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return -1;
}

/* sends a sigterm to given pid */
void kill_p(char **args){
  int len = strlen(args[1]);
  args[1][len-1] = '\0';
  int id = atoi(args[1]);
  if(kill(id, SIGTERM) != 0){
    perror("kill");
  }
}

/* kill process with given signal */
void kill_p2(char **args){
  int sig = atoi(args[1]);
  int len = strlen(args[2]);
  args[2][len-1] = '\0';
  int id = atoi(args[2]);
  if(kill(id, sig) != 0){
    perror("kill");
  }
}

