#ifndef SH_H_
#define SH_H_

#include "get_path.h"

int pid;
int shell( int argc, char **argv, char **envp);
char *which(char *command, pathelement_t *pathlist);
char *where(char *command, pathelement_t *pathlist);
char **input_to_array(char **args, char *input);
void printenv(char **envp);
void change_prompt(char **args, char *prompt_pre);
void change_p(char **args, char *prompt_pre);
void print_pid();
void print_clist(char **args);
void print_glist(char **args);
void print_cwd();
void change_dir(char **args, char *owd);
void change_hdir(char *homedir);
void print_env(char **envp);
void print_env_v(char **args, char **envp);
int shell_exec(char *cmd, char **args, char **envp, pathelement_t *pathlist);
void kill_p(char **args);
void kill_p2(char **args);


#define PROMPTMAX 32
#define MAXARGS 10

#endif /* SH_H_ */


