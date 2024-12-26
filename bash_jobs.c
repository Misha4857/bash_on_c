#include "main.h"
#include "parse_split_jobs.h"

job* create_job(pid_t pid, const char* comm){
  job* tmp = malloc(sizeof(job));
  tmp -> pid = pid;
  tmp -> comm = malloc(strlen(comm) + 1);
  strcpy(tmp->comm, comm);
  tmp -> nxt = NULL;
  return tmp;
}

int push_job(job* jobs, pid_t pid, const char* comm){
  job* tmp = create_job(pid, comm);
  while(jobs -> nxt != NULL){
    jobs = jobs -> nxt;
  }
  jobs -> nxt = tmp;
  return 0;
}

int delete_job(job** jobs, pid_t pid){
  job* head = *jobs;
  job* tmp = *jobs;
  job* prev = *jobs;
  while (tmp -> pid != pid){
    prev = tmp;
    tmp = tmp -> nxt;
  }
  if (tmp == head){
    *jobs = head -> nxt;
    return 0;
  }
  prev -> nxt = tmp -> nxt;
  free(tmp);
  return 0;
}

void print_jobs(job* jobs){
  printf("PID | NAME \n");
  while (jobs != NULL){
    printf("%d | %s \n", jobs -> pid, jobs -> comm);
    jobs = jobs -> nxt;
  }
  return;
}