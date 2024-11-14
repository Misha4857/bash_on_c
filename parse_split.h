#ifndef PARSE_SPLIT_H
#define PARSE_SPLIT_H

enum Type {
  LOGIC, OPERATION, REDIRECT
};

typedef struct node {
  enum Type type;
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

void free_tree(node* root);

#endif