#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include "bunny.h"

void handle_bunny_arrival(int sig) {
    printf("Chief Bunny: A bunny has arrived for watering!\n");
}

void trim_newline(char *str) {
    str[strcspn(str, "\n")] = '\0';
}

int is_empty(const char *str) {
    return str[0] == '\0';
}

void addBunny() {
    FILE *f = fopen(FILE_NAME, "a");
    if (!f) {
        printf("Error opening file for writing.\n");
        return;
    }

    Bunny b;

    
    do {
        printf("Enter bunny name: ");
        fgets(b.name, MAX_NAME_LENGTH, stdin);
        trim_newline(b.name);
    } while (is_empty(b.name));

    
    do {
        printf("Enter poem: ");
        fgets(b.poem, MAX_POEM_LENGTH, stdin);
        trim_newline(b.poem);
    } while (is_empty(b.poem));

    b.red_eggs_count = 0; 

    fprintf(f, "%s|%s|%d\n", b.name, b.poem, b.red_eggs_count);
    fclose(f);
    printf("Bunny registered successfully!\n");
}

void listBunnies() {
    FILE *f = fopen(FILE_NAME, "r");
    if (!f) {
        printf("Error opening file for reading.\n");
        return;
    }

    Bunny b;
    char line[512];
    int count = 0;

    printf("\n--- List of Bunnies ---\n");
    while (fgets(line, sizeof(line), f)) {
        sscanf(line, "%[^|]|%[^|]|%d", b.name, b.poem, &b.red_eggs_count);
        printf("%d. Name: %s\n   Poem: %s\n   Red Eggs: %d\n", ++count, b.name, b.poem, b.red_eggs_count);
    }

    if (count == 0)
        printf("No bunny has been registered yet!\n");

    fclose(f);
}

int loadBunnies(Bunny *bunnies) {
    FILE *f = fopen(FILE_NAME, "r");
    if (!f) {
        return 0;
    }

    int count = 0;
    char line[512];
    while (fgets(line, sizeof(line), f) && count < 100) {
        sscanf(line, "%[^|]|%[^|]|%d", bunnies[count].name, bunnies[count].poem, &bunnies[count].red_eggs_count);
        count++;
    }
    fclose(f);
    return count;
}

void saveBunnies(Bunny *bunnies, int count) {
    FILE *f = fopen(FILE_NAME, "w");
    if (!f) {
        printf("Error opening file for saving.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(f, "%s|%s|%d\n", bunnies[i].name, bunnies[i].poem, bunnies[i].red_eggs_count);
    }
    fclose(f);
}

void setRedEggsCount() {
    Bunny bunnies[100];
    int count = loadBunnies(bunnies);
    if (count == 0) {
        printf("No bunny has been registered yet!\n");
        return;
    }

    listBunnies();

    int index;
    printf("Enter bunny number to set red eggs count: ");
    scanf("%d", &index);
    getchar();

    if (index < 1 || index > count) {
        printf("Invalid bunny number!\n");
        return;
    }

    printf("Enter new red eggs count for %s: ", bunnies[index - 1].name);
    scanf("%d", &bunnies[index - 1].red_eggs_count);
    getchar();

    saveBunnies(bunnies, count);
    printf("Red eggs count updated!\n");
}

void deleteBunny() {
    Bunny bunnies[100];
    int count = loadBunnies(bunnies);
    if (count == 0) {
        printf("No bunny has been registered yet!\n");
        return;
    }

    listBunnies();

    int index;
    printf("Enter bunny number to delete: ");
    scanf("%d", &index);
    getchar();

    if (index < 1 || index > count) {
        printf("Invalid bunny number entered!\n");
        return;
    }

    for (int i = index - 1; i < count - 1; i++) {
        bunnies[i] = bunnies[i + 1];
    }

    saveBunnies(bunnies, count - 1);
    printf("Appropriate bunny deleted successfully!.\n");
}

void announceWinner() {
    Bunny bunnies[100];
    int count = loadBunnies(bunnies);
    if (count == 0) {
        printf("No bunnies are registered yet.\n");
        return;
    }

    int max = bunnies[0].red_eggs_count;
    int winner_index = 0;

    for (int i = 1; i < count; i++) {
        if (bunnies[i].red_eggs_count > max) {
            max = bunnies[i].red_eggs_count;
            winner_index = i;
        }
    }

    printf("\nThe Easter Bunny King:\n");
    printf("Name: %s\nPoem: %s\nRed Eggs: %d\n",
           bunnies[winner_index].name,
           bunnies[winner_index].poem,
           bunnies[winner_index].red_eggs_count);
}

void countBunnies() {
    Bunny bunnies[100];
    int count = loadBunnies(bunnies);
    printf("\nTotal number of bunnies registered so far: %d\n", count);
}

void startWatering() {
    Bunny bunnies[100];
    int count = loadBunnies(bunnies);
    if (count == 0) {
        printf("No bunnies are registered yet.\n");
        return;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(1);
    }

    signal(SIGUSR1, handle_bunny_arrival);

    printf("\n--- Watering begins! ---\n");

    for (int i = 0; i < count; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }
        // Child process
        else if (pid == 0) {
            kill(getppid(), SIGUSR1);

            printf("\nBunny: %s is reciting poem:\n\"%s\"\n", bunnies[i].name, bunnies[i].poem);

            srand(time(NULL) ^ (getpid() << 16));
            int eggs = (rand() % 20) + 1;

            printf("Bunny: %s received %d red eggs!\n", bunnies[i].name, eggs);

            close(pipefd[0]);
            write(pipefd[1], &eggs, sizeof(int));
            close(pipefd[1]);

            exit(0);
        }
        
    }

    // Parent process
    close(pipefd[1]);

    for (int i = 0; i < count; i++) {
        int eggs;
        read(pipefd[0], &eggs, sizeof(int));
        bunnies[i].red_eggs_count = eggs;
        wait(NULL); 
    }

    close(pipefd[0]);

    saveBunnies(bunnies, count);

    printf("\n--- Watering complete ---\n");
    announceWinner();
}
