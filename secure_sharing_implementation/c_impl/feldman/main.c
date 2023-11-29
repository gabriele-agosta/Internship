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

int choose_generator(int p, int q, int r);
int validate_input(char* text, int min_val, int max_val);
int get_secret_type();
char* read_input();
int choose_modulus(int n, int secret);
int gcd(int a, int b);
int choose_p(int q, int* r, int i);
double long mod(double long a, int b);
int* generate_coefficients(int threshold, int q, int secret);
void get_secret(int secret_type, int** secret, int* secret_length);
bool is_prime(int value);
void print_polynomial(Polynomial polynomial);
void print_players(Player* players, int n, int j);
void split_shares(Player* players, Polynomial f, int n, int q, int l);
int rebuild_secret(Player* players, int k, int q, int l);
Player* slice_array(Player* players, int start, int end);
Player* populate_players(int n, int length);
Polynomial initialize_polynomial(int* coefficients, int threshold);


int main(){
    int n_player = 0, threshold = 0, secret_type = 0, secret_length = 0;
    int* secret = NULL;
    int* q = NULL;
    int* p = NULL;
    int* generator = NULL;
    int* r = NULL;
    int** coefficients = NULL;
    long long** commitments = NULL;
    Player* players = NULL;
    FILE* result = NULL;

    n_player = validate_input("Insert the value of 'n': ", 0, INT_MAX);
    threshold = validate_input("Insert the value of the threshold (k): ", 1, n_player);
    secret_type = get_secret_type();
    get_secret(secret_type, &secret, &secret_length);
    players = populate_players(n_player, secret_length);

    coefficients = (int **)malloc(secret_length * sizeof(int *));
    for (int i = 0; i < secret_length; i++) {
        coefficients[i] = (int *)malloc(threshold * sizeof(int));
    }

    // The secret is split among players
    for (int i = 0; i < secret_length; i++) {
        printf("------------\n%d° loop\n", i + 1);
        q = (int*)realloc(q, sizeof(int) * i + 1);
        p = (int*)realloc(p, sizeof(int) * i + 1);
        r = (int*)realloc(r, sizeof(int) * i + 1);
        generator = (int*)realloc(generator, sizeof(int) * i + 1);
        q[i] = choose_modulus(n_player, secret[i]);
        p[i] = choose_p(q[i], r, i);
        generator[i] = choose_generator(p[i], q[i], r[i]);
        coefficients[i] = generate_coefficients(threshold, q[i], secret[i]);
        Polynomial f = initialize_polynomial(coefficients[i], threshold);

        printf("q = %d\n", q[i]);
        printf("p = %d\n", p[i]);
        printf("g = %d\n", generator[i]);
        print_polynomial(f);

        split_shares(players, f, n_player, q[i], i);
        print_players(players, n_player, i);
    }

    // Commitments' memory is being allocated
    commitments = (long long**)malloc(threshold * sizeof(long long*));
    for (int j = 0; j < threshold; j++) {
        commitments[j] = (long long*)malloc(secret_length * sizeof(long long));
    }

    // Dealer's commitments are being created
    for (int j = 0; j < threshold; j++) {
        for (int k = 0; k < threshold; k++) {
            long long temp = powl(generator[j], players[k].y[j]);
            commitments[j][k] = fmod(temp, p[j]);
        }
    }

    // Here players' commitments are created and then compared with the dealers'
    for (int i = 0; i < threshold; i++){
        long long value = 1;
        for (int j = 0; j < threshold; j++) {
            long long temp = pow(pow(generator[i], coefficients[i][j]), pow(i, j));
            value *= temp;
        }
        value = mod(value, p[i]);
        for (int j = 0; j < secret_length; j++) {
            printf("Dealer commitment: %lld\n", commitments[i][j]);
            printf("Value calculated by the player: %lld\n", value);
            if (value != commitments[i][j]) {
                printf("There was an error with commitments\n");
                break;
            }
        }
    }


    if ((result = fopen("./result.txt", "w+")) != NULL) {
        // The secret is getting decrypted, each time with a different number of players
        for (int i = 0; i < n_player; i++) {
            char *rebuilt_secret = NULL;
            rebuilt_secret = (char *) malloc(secret_length * sizeof(char));
            Player *sliced_array = slice_array(players, 0, i);
            for (int j = 0; j < secret_length; j++) {
                char s = rebuild_secret(sliced_array, i, q[j], j);
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
    int q = secret > n ? secret + 1 : n + 1;
    while (!is_prime(q)){
        q++;
    }
    return q;
}

int choose_p(int q, int* r, int i) {
    r[i] = 1;
    int n = 4;
    while (!is_prime(n)){
        n = r[i]*q + 1;
        r[i]++;
    }
    r[i]--;
    return n;
}

int gcd(int a, int b){
    if (a == 0)
        return b;
    if (b == 0)
        return a;
    if (a == b)
        return a;

    if (a > b)
        return gcd(a - b, b);
    return gcd(a, b - a);
}

int choose_generator(int p, int q, int r){
    int *Z_p = malloc(p * sizeof(int));
    int count_z_p = 0;

    // Campo finito Z_p
    for (int i = 0; i < p; i++) {
        if (gcd(i, p) == 1) {
            Z_p[count_z_p++] = i;
        }
    }

    // L'insieme di tutti i generatori G = {h^r mod p | h in Z_p*}
    int *G = malloc(p * sizeof(int));
    for (int i = 0; i < count_z_p; i++){
        G[i] = mod(pow(Z_p[i], r), p);
    }

    for (int i = 0; i < count_z_p; i ++) {
        for (int j = i + 1; j < count_z_p; j++) {
            if ( G[i] == G[j]) {
                for (int k = j; k < count_z_p - 1; k++) {
                    G[k] = G[k + 1];
                }
                count_z_p--;
                j--;
            }
        }
    }

    printf("\n%G=\n");
    for (int i = 0; i < count_z_p; i++){
        printf("%d ", G[i]);
    }

    int g = p;
    for (int i = 0; i < count_z_p; i++) {
        if (G[i] < g && G[i] != 1) {
            g = G[i];
            break;
        }
    }
    return g;
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

int* generate_coefficients(int threshold, int q, int secret){
    int* coefficients = malloc(threshold * sizeof(int));
    int i;
    time_t t;
    srand((unsigned) time(&t));

    for(i = threshold - 1; i > 0 ; i-- ) {
        coefficients[i] = 1 + rand() / (RAND_MAX / (q - 1) + 1);
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

void split_shares(Player* players, Polynomial f, int n, int q, int l) {
    for(int i = 0; i < n; i++) {
        for (int j = 0; j < f.n; j ++){
            players[i].y[l] += f.t[j].coefficient * ((int) pow(players[i].x[l], f.t[j].exponent));
        }
        players[i].y[l] = mod(players[i].y[l], q);
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

double long mod(double long a, int b){
    double long r = fmodl(a, b);
    return r < 0 ? r + b : r;
}

int rebuild_secret(Player* players, int k, int q, int l){
    double long result = 0;
    for (int i = 0; i <= k; i++) {
        double long product = 1;
        for (int j = 0; j <= k; j++) {
            if (j == i) continue;
            product *= (double long) (0 - players[j].x[l]) / (players[i].x[l] - players[j].x[l]);
        }
        result += players[i].y[l] * product;
    }
    result = mod(result, q);
    return (int) result;
}




