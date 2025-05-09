#ifndef BUNNY_H
#define BUNNY_H

#define MAX_NAME_LENGTH 100
#define MAX_POEM_LENGTH 256
#define FILE_NAME "bunnies_data.txt"

typedef struct
{
    char name[MAX_NAME_LENGTH];
    char poem[MAX_POEM_LENGTH];
    int red_eggs_count;
} Bunny;

void addBunny();
void listBunnies();
void setRedEggsCount();
void deleteBunny();
int loadBunnies(Bunny *bunnies);
void saveBunnies(Bunny *bunnies, int count);
void announceWinner();
void countBunnies();

#endif
