#include "main.h"
#include "parse_split.h"

node* create_node(enum Type type, char* oper, char* comm, node* right, node* left){
  node* tmp = (node*)malloc(sizeof(node));
  tmp->type = type;
  tmp->oper = oper;
  tmp->comm = comm;
  tmp->right = right;
  tmp->left = left;
  return tmp;
}

node* parse_comm(char** sent, int* ind){
  node* comm = create_node(OPERATION, NULL, sent[*ind], NULL, NULL);
  (*ind)++;
  return comm;
}

node* parse_redirect_left(char** sent, int* ind){
  node* left = parse_comm(sent, ind);
  while(sent[*ind] != NULL && !strcmp(sent[*ind], "<")){
    char* oper = malloc(strlen(sent[*ind]));
    strcpy(oper, sent[*ind]);
    (*ind)++;
    node* right = parse_redirect_left(sent, ind);
    left = create_node(REDIRECT, oper, NULL, right, left);
  }
  return left;
}

node* parse_redirect_right(char** sent, int* ind){
  node* left = parse_redirect_left(sent, ind);
  while(sent[*ind] != NULL && (!strcmp(sent[*ind], ">") || !strcmp(sent[*ind], ">>"))){
    char* oper = malloc(strlen(sent[*ind]));
    strcpy(oper, sent[*ind]);
    (*ind)++;
    node* right = parse_redirect_right(sent, ind);
    left = create_node(REDIRECT, oper, NULL, right, left);
  }
  return left;
}

node* parse_pipe(char** sent, int* ind){
  node* left = parse_redirect_right(sent, ind);
  while(sent[*ind] != NULL && !strcmp(sent[*ind], "|")){
    char* oper = malloc(strlen(sent[*ind]));
    strcpy(oper, sent[*ind]);
    (*ind)++;
    node* right = parse_pipe(sent, ind);
    left = create_node(REDIRECT, oper, NULL, right, left);
  }
  return left;
}

node* parse_and(char** sent, int* ind){
  node* left = parse_pipe(sent, ind);
  while(sent[*ind] != NULL && !strcmp(sent[*ind], "&&")){
    (*ind)++;
    node* right = parse_and(sent, ind);
    left = create_node(LOGIC, "&&", NULL, right, left);
  }
  return left;
}

node* parse_or(char** sent, int* ind){
  node* left = parse_and(sent, ind);
  while(sent[*ind] != NULL && !strcmp(sent[*ind], "||")){
    (*ind)++;
    node* right = parse_or(sent, ind);
    left = create_node(LOGIC, "||", NULL, right, left);
  }
  return left;
}

node* parse_continue(char** sent, int* ind){
  node* left = parse_or(sent, ind);
  while(sent[*ind] != NULL && !strcmp(sent[*ind], ";")){
    (*ind)++;
    node* right = parse_continue(sent, ind);
    left = create_node(LOGIC, ";", NULL, right, left);
  }
  return left;
}

node* parse(char** sent){
  int ind = 0;
  return parse_continue(sent, &ind);
}

char* exp_comm(const char* exp){
  
  char* tmp = malloc(BUF_MAX);
  int sz = BUF_MAX, i = 0;
  
  for(; exp[i] != '\0' && exp[i] != ' '; i++){
    if(i >= sz){
      sz += BUF_MAX;
      tmp = (char*)realloc(tmp, sz);
    }
    tmp[i] = exp[i];
  }
  tmp[i] = '\0';
  
  return tmp;
}

char** exp_arg(const char* exp) {
  
  int arg_sz = 1;
  char** arg = malloc((arg_sz + 1) * sizeof(char*));
  int arg_ind = 0;
  char* tmp = malloc(BUF_MAX);
  int tmp_ind = 0;

  for(int i = 0; exp[i] != '\0'; i++){
    if(exp[i] == ' '){
      while(exp[i+1] == ' ') i++;
      tmp[tmp_ind] = '\0';
      arg[arg_ind] = malloc(strlen(tmp) + 1);
      strcpy(arg[arg_ind++], tmp);
      tmp_ind = 0;
    }else{
      tmp[tmp_ind++] = exp[i];
    }
  }

  tmp[tmp_ind++] = '\0';
  arg[arg_ind] = (char*)malloc(tmp_ind + 1);
  strcpy(arg[arg_ind++], tmp);
  arg[arg_ind] = NULL;
  
  return arg;
}

