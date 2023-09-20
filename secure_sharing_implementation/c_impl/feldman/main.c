#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <time.h>

typedef struct {
    int coefficient;
    int exponent;
} Term;
typedef struct {
    int n;
    Term *t;
} Polynomial;
typedef struct {
    char name[8];
    int x;
    int y;
} Player;

int validate_input(char* text, int min_val, int max_val);
int read_input();
int get_secret();
int choose_modulus(int n, int secret);
int mod(int a, int b);
int* generate_coefficients(int threshold, int p, int secret);
bool is_prime(int value);
void print_polynomial(Polynomial polynomial);
void print_players(Player* players, int n);
void split_shares(Player* players, Polynomial f, int n, int p);
void rebuild_secret(Player* players, int k, int p);
Player* slice_array(Player* players, int start, int end);
Player* populate_players(int n);
Polynomial initialize_polynomial(int* coefficients, int threshold);


int main(){
    int n_player = validate_input("Insert the value of 'n': ", 0, INT_MAX);
    int threshold = validate_input("Insert the value of the threshold (k): ", 1, n_player);
    int secret = get_secret();
    int p = choose_modulus(n_player, secret);
    Player* players = populate_players(n_player);
    int* coefficients = generate_coefficients(threshold, p, secret);
    Polynomial f = initialize_polynomial(coefficients, threshold);

    printf("P = %d\n", p);
    print_polynomial(f);

    split_shares(players, f, n_player, p);

    print_players(players, n_player);

    for (int i = 0; i < n_player; i++) rebuild_secret(slice_array(players, 0, i), i, p);
    return 1;
}

int validate_input(char* text, int min_val, int max_val){
    int value = 0;

    do {
        printf("%s", text);
        scanf("%d", &value);
    } while (value > max_val || value < min_val);
    return value;
}

int read_input(){
    char *string = NULL;
    size_t size = 0;

    fflush(stdin);
    getline(&string, &size, stdin);

    return atoi(string);
}

int get_secret(){
    FILE* fp = NULL;
    int secret = 0;

    printf("Insert the secret (M) you want to share: ");
    secret = read_input();
    if ((fp = fopen("./secret.txt", "w+")) != NULL) {
        fprintf(fp, "%d", secret);
        fclose(fp);
    }
    else {
        fprintf(stderr, "%s\n", "Couldn't open the file!");
    }
    return secret;
}

bool is_prime(int value){
    for (int divider = 2; divider < sqrt(value) + 1; divider++){
        if (value % divider == 0) return false;
    }
    return true;
}

int choose_modulus(int n, int secret) {
    int p = secret > n ? secret + 1 : n + 1;
    while (!is_prime(p)){
        p++;
    }
    return p;
}

Player* populate_players(int n){
    Player* players = malloc(n * sizeof(Player));
    if(players == NULL) {
        fprintf(stderr, "Errore nell'allocazione di memoria.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        sprintf(players[i].name, "Player%d", i + 1);
        players[i].x = i + 1;
        players[i].y = 0;
    }
    return players;
}

void print_players(Player* players, int n){
    for (int i = 0; i < n; i++) {
        printf("%s -> (%d, %d)\n", players[i].name, players[i].x, players[i].y);
    }
}

int* generate_coefficients(int threshold, int p, int secret){
    int* coefficients = malloc(threshold * sizeof(int)); // Allocazione della memoria per l'array
    int i;
    time_t t;
    srand((unsigned) time(&t));

    for(i = threshold - 1; i > 0 ; i-- ) {
        coefficients[i] = rand() % (p - 1);
    }
    coefficients[i] = secret;

    return coefficients;
}

Polynomial initialize_polynomial(int* coefficients, int threshold){
    Polynomial f;
    f.n = threshold;
    f.t = (Term*)malloc(f.n * sizeof(Term));

    for (int i = 0; i<threshold; i++) {
        f.t[i].coefficient = coefficients[i];
        f.t[i].exponent = i;
    }
    return f;
}

void print_polynomial(Polynomial polynomial){
    int i;
    for (i = 0; i < polynomial.n - 1; i++) {
        printf("%dx^%d + ", polynomial.t[i].coefficient, polynomial.t[i].exponent);
    }
    printf("%dx^%d\n", polynomial.t[i].coefficient, polynomial.t[i].exponent);
}

void split_shares(Player* players, Polynomial f, int n, int p) {
    for(int i = 0; i < n; i++) {
        for (int j = 0; j < f.n; j ++){
            players[i].y += f.t[j].coefficient * ((int) pow(players[i].x, f.t[j].exponent));
        }
        players[i].y %= p;
    }
}

Player* slice_array(Player* players, int start, int end){
    Player* sliced_array = malloc((end - start + 1) * sizeof(Player));
    if(sliced_array == NULL) {
        fprintf(stderr, "Errore nell'allocazione di memoria.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = start; i <= end; i++){
        sliced_array[i] = players[i];
    }
    return sliced_array;
}

int mod(int a, int b){
    int r = a % b;
    return r < 0 ? r + b : r;
}

void rebuild_secret(Player* players, int k, int p){
    double result = 0;
    for (int i = 0; i <= k; i++) {
        float product = 1;
        for (int j = 0; j <= k; j++) {
            if (j == i) continue;
            product *= (float) (0 - players[j].x) / (players[i].x - players[j].x);
        }
        result += players[i].y * product;
    }
    result = mod((int) result, p);
    printf("Result with %d shares: %f\n",  k + 1, result);
}





