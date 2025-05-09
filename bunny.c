#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include "bunny.h"
#include <errno.h>
#include <string.h>


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

void sig_handler(int signo) {
    if (signo == SIGUSR1) {
        printf("Chief Bunny: A bunny has arrived for watering!\n");
    }
}

void startWatering() {
    Bunny bunnies[100];
    int count = loadBunnies(bunnies);
    if (count == 0) {
        printf("No bunnies are registered yet.\n");
        return;
    }

    // Generate message queue key and create queue
    key_t key = ftok(MSG_KEY_PATH, MSG_KEY_ID);
    if (key == -1) {
        perror("ftok failed");
        return;
    }

    int msqid = msgget(key, 0666 | IPC_CREAT);
    if (msqid == -1) {
        perror("msgget failed");
        return;
    }

    // Set up signal handler for SIGUSR1
    struct sigaction sa;
    sa.sa_handler = sig_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    printf("\n--- Watering Begins! ---\n");

    // Forking child processes for each bunny
    for (int i = 0; i < count; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            // Child process: Bunny boy's actions
            sleep(1);  // Give time for the parent to set up signal handling
            kill(getppid(), SIGUSR1);  // Notify the Chief Bunny that the bunny boy has arrived
            sleep(1);

            printf("\n%s recites: \"%s\"\n", bunnies[i].name, bunnies[i].poem);

            srand(time(NULL) + getpid());  // Seed the random number generator with a unique seed
            int eggs = rand() % 20 + 1;  // Random number between 1 and 20
            printf("%s received %d red eggs!\n", bunnies[i].name, eggs);

            // Prepare message to send to the Chief Bunny
            EggMessage msg;
            msg.mtype = 1;
            msg.index = i;
            msg.eggs = eggs;

            if (msgsnd(msqid, &msg, sizeof(EggMessage) - sizeof(long), 0) == -1) {
                perror("msgsnd failed");
            }

            exit(0);  // Child process ends after sending the message
        }
    }

    // Parent process: Chief Bunny's actions

    // Receiving messages from each child and updating bunny data
    for (int i = 0; i < count; i++) {
        EggMessage msg;
        while (1) {
            if (msgrcv(msqid, &msg, sizeof(EggMessage) - sizeof(long), 1, 0) == -1) {
                if (errno == EINTR) {
                    continue; // Retry if interrupted by signal
                } else {
                    perror("msgrcv failed");
                    break;
                }
            } else {
                bunnies[msg.index].red_eggs_count = msg.eggs;  // Update bunny's red eggs count
                break;
            }
        }

        // Wait for the child process to finish before moving to the next one
        waitpid(-1, NULL, 0);
    }

    // Announce the Easter Bunny King
    announceWinner();

    // Remove the message queue
    msgctl(msqid, IPC_RMID, NULL);

    printf("\n--- Watering Finished ---\n");
    saveBunnies(bunnies, count);  // Save updated bunny data
    announceWinner();  // Announce the winner based on red eggs count
}