int exp_exe(const char* exp) {
  int stat;
  if(!strcmp(exp_comm(exp), "cd")){
    if(chdir(exp_arg(exp)[1])){
      printf(RED "ОШИБКА" RESET ": дирректории [%s] не существует. Возможно указан неверный путь.\n", exp_arg(exp)[1]);
      return 1;
    }
    return 0;
  }
  pid_t pid = fork();
  if(pid == 0){
    if(execvp(exp_comm(exp), exp_arg(exp))){
      printf(RED "ОШИБКА" RESET ": команды [%s] не существует. Попробуйте другую команду.\n", exp_comm(exp));
    };
    exit(EXIT_FAILURE);
  }else if(pid < 0){
    perror("ошибка создания процесса\n");
    return 1;
  }else{
    waitpid(pid, &stat, 0);
    if(WIFEXITED(stat)) {
      return WEXITSTATUS(stat);
    }else{
      return -1;
    }
  }
}

void free_tree(node* root) {
  if (root == NULL){
    return ;
  }
  free_tree(root->right);
  free_tree(root->left);
  free(root);
}

int tree_exe(node* root){
  if(root == NULL) return 0;

  if(root->type == OPERATION){
    return exp_exe(root->comm);
  } else if(root->type == LOGIC){
    int left_stat = tree_exe(root->left);
    if(!strcmp(root->oper, "&&")){
      if (left_stat == 0) {
        return tree_exe(root->right);
      }else{
        return left_stat;
      }
    }else if(!strcmp(root->oper, "||")){
      if (left_stat != 0) {
        return tree_exe(root->right);
      }else{
        return left_stat;
      }
    }else if(!strcmp(root->oper, ";")){ 
      return tree_exe(root->right);
    }
  }else if(root->type == REDIRECT){
    int saved_stdin, saved_stdout;
    saved_stdin = dup(STDIN_FILENO);
    saved_stdout = dup(STDOUT_FILENO);
    if(!strcmp(root->oper, "|")){
      int pipefd[2];
      if(pipe(pipefd) != 0){
        perror("ошибка перевода вывода на ввод");
      }
      pid_t cpid = fork();
      int stat;
      if(cpid == 0){
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        tree_exe(root->left);
        exit(1);
      }else{
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        tree_exe(root->right);
        waitpid(cpid, &stat, 0);
        dup2(saved_stdin, STDIN_FILENO);
        dup2(saved_stdout, STDOUT_FILENO);
        return stat;
      }
    }
    else if(!strcmp(root->oper, ">")){
      pid_t cpid = fork();
      int stat;
      if(cpid == 0){
        int fl = open(root->right->comm, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(fl, STDOUT_FILENO);
        close(fl);
        tree_exe(root->left);
        exit(1);
      }else{
        tree_exe(root->right);
        waitpid(cpid, &stat, 0);
        dup2(saved_stdin, STDIN_FILENO);
        dup2(saved_stdout, STDOUT_FILENO);
        return stat;
      }
    }

    else if(!strcmp(root->oper, ">>")){
      pid_t cpid = fork();
      int stat;
      if(cpid == 0){
        int fl = open(root->right->comm, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(fl, STDOUT_FILENO);
        close(fl);
        tree_exe(root->left);
        exit(1);
      }else{
        tree_exe(root->right);
        waitpid(cpid, &stat, 0);
        dup2(saved_stdin, STDIN_FILENO);
        dup2(saved_stdout, STDOUT_FILENO);
        return stat;
      }
    }

    else if(!strcmp(root->oper, "<")){
      pid_t cpid = fork();
      int stat;
      if(cpid == 0){
        int fl = open(root->right->comm, O_RDONLY, 0644);
        dup2(fl, STDIN_FILENO);
        close(fl);
        tree_exe(root->left);
        exit(1);
      }else{
        tree_exe(root->right);
        waitpid(cpid, &stat, 0);
        dup2(saved_stdin, STDIN_FILENO);
        dup2(saved_stdout, STDOUT_FILENO);
        return stat;
      }
    }
  }
  return 0;
}