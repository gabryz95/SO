#include "menu.h"

void print_menu() {
    printf("MENU EXECUTABLE FILES:\n[1] philosophers\n[2] philosophers_try\n[3] producer_consumer\n[4] producer_consumer_cond\n[5] readers_writers_1\n[6] "
           "readers_writers_2\n[7] readers_writers_3\n[8] sleeping_barber\nFai la tua scelta:\n");
}

void scelta_menu(int argc, char *argv[]) {
    int scelta;
    do {
        print_menu();
        scanf("%d", &scelta);
    } while (scelta < 0 || scelta > 8);

    switch (scelta) {
        case 1:
            printf("\nEXECUTE PHILOSOPHER:\n");
            init_philosophers(argc, argv);
            break;
        case 2:
            printf("\nEXECUTE PHILOSOPHERS TRY:\n");
            init_philosophers_try(argc, argv);
            break;
        case 3:
            printf("\nEXECUTE PRODUCER CONSUMER:\n");
            init_consumer(argc, argv);
            break;
        case 4:
            printf("\nEXECUTE PRODUCER CONSUMER CONDITIONs:\n");
            init_consumer_cond(argc, argv);
            break;
        case 5:
            printf("\nEXECUTE READERS WRITERS 1:\n");
            init_rw1(argc, argv);
            break;
        case 6:
            printf("\nEXECUTE READERS WRITERS 2:\n");
            init_rw2(argc, argv);
            break;
        case 7:
            printf("\nEXECUTE READERS WRITERS 3:\n");
            init_rw3(argc, argv);
            break;
        case 8:
            printf("\nEXECUTE SLEEPING BARBER:\n");
            init_sleeping_barber(argc, argv);
            break;
        default:
            break;
    }

}
