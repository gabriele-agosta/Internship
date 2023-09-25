#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

typedef struct {
    int coefficient;
    int exponent;
} Term;
typedef struct {
    int n;
    Term *t;
} Polynomial;
typedef struct {
    char name[32];
    int* x;
    int* y;
} Player;

int validate_input(char* text, int min_val, int max_val);
int get_secret_type();
char* read_input();
int choose_modulus(int n, int secret);
int mod(long a, int b);
int* generate_coefficients(int threshold, int p, int secret);
void get_secret(int secret_type, int** secret, int* secret_length);
bool is_prime(int value);
void print_polynomial(Polynomial polynomial);
void print_players(Player* players, int n, int j);
void split_shares(Player* players, Polynomial f, int n, int p, int l);
int rebuild_secret(Player* players, int k, int p, int l);
Player* slice_array(Player* players, int start, int end);
Player* populate_players(int n, int length);
Polynomial initialize_polynomial(int* coefficients, int threshold);


int main(){
    int n_player = 0, threshold = 0, secret_type = 0, secret_length = 0;
    int* secret = NULL;
    int* p = NULL;
    Player* players = NULL;
    FILE* result = NULL;

    n_player = validate_input("Insert the value of 'n': ", 0, INT_MAX);
    threshold = validate_input("Insert the value of the threshold (k): ", 1, n_player);
    secret_type = get_secret_type();
    get_secret(secret_type, &secret, &secret_length);
    players = populate_players(n_player, secret_length);

    for (int t = 0; t < secret_length; t++){
        printf("%d ", secret[t]);
    }

    // The secret is split among players
    for (int i = 0; i < secret_length; i++) {
        p = (int*)realloc(p, sizeof(int) * i + 1);
        p[i] = choose_modulus(n_player, secret[i]);
        int* coefficients = generate_coefficients(threshold, p[i], secret[i]);
        Polynomial f = initialize_polynomial(coefficients, threshold);

        printf("P = %d\n", p[i]);
        print_polynomial(f);

        split_shares(players, f, n_player, p[i], i);
        print_players(players, n_player, i);
    }

    if ((result = fopen("./result.txt", "w+")) != NULL) {
        // The secret is getting decrypted, each time with a different number of players
        for (int i = 0; i < n_player; i++) {
            char *rebuilt_secret = NULL;
            rebuilt_secret = (char *) malloc(secret_length * sizeof(char));
            Player *sliced_array = slice_array(players, 0, i);
            for (int j = 0; j < secret_length; j++) {
                char s = rebuild_secret(sliced_array, i, p[j], j);
                strncat(rebuilt_secret, &s, 1);
            }
            fprintf(result, "Result with %d players: %s\n", i + 1, rebuilt_secret);
        }
    } else {
        fprintf(stderr, "%s\n", "Couldn't open the file");
    }
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

char* read_input(){
    char *string = NULL;
    size_t size = 0;

    fflush(stdin);
    getline(&string, &size, stdin);

    return string;
}

int get_secret_type(){
    int secret_type = 0;

    do {
        printf("Choose what kind of secret you want to share: \n1.Number/String; \n2.File \n");
        scanf("%d", &secret_type);
    } while (secret_type > 2 || secret_type < 1);
    return secret_type;
}

void get_secret(int secret_type, int** secret, int* secret_length){
    char* input = NULL;

    if (secret_type == 2) {
        FILE* fp = NULL;
        int length = 0;

        if ((fp = fopen("./secret.txt", "r")) != NULL) {
            char ch;

            while ((ch = fgetc(fp)) != EOF) {
                input = (char*)realloc(input, (length + 1) * sizeof(char));
                if (input == NULL) {
                    fprintf(stderr, "Memory allocation error.\n");
                    exit(EXIT_FAILURE);
                }
                input[length] = ch;
                length++;
            }
            input = (char*)realloc(input, (length + 1) * sizeof(char));
            if (input == NULL) {
                fprintf(stderr, "Memory allocation error.\n");
                exit(EXIT_FAILURE);
            }
            input[length] = '\0';
            *secret_length = length;
            fclose(fp);
        }
        else {
            fprintf(stderr, "%s\n", "Couldn't open the file");
        }

    }
    else {
        printf("Insert the secret (M) you want to share: ");
        input = read_input();
        *secret_length = strlen(input) - 1;
    }

    *secret = (int*)realloc(*secret, sizeof(int) * strlen(input));
    for (int i = 0; input[i] != '\0'; i++) {
        (*secret)[i] = input[i];
    }
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

Player* populate_players(int n, int length){
    Player* players = malloc(n * sizeof(Player));
    if(players == NULL) {
        fprintf(stderr, "Memory allocation error.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) {
        sprintf(players[i].name, "Player%d", i + 1);

        players[i].x = malloc(length * sizeof(long));
        players[i].y = malloc(length * sizeof(long));
        if (players[i].x == NULL || players[i].y == NULL) {
            fprintf(stderr, "Memory allocation error for Player %d.\n", i + 1);
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < length; j++) {
            players[i].x[j] = i + 1;
            players[i].y[j] = 0;
        }
    }
    return players;
}

void print_players(Player* players, int n, int j){
    for (int i = 0; i < n; i++) {
        printf("%s -> (%d, %d)\n", players[i].name, players[i].x[j], players[i].y[j]);
    }
}

int* generate_coefficients(int threshold, int p, int secret){
    int* coefficients = malloc(threshold * sizeof(int));
    int i;
    time_t t;
    srand((unsigned) time(&t));

    for(i = threshold - 1; i > 0 ; i-- ) {
        coefficients[i] = 1 + rand() / (RAND_MAX / (p - 1) + 1);
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

void split_shares(Player* players, Polynomial f, int n, int p, int l) {
    for(int i = 0; i < n; i++) {
        for (int j = 0; j < f.n; j ++){
            players[i].y[l] += f.t[j].coefficient * ((int) pow(players[i].x[l], f.t[j].exponent));
        }
        players[i].y[l] = mod(players[i].y[l], p);
    }
}

Player* slice_array(Player* players, int start, int end){
    Player* sliced_array = malloc((end - start + 1) * sizeof(Player));
    if(sliced_array == NULL) {
        fprintf(stderr, "Memory allocation error.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = start; i <= end; i++){
        sliced_array[i] = players[i];
    }
    return sliced_array;
}

int mod(long a, int b){
    long r = a % b;
    return r < 0 ? r + b : r;
}

int rebuild_secret(Player* players, int k, int p, int l){
    double long result = 0;
    for (int i = 0; i <= k; i++) {
        double long product = 1;
        for (int j = 0; j <= k; j++) {
            if (j == i) continue;
            product *= (double long) (0 - players[j].x[l]) / (players[i].x[l] - players[j].x[l]);
        }
        result += players[i].y[l] * product;
    }
    result = mod((long) result, p);
    return result;
}




