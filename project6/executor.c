/* Michelle Lu, mlu34, 117800524 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sysexits.h>
#include <err.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "command.h"
#include "executor.h"

int execute_h(struct tree *t, int fd_in, int fd_out);
static void print_tree(struct tree *t);

/* parse the tree and execute its instructions */
int execute(struct tree *t) {
    
    execute_h(t, STDIN_FILENO, STDOUT_FILENO);
   
   /* print_tree(t); */ 
    return 0;
}

int execute_h(struct tree *t, int fd_in, int fd_out){
    int status, pipe_fd[2];
    pid_t pid;
    
    if(t != NULL){
        if(t->conjunction == NONE){ /* NONE: does not have right/left subtree */
            if(!strcmp(t->argv[0], "cd")){ /* the command is cd */
                if(t->argv[1] == NULL){ /* cd to HOME */
                    chdir(getenv("HOME"));
                    return 0;
                    
                }else{ /* cd to t->argv[1] */
                    if(chdir(t->argv[1]) < 0){ /* chdir failed */
                        perror(t->argv[1]);
                        exit(EX_OSERR);
                    }
                    return 0;
                }
            }else if(!strcmp(t->argv[0], "exit")){ /* the command is exit */
                exit(0);
                
            }else{
                if((pid = fork()) < 0){
                    perror("fork Error\n");
                    exit(EX_OSERR);
                }
                if(pid){ /* parent process */
                    wait(&status); /* reaps child process */
                    return status;
                    
                }else{ /* child process */
                    if(t->input){ /* input redirection */
                        if((fd_in = open(t->input, O_RDONLY)) < 0){
                            perror("open Error\n");
                            exit(EX_OSERR);
                        }
                        if(dup2(fd_in, STDIN_FILENO) < 0){
                            perror("dup2 Error\n");
                            exit(EX_OSERR);
                        }
                        close(fd_in);
                    }
                    if(t->output){ /* output redirection */
                        if((fd_out = open(t->output, O_WRONLY | O_CREAT | 
                        O_TRUNC, 0664)) < 0){
                            perror("open Error\n");
                            exit(EX_OSERR);
                        }
                        if(dup2(fd_out, STDOUT_FILENO) < 0){
                            perror("dup2 Error\n");
                            exit(EX_OSERR);
                        }
                        close(fd_out);
                    }
                    execvp(t->argv[0], t->argv);
                    fprintf(stderr, "Failed to execute %s\n", t->argv[0]);
                    exit(EX_OSERR);
                }
            }  
        }else if(t->conjunction == AND){ /* AND */
            if(!execute_h(t->left, fd_in, fd_out) && 
            !execute_h(t->right, fd_in, fd_out)){
                return 0;
            }
            return -1;
            
        }else if(t->conjunction == PIPE){ /* PIPE */
            /* Checking for ambiguous redirection */
            if(t->left->output || (t->left->output && t->right->input)){
                fprintf(stdout, "Ambiguous output redirect.\n");
                return -1;
                    
            }else if(t->right->input){
                fprintf(stdout, "Ambiguous input redirect.\n");
                return -1;      
            }
            
            if(t->input){
                if((fd_in = open(t->input, O_RDONLY)) < 0){
                    perror("open Error\n");
                    exit(EX_OSERR);
                }
            }
            if(t->output){
                if((fd_out = open(t->output, O_WRONLY | O_CREAT | 
                O_TRUNC, 0664)) < 0){
                    perror("open Error\n");
                    exit(EX_OSERR);
                }
            }
            
            if(pipe(pipe_fd) < 0){ /* checking pipe error */
                perror("pipe Error\n");
                exit(EX_OSERR);
            }
            
            if((pid = fork()) < 0){ /* checking fork error */
                perror("fork Error\n");
                exit(EX_OSERR);
            }
            
            if(pid){ /* PIPE: parent process handles right side of pipe */
                wait(&status);
                close(pipe_fd[1]); /* closing write end */
                if(!status){ /* runs if child process runs successfully */
                    if(dup2(pipe_fd[0], STDIN_FILENO) < 0){ 
                        perror("dup2 Error\n");
                        exit(EX_OSERR);
                    }
                    if(execute_h(t->right, fd_in, fd_out)){
                        close(pipe_fd[0]);
                        return -1;
                    }
                }
                close(pipe_fd[0]);
                return status;
                
            }else{ /* PIPE: child process handles left side of pipe */
                close(pipe_fd[0]); /* closing read end */
                if(dup2(pipe_fd[1], STDOUT_FILENO) < 0){ 
                    perror("dup2 Error\n");
                    exit(EX_OSERR);
                }
                execute_h(t->left, fd_in, fd_out);
                close(pipe_fd[1]);
                exit(0);
            }
            
        }else if(t->conjunction == SUBSHELL){ /* SUBSHELL */
            if(t->input){
                if((fd_in = open(t->input, O_RDONLY)) < 0){
                    perror("open Error\n");
                    exit(EX_OSERR);
                }
            }
            if(t->output){
                if((fd_out = open(t->output, O_WRONLY | O_CREAT | 
                O_TRUNC, 0664)) < 0){
                    perror("open Error\n");
                    exit(EX_OSERR);
                }
            }
            if((pid = fork()) < 0){
                perror("fork Error\n");
                exit(EX_OSERR);
            }
            if(pid){
                wait(&status);
            }else{
                execute_h(t->left, fd_in, fd_out);
                exit(0);
            }
        }
    }
    return 0;
}

static void print_tree(struct tree *t) {
   if (t != NULL) {
      print_tree(t->left);

      if (t->conjunction == NONE) {
         printf("NONE: %s, ", t->argv[0]);
      } else {
         printf("%s, ", conj[t->conjunction]);
      }
      printf("IR: %s, ", t->input);
      printf("OR: %s\n", t->output);

      print_tree(t->right);
   }
}

