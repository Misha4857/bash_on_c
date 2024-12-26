 
#ifndef PARSE_SPLIT_JOBS_H
#define PARSE_SPLIT_JOBS_H
#include "main.h"

enum Type {
  LOGIC, OPERATION, REDIRECT
};

enum Ground {
  FOREGROUND, BACKGROUND
};

typedef struct node {
  enum Type type;
  enum Ground grnd;
  char* oper;
  char* comm;
  struct node* right;
  struct node* left;
} node;

typedef struct job {
  pid_t pid;
  char* comm;
  struct job* nxt;
} job;



char** split(const char* s);
char* readline();
int tree_exe(node* root);
node* parse(char** sent);

job* create_job(pid_t pid, const char* comm);
int push_job(job* jobs, pid_t pid, const char* comm);
int delete_job(job** jobs, pid_t pid);
void print_jobs(job* jobs);

void free_tree(node* root);

#endif