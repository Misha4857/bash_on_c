
#include "main.h"
#include "parse_split_jobs.h"

job* jobs = NULL;

node* create_node(enum Type type, enum Ground grnd, char* oper, char* comm, node* right, node* left){
  node* tmp = (node*)malloc(sizeof(node));
  tmp->type = type;
  tmp->grnd = grnd;
  tmp->oper = oper;
  tmp->comm = comm;
  tmp->right = right;
  tmp->left = left;
  return tmp;
}

node* parse_comm(char** sent, int* ind){
  node* comm = create_node(OPERATION, FOREGROUND, NULL, sent[*ind], NULL, NULL);
  (*ind)++;
  if (sent[*ind] != NULL && !strcmp(sent[*ind], "&")){
    comm -> grnd = BACKGROUND;
    (*ind)++;
  }
  return comm;
}

node* parse_redirect_left(char** sent, int* ind){
  node* left = parse_comm(sent, ind);
  while(sent[*ind] != NULL && !strcmp(sent[*ind], "<")){
    char* oper = malloc(strlen(sent[*ind]));
    strcpy(oper, sent[*ind]);
    (*ind)++;
    node* right = parse_redirect_left(sent, ind);
    left = create_node(REDIRECT, FOREGROUND, oper, NULL, right, left);
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
    left = create_node(REDIRECT, FOREGROUND, oper, NULL, right, left);
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
    left = create_node(REDIRECT, FOREGROUND, oper, NULL, right, left);
  }
  return left;
}

node* parse_and(char** sent, int* ind){
  node* left = parse_pipe(sent, ind);
  while(sent[*ind] != NULL && !strcmp(sent[*ind], "&&")){
    (*ind)++;
    node* right = parse_and(sent, ind);
    left = create_node(LOGIC, FOREGROUND, "&&", NULL, right, left);
  }
  return left;
}

node* parse_or(char** sent, int* ind){
  node* left = parse_and(sent, ind);
  while(sent[*ind] != NULL && !strcmp(sent[*ind], "||")){
    (*ind)++;
    node* right = parse_or(sent, ind);
    left = create_node(LOGIC, FOREGROUND, "||", NULL, right, left);
  }
  return left;
}

node* parse_continue(char** sent, int* ind){
  node* left = parse_or(sent, ind);
  while(sent[*ind] != NULL && !strcmp(sent[*ind], ";")){
    (*ind)++;
    node* right = parse_continue(sent, ind);
    left = create_node(LOGIC, FOREGROUND, ";", NULL, right, left);
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
    if(i > sz){
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
  for (int i = 0; exp[i] != '\0'; i++){
    if (exp[i] == ' '){
      while(exp[++i] == ' ');
      i--;
      tmp[tmp_ind] = '\0';
      arg[arg_ind] = malloc(strlen(tmp) + 1);
      strcpy(arg[arg_ind++], tmp);
      tmp_ind = 0;
    }else{
      tmp[tmp_ind++] = exp[i];
    }
  }
  tmp[tmp_ind] = '\0';
  arg[arg_ind] = malloc(strlen(tmp) + 1);
  strcpy(arg[arg_ind++], tmp);
  arg[arg_ind] = NULL;
  return arg;
}

int exp_exe(const char* exp, enum Ground grnd) {
  int stat;

  if(!strcmp(exp_comm(exp), "exit") || !strcmp(exp_comm(exp), "q")){
    printf("ну бывай!\n");
    exit(1);
  }

  if (!strcmp(exp_comm(exp), "jobs")){
    print_jobs(jobs);
    return 0;
  }
  if (!strcmp(exp_comm(exp), "kill")){
    kill(atoi(exp_arg(exp)[1]), SIGKILL);
    delete_job(&jobs, atoi(exp_arg(exp)[1]));
    return 0;
  }

  if(!strcmp(exp_comm(exp), "cd")){
    if(chdir(exp_arg(exp)[1])){
      printf(RED "ОШИБКА" RESET ": дирректории [%s] не существует. Возможно указан неверный путь.\n", exp_arg(exp)[1]);
      return 1;
    }
    return 0;
  }

  if (!strcmp(exp_comm(exp), "fg")) {
    if (jobs == NULL) {
        printf("Нет background процессов для восстановления.\n");
        return 1;
    }

    job* last_job = jobs; // jobs хранит список задач, начнем с головы
    while (last_job->nxt != NULL) {
        last_job = last_job->nxt; // ищем последний процесс
    }

    pid_t pid = last_job->pid; // получаем PID последнего процесса
    printf("pid: %d", pid);
    if (pid > 0) {
        kill(pid, SIGCONT); // Отправляем сигнал продолжения
        int stat;
        waitpid(pid, &stat, 0); // Ждем завершения процесса
        delete_job(&jobs, pid); // Удаляем процесс из списка jobs
        return WIFEXITED(stat) ? WEXITSTATUS(stat) : -1;
    } else {
        printf("Невалидный PID: %d\n", pid);
        return 1;
    }
}

  pid_t pid = fork();
  if(pid == 0){
    if(grnd == BACKGROUND){
      int dev_null = open("/dev/null", O_RDWR);
      dup2(dev_null, STDIN_FILENO);
      dup2(dev_null, STDOUT_FILENO);
      dup2(dev_null, STDERR_FILENO);
      close(dev_null);
    }
    execvp(exp_comm(exp), exp_arg(exp));
    exit(EXIT_FAILURE);
  }else if(pid < 0){
    perror("ошибка создания процесса\n");
    return 1;
  }else{
    push_job(jobs, pid, exp_comm(exp));
    if(grnd == BACKGROUND){
      printf("background процесс - %d\n", pid);
    }else{
      waitpid(pid, &stat, 0);
      delete_job(&jobs, pid);
    }
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
    return exp_exe(root->comm, root->grnd);

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

void print_prompt(){
  char pth[PATH_MAX];
  char* pthptr;
  pthptr = getwd(pth);
  printf(GRN "mishbash" RESET ":" BLU "%s" RESET "$ ", pth);
}

void handle_signal(int sig) {
    printf("\n");
    fflush(stdout);
}


void handle_sigchld(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}


void setup_signal_handlers() {
    struct sigaction sa;
    
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        perror("sigaction SIGTSTP");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        perror("sigaction SIGQUIT");
        exit(EXIT_FAILURE);
    }
     sa.sa_handler = handle_sigchld;
    sigaction(SIGCHLD, &sa, NULL);
}

void reset_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
}

int main() {
  jobs = create_job(getpid(), "mishbash");
  setup_signal_handlers();
  signal(SIGCHLD, handle_sigchld);
  while(1){
    print_prompt();
    char *sentence = readline();
    if(!strcmp(sentence, "\0")) continue;
    char **splitted_sent = split(sentence);
    /*
    for (int i = 0; splitted_sent[i] != NULL; i++){
      printf("[%s]\n", splitted_sent[i]);
    }
    */
    node* tree = parse(splitted_sent);
    tree_exe(tree);
    free(sentence);
    free_tree(tree);
    free(splitted_sent);
  }

  return 0;
}