#include <ncurses.h>
#include <unistd.h>
#include <math.h>

int getCharIndex(char *in, int stringSize, char itemToSearch) {
  stringSize--;
  while (stringSize >= 0) {
    if (in[stringSize] == itemToSearch)
      return stringSize;
    stringSize--;
  }
  return -1;
}

void delay(int time) {
  usleep(1000000 * time);
}

bool getStringContainsChar(char *in, int stringSize, char itemToSearch) {
  int index = getCharIndex(in, stringSize, itemToSearch);
  if (index < 0)
    return 0;
  else
    return 1;
}

bool getStringContainsSequence(char *in, int stringSize, char *sequence,
                               int sequenceLength) {
  int indexBuffer;
  for(int i = 0; i < sequenceLength; i++) {
    int index = getCharIndex(in, stringSize, sequence[i]);
    if(index < 0)
      return false;
    if(index - 1 != indexBuffer && i != 0)
      return false;
    indexBuffer = index;
  }
  return true;
}

int charToInt(char* inString, int stringLength) {
  int out = 0;
  for(int i = 0; i < stringLength; i++) {
    out = out * 10;
    out += inString[i] - '0';
  }
  return out;
}

int main(int argc, char *argv[]) {
  argc--;
  int time = 0;
  while(argc > 0) { // have it > 0 to exclude excecution
    char* currentItem = argv[argc];
    int itemSize = sizeof(currentItem) / sizeof(char);

    // argument logic:
    if(getStringContainsSequence(currentItem, itemSize, "-h", 2)) {
      puts("this is (intended to be) a minimal clock/timer. emphasis on the intended to be");
      puts("-h --help -hell -hello, -hAAAAAA  displays this message");
      goto ret;
    }

    if(getStringContainsSequence(currentItem, itemSize, "-t", 2)) {
      //get the *NEXT* argument (./tuitimer -t 12)
      char* timeChar = argv[argc + 1];
      time = charToInt(timeChar, sizeof(timeChar)/sizeof(char)); //TODO: add input
      printf("%d\n", time);
      return 0;
    }
    argc--;
  }
  initscr();
  printw("%d", time);;
  refresh();
  delay(1);
  while(time > 0) {
    time--;
    clear();
    printw("%d", time);
    refresh();
    delay(1);
  }

end:
  endwin();
ret:
  return 0;
}
