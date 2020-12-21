#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <chrono>

/*Hyperparams*/
#define PARAMS_population_size 100
#define PARAMS_survival_size 10
#define PARAMS_genotype_length 1600
#define PARAMS_mutation_rate 0.05
#define PARAMS_instance_per_phone_target 60
#define PARAMS_zero_padded_sentence_space 120000
#define PARAMS_epochs 10
#define PARAMS_parents 2
#define PARAMS_time_epochs 1
#define NUM_phones 399

using namespace std;

/*Struct*/
struct COST_AND_INDEX {
    int cost;
    int index;
};

/*FunctionDeclaration*/
void init_tag_distro_space();
int chars2int(char ch[]);
void LoadCSVdata();
void init_GA_pool();
void init_random_alias();
bool compare_cost(COST_AND_INDEX &a, COST_AND_INDEX &b);
int calc_loss(int genotype[]);
COST_AND_INDEX cost_and_index[PARAMS_population_size];

/*GlobalVriables*/
int tag_distro_space[PARAMS_zero_padded_sentence_space][NUM_phones];
int GA_pool[PARAMS_population_size][PARAMS_genotype_length];
int random_alias[PARAMS_genotype_length];

void init_tag_distro_space() {
    for (int i=0; i<PARAMS_zero_padded_sentence_space; ++i) {
        for (int j=0; j<NUM_phones; ++j) {
            tag_distro_space[i][j] = 0;
        }
    }
    printf("tag_distro_space init to %d\n", 0);
}

void init_random_alias() {
    for (int i=0; i<PARAMS_genotype_length; ++i) {
        random_alias[i] = i;
    }
    printf("random_alias MAX_val is %d\n", random_alias[PARAMS_genotype_length-1]);
}

int chars2int(char ch[]) {
    int last_digit = ch[0]-'0';
    int val = 0;
    if (ch[1] == 'e' || ch[1] == 10 || ch[1] == 13 || ch[1] == 32 || ch[1] == 44) {
        return last_digit;
    }

    for (int i=1; ; ++i) {
        if (ch[i] == 'e' || ch[i] == 10 || ch[i] == 13 || ch[i] == 32 || ch[i] == 44) {
            break;
        }
        val += last_digit*10 + ch[i]-'0';
        last_digit = ch[i]-'0';
    }
    return val;
}

void LoadCSVdata() {
    char row_buffer[1024];
    char number_buffer[10];
    
    FILE* stream = fopen("cpp_sentence_phoneDist.csv", "r");

    if(stream == NULL) {
      perror("Error opening file");
   }

    int row_count = 0;
    int char_count = 0;
    while (fgets(row_buffer, 1024, stream))
    {
        int row_index = 0;
        for (int i=0; i<(int)strlen(row_buffer); ++i) {

            if (row_buffer[i] != 10 && row_buffer[i] != 32 && row_buffer[i] != 44 && row_buffer[i] != 13) {
                number_buffer[char_count] = row_buffer[i];
                char_count++;
            }
            else {
                number_buffer[char_count] = 'e';
                if (number_buffer[0] != 'e' && number_buffer[0] != 10 && number_buffer[0] != 32 && number_buffer[0] != 44 && number_buffer[0] != 13) {
                    tag_distro_space[row_count][row_index] = chars2int(number_buffer);
                    //printf("%d ", tag_distro_space[row_count][row_index]);
                    row_index++;
                }
                
                char_count = 0;

                if (row_buffer[i] == 10) {
                    //printf("Row_num %d. Phones in this row: %d\n", row_count, row_index);
                    //printf("\n");
                    row_count++;
                    continue;
                }
            }
        }
    }
}

void init_GA_pool() {
    float init_AVG = 0;
    int init_MAX = 0;
    int init_MIN = PARAMS_zero_padded_sentence_space;
    int num_Val = PARAMS_population_size*PARAMS_genotype_length;
    
    for (int i=0; i<PARAMS_population_size; ++i) {
        for (int j=0; j<PARAMS_genotype_length; ++j) {
            GA_pool[i][j] = (int)(rand()%PARAMS_zero_padded_sentence_space);
            init_AVG += GA_pool[i][j];
            if (GA_pool[i][j]>init_MAX) init_MAX = GA_pool[i][j];
            if (GA_pool[i][j]<init_MIN) init_MIN = GA_pool[i][j];
        }
    }

    printf("init_MAX = %d\n", init_MAX);
    printf("init_MIN = %d\n", init_MIN);
    printf("init_AVG = %f\n", init_AVG / num_Val);
}

int compare_cost(const void * a, const void * b) {
    COST_AND_INDEX *C_I_a = (COST_AND_INDEX *)a;
    COST_AND_INDEX *C_I_b = (COST_AND_INDEX *)b;
    return (C_I_a->cost > C_I_b->cost) - (C_I_a->cost < C_I_b->cost);
}

int calc_loss(int genotype[]) {
    int num_syl = 0;
    int cost = 0;
    int pheno_distribution[NUM_phones]={};

    for (int i=0; i<PARAMS_genotype_length; ++i) {
        for (int j=0; j<NUM_phones; ++j) {
            num_syl += tag_distro_space[(int)genotype[i]][j];
            pheno_distribution[j] += tag_distro_space[(int)genotype[i]][j];
        }
        if (genotype[i]>PARAMS_zero_padded_sentence_space-1) {
            printf("genotype overflow");
        }
    }
    for (int i=0; i<NUM_phones; ++i) {
        //printf("pheno %d\n", pheno_distribution[i]);
        cost += abs(PARAMS_instance_per_phone_target-pheno_distribution[i]);
    }
    //printf("num_syl %d\n", num_syl);
    return cost;
}

void sort_population_by_cost() {
    /*write cost2individual alias*/
    for (int i=0; i<PARAMS_population_size; ++i) {
        cost_and_index[i].index = i;
        cost_and_index[i].cost =calc_loss(GA_pool[i]);
    }
    /*SortAlias*/
    qsort(cost_and_index, PARAMS_population_size, sizeof(COST_AND_INDEX), compare_cost);

    for (int i=0; i<PARAMS_population_size; ++i) {
        printf("%d ", cost_and_index[i].cost);
    }
}

int main() {
    init_tag_distro_space();
    init_random_alias();
    srand((unsigned int)time(NULL));
    LoadCSVdata();
    init_GA_pool();

    printf("calc_cost test %d\n", calc_loss(GA_pool[0]));
    sort_population_by_cost();
    for(int epoch=0; epoch<PARAMS_epochs; ++epoch) {

    }
    
    return 0;
}