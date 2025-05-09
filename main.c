#include <stdio.h>
#include "bunny.h"

int main()
{
    int choice;
    do
    {
        printf("\n*** Easter Bunny King Competition ***\n");
        printf("1. Register Bunny\n");
        printf("2. List Bunnies\n");
        printf("3. Count Bunnies\n");
        printf("4. Set Red Eggs Count\n");
        printf("5. Delete Bunny\n");
        printf("6. Announce Winner\n");
        printf("7. Start Watering (Easter Monday)\n");
        printf("8. Exit\n");
        printf("Choose an option: ");
        scanf("%d", &choice);
        getchar();

        switch (choice)
        {
        case 1:
            addBunny();
            break;
        case 2:
            listBunnies();
            break;
        case 3:
            countBunnies();
            break;
        case 4:
            setRedEggsCount();
            break;
        case 5:
            deleteBunny();
            break;
        case 6:
            announceWinner();
            break;
        case 7:
            startWatering();
            break;
        case 8:
            printf("Goodbye!\n");
            break;
        default:
            printf("Invalid choice. Please try again.\n");
            break;
        }
    } while (choice != 7);

    return 0;
}
