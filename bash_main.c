#include "main.h"
#include "parse_split.h"

int main() {
  while(1){
    char pth[PATH_MAX];
    char* pthptr;
    pthptr = getwd(pth);
    printf(GRN "mishbash" RESET ":" BLU "%s" RESET "$ ", pth);
    char *sentence = readline();
    char **splitted_sent = split(sentence);

    int chk = 0;
    for(int i = 0; splitted_sent[i] != NULL; i++){
      if(!strcmp(splitted_sent[i], "exit")){
        printf("ну бывай!\n");
        chk = 1;
      }
      if(!strcmp(splitted_sent[i], "./bash")){
        printf("не, ну ты че делаешь-то?\n");
        chk = 2;
      }
    } 

    if(chk == 1) break;
    if(chk == 2) continue;

    node* tree = parse(splitted_sent);
    tree_exe(tree);
    free(sentence);
    for (int i = 0; splitted_sent[i] != NULL; i++){
      free(splitted_sent[i]);
    }
    free_tree(tree);
    free(splitted_sent);
  }

  return 0;
}