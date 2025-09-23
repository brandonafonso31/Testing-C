#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    int nombreMystere, proposition, essais = 0;
    
    srand(time(NULL)); // Initialisation de l'aléatoire
    nombreMystere = rand() % 100 + 1; // entre 1 et 100
    
    printf("Devinez le nombre mystere (entre 1 et 100) !\n");
    
    do {
        printf("Votre proposition : ");
        scanf("%d", &proposition);
        essais++;
        
        if(proposition < nombreMystere) {
            printf("C'est plus grand !\n");
        } else if(proposition > nombreMystere) {
            printf("C'est plus petit !\n");
        } else {
            printf("Bravo ! Trouvé en %d essais.\n", essais);
        }
        
    } while(proposition != nombreMystere);
    
    return 0;
}
