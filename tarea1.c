#include <stdio.h>
#include <pthread.h>
#include <curses.h>
#include <stdlib.h> // para el malloc

void *worker(void *key_code) {
  printw(" -> (%i) / ", *(char *)key_code);
  refresh();
  free(key_code);

  return NULL;
}

int main() {
  pthread_t thread = NULL;
  initscr();

  printw("== Lee las teclas (q para salir) ==\n");

  while(1) {
    char c = getch();
    char *key = (char *) malloc(sizeof(char));
    *key = c;

    if(pthread_create(&thread, NULL, worker, (void *) key)) {
      printw("[ERROR] No se pudo crear el thread\n");
      free(key);
      return 1;
    }

    if(pthread_detach(thread)) {
      printw("[ERROR] No se pudo detachear el thread\n");
      return 1;
    }

    if (c == 'q') {
      clear();
      endwin();

      break;
    }
  }

  return 0;
}
