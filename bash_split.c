#include "main.h"

char** split(const char* s){
  int n = 0;
  for (int i = 0; s[i] != '\0'; i++){
    if (s[i] == '|' || s[i] == '&' || s[i] == '<' || s[i] == '>' || s[i] == ';'){
      n++;
      while(s[i] == '|' || s[i] == '&' || s[i] == '<' || s[i] == '>' || s[i] == ';'){
        i++;
      }
    }
  }
  if (n) n = 2 * n + 1;
  else if (s[0] != '\0'){
    n = 1;
  }
  else return NULL;

  char** arr = (char**)malloc((n + 1) * sizeof(char*)); // строка
  arr[n] = NULL;
  int arr_ind = 0;
  int tmp_sz = BUF_MAX;
  int tmp_ind = 0;
  char* tmp = malloc(tmp_sz); // слово

  for(int i = 0; s[i] != '\0'; i++){
    if(s[i] == '&' || s[i] == '|' || s[i] == '<' || s[i] == '>' || s[i] == ';' || (s[i] == ' ' && (s[i+1] == '&' || s[i+1] == '|' || s[i+1] == '<' || s[i+1] == '>' || s[i+1] == ';'))){
      if(tmp_ind){ // запись слова в строку
        tmp[tmp_ind++] = '\0';
        arr[arr_ind] = (char*)malloc(tmp_ind);
        strcpy(arr[arr_ind++], tmp);
        tmp_ind = 0;
      }
      if(s[i] == '&' || s[i] == '|' || s[i] == '>' || s[i] == '<' || s[i] == ';'){ // выделение лог символа
        tmp[tmp_ind++] = s[i];
        
        if(s[i+1] == '&' || s[i+1] == '|' || s[i+1] == '<' || s[i+1] == '>'){
          i++;
          tmp[tmp_ind++] = s[i];
        }
        tmp[tmp_ind++] = '\0';
        arr[arr_ind] = (char*)malloc(tmp_ind); // запись лог символа в строку
        strcpy(arr[arr_ind++], tmp);
        tmp_ind = 0;
      }
      while((s[i] == ' ' || s[i] == '&' || s[i] == '|' || s[i] == '<' || s[i] == '>' || s[i] == ';') && s[i+1] == ' ') i++; // пропуск пробелов
      continue; // начало проверки заново
    }

    tmp[tmp_ind++] = s[i]; // запись в слово
    
    if (tmp_ind >= tmp_sz){
      tmp_sz += BUF_MAX;
      tmp = (char*)realloc(tmp, tmp_sz);
    }
  }

  tmp[tmp_ind++] = '\0';
  arr[arr_ind] = (char*)malloc(tmp_ind); // запись последнего слова в строку
  strcpy(arr[arr_ind], tmp);

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