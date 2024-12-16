#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define secondHandChar '|'
#define minuteHandChar '='
#define hourHandChar '$'

#define charRat 1.0f/2.0f

const float pi = 3.14159267f;
const float twopi = pi * 2.0f;
const float halfPi = pi/2.0f;

int min(int a, int b) {
  return a < b ? a : b;
}

int getCharIndex(char *in, int stringSize, char itemToSearch) {
  stringSize--;
  while (stringSize >= 0) {
    if (in[stringSize] == itemToSearch)
      return stringSize;
    stringSize--;
  }
  return -1;
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

bool getStringIsSequence(char* in, char* sequence) {
  int inLen = strlen(in);
  int seqLen = strlen(sequence);
  if(inLen != seqLen)
    return 0;
  for(int i = 0; i < seqLen; i++) {
    if(in[i] != sequence[i])
      return 0;
  }
  return 1;
}

void drawHand(float angle, int radius, char character) {
  //i would do some evil bit hacks but then i realized there is a *very* simple way to do this
  double maxX, maxY;
  getmaxyx(stdscr, maxY, maxX);
  maxX = maxX / 2.0;  //if only we could bit shift :(
  maxY = maxY / 2.0;
  float sinVal, cosVal;
  sinVal = sin(angle);
  cosVal = cos(angle);
  for(int i = 0; i < radius; i++) {
    wmove(stdscr, sinVal * i * charRat + maxY, cosVal * i + maxX);
    printw("%c", character);
  }
}

void printNum(int radius) {
  float angleGrowth = pi/6.0f;
  float angle = -halfPi + angleGrowth;
  int maxX, maxY;
  getmaxyx(stdscr, maxY, maxX);
  maxX = maxX >> 1;
  maxY = maxY >> 1;
  for(int i = 1; i <= 12; i++) {
    wmove(stdscr, sin(angle) * radius * charRat + maxY, cos(angle) * radius + maxX);
    printw("%d", i);
    angle += angleGrowth;
  }
}

void printCircle(int radius, char* characters) {
  const float stepSize = pi/(2 * (float)radius);

  int maxX, maxY;
  getmaxyx(stdscr, maxY, maxX);
  maxX = maxX >> 1;
  maxY = maxY >> 1;

  float angleStep = pi/180.0f;
  float angle = 0;
  while(angle < twopi) {
    wmove(stdscr, maxY + sin(angle) * radius * charRat, maxX + cos(angle) * radius);
    printw(characters);
    angle += angleStep;
  }
}

void getTimeFromSeconds(int time, float* hours, float* minutes, float* seconds) {
  //how do we go about the time? well we know it goes
  //h < min < sec
  //and so chances are that we wanna get seconds first.
  //(mod 60)
  //then we can worry about minuts and hours
  //minutes, again mod 60?, i think so 120 seconds/60... 20, not mod 60, rather floor(x/60)
  //hours
  //simply floor(min/60) (60 minutes an a single hour)
  *seconds = time % 60;
  *minutes = time / 60.0f;
  *hours = 0;
  return;
  //splendid
}

int getTimeFromTimeChar(char* inChar, float* hours, float* minutes, float* seconds) {
  int charSize = strlen(inChar);
  if(charSize <= 0) return 1;
  *hours = -1;
  *minutes = -1;
  *seconds = -1;
  const int zeroAscii = 48;
  int currentItem = 0;
  int colonCount = 0;
  for(int i = 0; i < charSize; i++) {
    if(inChar[i] == ':') {
      colonCount++;
      if(*hours == -1)
        *hours = currentItem;
      else if(*minutes == -1)
        *minutes = currentItem;
      else if(*seconds == -1)
        *seconds = currentItem;
      currentItem = 0;
      continue;
    }
    currentItem *= 10;
    currentItem += inChar[i] - zeroAscii;
  }
  if(*seconds != -1) return 0;
  *seconds = *minutes;
  *minutes = *hours;
  *hours = 0;
  return 0;
}

int main(int argc, char *argv[]) {

  if(argc == 1) {
    printf("Please provide a flag, -s [SECONDS] for a simple seconds timer\n");
    printf("./tui-timer -h for a list of flags and what they do\n");
    return 1;
  }

  argc--;
  int time = -1;
  bool fancyShmancy = true;
  bool display[6] = {true, true, true, true, true, true};
  while(argc > 0) { // have it > 0 to exclude excecution
    char* currentItem = argv[argc];
    int itemSize = strlen(currentItem);
    // argument logic:
    if(getStringIsSequence(currentItem, "-h")) {
      puts("this is a timer written in c intended to be customizable!");
      puts("by default, this will show the actual time");
      puts("commands:");
      puts("-h                         Displays this message");
      puts("-s                         Sets the timer length (seconds)");
      puts("-t                         Sets the timer length HH:MM:SS or MM:SS, your choice");
      puts("Display rules");
      puts("-r                         Displays the time left without anything fancy");
      puts("-a                         Displays the clock without the seconds hand");
      puts("-b                         Displays the clock without the minute hand");
      puts("-c                         Displays the clock without the hour hand");
      puts("-d                         Displays the clock without the outer circle");
      puts("-e                         Displays the clock without the numbers on the inside");
      puts("-f                         Displays the clock without the time printed");
      return 0;
    } else if(getStringIsSequence(currentItem, "-t")) {
      char* timeChar = argv[argc + 1];
      float sec;
      float min;
      float hou;
      getTimeFromTimeChar(timeChar, &hou, &min, &sec);
      time = sec + (hou * 60 + min) * 60;
    } else if(getStringIsSequence(currentItem, "-s")) {
      char* timeChar = argv[argc + 1];
      time = charToInt(timeChar, strlen(timeChar));
      int charCount = strlen(currentItem);
    } else if(getStringIsSequence(currentItem, "-r")) {
      fancyShmancy = false;
    } else if(getStringIsSequence(currentItem, "-a")) {
      display[0] = false;
    } else if(getStringIsSequence(currentItem, "-b")) {
      display[1] = false;
    } else if(getStringIsSequence(currentItem, "-c")) {
      display[2] = false;
    } else if(getStringIsSequence(currentItem, "-d")) {
      display[3] = false;
    } else if(getStringIsSequence(currentItem, "-e")) {
      display[4] = false;
    } else if(getStringIsSequence(currentItem, "-f")) {
      display[5] = false;
    }
    argc--;
  }

  if(time == -1) {
    printf("plese provide a time based argument:\n");
    printf("eg:\n");
    printf("./tui-timer -t 12:00 (12 minutes)");
    goto ret;
  }

  initscr();
  curs_set(0);
  noecho();
  nodelay(stdscr, true);

  int maxX, maxY;
  getmaxyx(stdscr, maxY, maxX);

  float secRadius, minRadius, hourRadius, circleRadius, numRadius, timeOffset;
  circleRadius = min(maxX, maxY)/4.0f * 3.0f;
  secRadius = circleRadius / 4.0f * 3.0f;
  minRadius = (circleRadius / 8) * 7.0f;
  hourRadius = circleRadius / 2;
  numRadius = circleRadius - 2;
  timeOffset = circleRadius / 8;

  if(!fancyShmancy)
    goto notFancyLoop;

  float seconds, minutes, hours;
  while(time > 0) {
    time--;

    getTimeFromSeconds(time, &hours, &minutes, &seconds);

    erase();

    if(display[0])
      drawHand(seconds/60.0f * twopi - halfPi, secRadius, secondHandChar);
    if(display[1])
      drawHand(minutes/60.0f * twopi - halfPi, minRadius, minuteHandChar);
    if(display[2])
      drawHand(hours/12.0f * twopi - halfPi, hourRadius, hourHandChar);
    if(display[3])
      printCircle(circleRadius, "#");
    if(display[4])
      printNum(numRadius);
    if(display[5]) {
      //print the time (for realzies)

      int digitCount = 0;
      digitCount += minutes > 0 ? ceil(log10((int)minutes)) : 1;
      digitCount += hours > 0 ? ceil(log10((int)hours)) : 1;
      digitCount += seconds > 0 ? ceil(log10((int)seconds)) : 1;
      wmove(stdscr, maxY - 1, (maxX - digitCount)>> 1);
      printw("%d:%d:%d", (int)hours, (int)minutes, (int)seconds);
    }
    refresh();
    for(int i = 0; i < 10; i++) {
      usleep(100000);
      if(getch() == 81)
        goto end;
    }
  }
  goto giveNotification;
notFancyLoop:
  int halfX, halfY;
  halfX = (maxX >> 1) - 1;
  halfY = (maxY >> 1) - 1;
  while(time > 0) {
    time--;
    clear();
    wmove(stdscr, halfX, halfY - ((int)log10(time)) / 2);
    printw("%d", time);
    refresh();
    for(int i = 0; i < 10; i++) {
      usleep(100000);
      if(getch() == 81)
        goto end;
    }
  }
giveNotification:
  system("notify-send \"Your timer is up!\"");
end:
  endwin();
ret:
  return 0;
}
