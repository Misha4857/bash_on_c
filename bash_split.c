
#include "main.h"

int single_oper_chk(char c) {
    return c == '|' || c == '&' || c == ';' || c == '>' || c == '<';
}

int double_oper_chk(const char *s, int i) {
    return (s[i] == '&' && s[i+1] == '&') || 
           (s[i] == '|' && s[i+1] == '|') || 
           (s[i] == '>' && s[i+1] == '>') || 
           (s[i] == '<' && s[i+1] == '<');
}

char** split(const char* s){

  char** arr = NULL;
  int arr_ind = 0;
  int tmp_sz = BUF_MAX;
  int tmp_ind = 0;
  char* tmp = malloc(tmp_sz); // слово
  int quotes_chk = 0;

  for(int i = 0; s[i] != '\0'; i++){
/*
    if(s[i] == ' ' && !quotes_chk){
      if(tmp_ind){ // запись слова в строку
        tmp[tmp_ind++] = '\0';
        arr = (char**)realloc(arr, (arr_ind+1)*sizeof(char*));
        arr[arr_ind] = (char*)malloc(tmp_ind);
        strcpy(arr[arr_ind++], tmp);
        tmp_ind = 0;
      }
      continue;
    }
*/
    if(s[i] == ' ' && !quotes_chk){
      while(s[i+1] == ' ') i++;
      if(single_oper_chk(s[i+1])){
        continue;
      }
      else if(tmp_ind != 0 && s[i+1] != ' ' && s[i+1] != '\0'){
        tmp[tmp_ind++] = ' ';
        continue;
      }
      //printf("peep: %d %d %d", arr_ind != 0, !single_oper_chk(arr[arr_ind-1]), !double_oper_chk(arr[arr_ind-1], 0));
      continue;
    }
    
    if(s[i] == '\'' || s[i] == '"'){
      quotes_chk = quotes_chk == 0 ? s[i] : 0;
      continue;
    }

    if(!quotes_chk && double_oper_chk(s, i)){
      if(tmp_ind){ // запись слова в строку
        tmp[tmp_ind++] = '\0';
        arr = (char**)realloc(arr, (arr_ind+1)*sizeof(char*));
        arr[arr_ind] = (char*)malloc(tmp_ind);
        strcpy(arr[arr_ind++], tmp);
        tmp_ind = 0;
      }
      tmp[tmp_ind++] = s[i++];
      tmp[tmp_ind++] = s[i];
      tmp[tmp_ind++] = '\0';
      arr = (char**)realloc(arr, (arr_ind+1)*sizeof(char*));
      arr[arr_ind] = (char*)malloc(tmp_ind); // запись лог символа в строку
      strcpy(arr[arr_ind++], tmp);
      tmp_ind = 0;
      continue;
    }

    if(!quotes_chk && single_oper_chk(s[i])){
      if(tmp_ind){ // запись слова в строку
        tmp[tmp_ind++] = '\0';
        arr = (char**)realloc(arr, (arr_ind+1)*sizeof(char*));
        arr[arr_ind] = (char*)malloc(tmp_ind);
        strcpy(arr[arr_ind++], tmp);
        tmp_ind = 0;
      }
      tmp[tmp_ind++] = s[i];
      tmp[tmp_ind++] = '\0';
      arr = (char**)realloc(arr, (arr_ind+1)*sizeof(char*));
      arr[arr_ind] = (char*)malloc(tmp_ind); // запись лог символа в строку
      strcpy(arr[arr_ind++], tmp);
      tmp_ind = 0;
      continue;
    }

    tmp[tmp_ind++] = s[i]; // запись в слово
    
    if (tmp_ind >= tmp_sz){
      tmp_sz += BUF_MAX;
      tmp = (char*)realloc(tmp, tmp_sz);
    }
  }
  if(tmp_ind > 0){
    tmp[tmp_ind++] = '\0';
    arr = (char**)realloc(arr, (arr_ind+1)*sizeof(char*));
    arr[arr_ind] = (char*)malloc(tmp_ind); // запись последнего слова в строку
    strcpy(arr[arr_ind++], tmp);
  }

  arr = (char**)realloc(arr, (arr_ind+1)*sizeof(char*));
  arr[arr_ind] = NULL;

  return arr;
}

char* readline(){
  int n = BUF_MAX, i = 0, c;
  char* buf = (char*)malloc(n*sizeof(char));
  while((c=getchar())!='\n'){
    if (i == n-1){
      n*=2;
      buf = (char*)realloc(buf, n*sizeof(char));
    }
    buf[i++] = c;
  }
  buf[i] = '\0';
  return buf;
}