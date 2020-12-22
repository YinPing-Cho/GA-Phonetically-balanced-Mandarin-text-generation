#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <chrono>
#include <fstream>
#include <limits.h>

/*Hyperparams*/
#define PARAMS_population_size 10000
#define PARAMS_survival_size 1000
#define PARAMS_genotype_length 1600
#define PARAMS_mutation_rate 0.05
#define PARAMS_instance_per_phone_target 60
#define PARAMS_zero_padded_sentence_space 120000
#define PARAMS_epochs 100
#define PARAMS_parents 2
#define PARAMS_time_epochs 1
#define NUM_phones 399
#define PRINT_info 1

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
void init_random_indices();
void init_survivor_indices();
bool compare_cost(COST_AND_INDEX &a, COST_AND_INDEX &b);
int calc_loss(int genotype[]);
void sort_population_by_cost();
template <class T>
void GA_swap (T *a, T *b);
void yates_fischer_shuffle (int arr[], int n);
void mate(int (&parent1)[], int (&parent2)[], int (&child)[], int num_crossover_points);
int compare_ints(const void* a, const void* b);
void cost_curve_to_csv();
void best_genotype_to_csv();

/*GlobalVriables*/
int tag_distro_space[PARAMS_zero_padded_sentence_space][NUM_phones];
int GA_pool[PARAMS_population_size][PARAMS_genotype_length];
int random_indices[PARAMS_genotype_length-2];
int survivor_indices[PARAMS_survival_size];
int fittest_log[PARAMS_epochs];
int min_cost=INT_MAX;
int num_crossover;
int best_genotype[PARAMS_genotype_length];
COST_AND_INDEX cost_and_index[PARAMS_population_size];

void init_tag_distro_space() {
    for (int i=0; i<PARAMS_zero_padded_sentence_space; ++i) {
        for (int j=0; j<NUM_phones; ++j) {
            tag_distro_space[i][j] = 0;
        }
    }
    printf("tag_distro_space init to %d\n", 0);
}

void init_random_indices() {
    for (int i=0; i<PARAMS_genotype_length; ++i) {
        random_indices[i] = i;
    }
    printf("random_indices MAX_val is %d\n", random_indices[PARAMS_genotype_length-1]);
}

void init_survivor_indices() {
    for (int i=0; i<PARAMS_survival_size; ++i) {
        survivor_indices[i] = i;
    }
    printf("survivor_indices MAX_val is %d\n", survivor_indices[PARAMS_survival_size-1]);
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
    auto t1 = std::chrono::high_resolution_clock::now();
    /*write cost2individual alias*/
    for (int i=0; i<PARAMS_population_size; ++i) {
        cost_and_index[i].index = i;
        cost_and_index[i].cost =calc_loss(GA_pool[i]);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    printf("calc time: %f", ((float)duration/1000000));

    t1 = std::chrono::high_resolution_clock::now();
    /*SortAlias*/
    qsort(cost_and_index, PARAMS_population_size, sizeof(COST_AND_INDEX), compare_cost);
    t2 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    printf("qsort time: %f", ((float)duration/1000000));
}

template <class T>
void GA_swap (T *a, T *b) {  
    T temp = *a;  
    *a = *b;  
    *b = temp;  
}

void yates_fischer_shuffle (int arr[], int n) {
    for (int i = n - 1; i > 0; i--) {  
        int j = rand() % (i + 1);   
        GA_swap(&arr[i], &arr[j]);  
    }  
}

int compare_ints(const void* a, const void* b) {
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;
 
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

void mate(int (&parent1)[PARAMS_genotype_length], int (&parent2)[PARAMS_genotype_length], int (&child)[PARAMS_genotype_length], int num_crossover_points) {
    yates_fischer_shuffle(random_indices, PARAMS_genotype_length);
    
    /*init crossover-points indices*/
    int points[num_crossover_points+2]; //plus 0 and end
    points[0] = 0;
    points[num_crossover_points+1] = PARAMS_genotype_length-1;
    memcpy(&points[1], &random_indices[0], num_crossover_points*sizeof(int));

    /*sort the points to small-to-big order*/
    qsort(points, sizeof(points)/sizeof(*points), sizeof(points[0]), compare_ints);
    
    /*crossover*/
    for (int i=0; i<num_crossover_points+1; ++i) {
        if (i%2 == 0) {
            memcpy(&child[points[i]], &parent1[points[i]], (points[i+1]-points[i])*sizeof(int));
        }
        else {
            memcpy(&child[points[i]], &parent2[points[i]], (points[i+1]-points[i])*sizeof(int));
        }
    }
}

void mutate(int index) {
    int random_index = rand()%((int)(PARAMS_genotype_length-PARAMS_genotype_length*PARAMS_mutation_rate));
    int points[(int)(PARAMS_genotype_length*PARAMS_mutation_rate)];
    memcpy(&points[0], &random_indices[random_index], sizeof(points));

    for (int i=0; i<(int)(sizeof(points)/sizeof(int)); ++i) {
        GA_pool[index][points[i]] = (int)(rand()%PARAMS_zero_padded_sentence_space);
    }
}

void copy_genotype(int geno_dst[PARAMS_genotype_length], int geno_src[PARAMS_genotype_length]) {
    memcpy(&geno_dst[0], &geno_src[0], PARAMS_genotype_length*sizeof(geno_dst[0]));
}

void cost_curve_to_csv() {
    std::ofstream cost_curve_file;
    cost_curve_file.open ("cpp_GA_cost_log.csv");
    for (int i=0; i<PARAMS_epochs; ++i) {
        cost_curve_file << fittest_log[i] << '\n';
    }
    cost_curve_file.close();
    printf("\nCost curve output done.\n");
}

void best_genotype_to_csv() {
    std::ofstream genotype_file;
    genotype_file.open ("cpp_best_genotype.csv");
    for (int i=0; i<PARAMS_genotype_length; ++i) {
        genotype_file << best_genotype[i] << '\n';
    }
    genotype_file.close();
    printf("\nBest genotype output done.\n");
}

int main() {
    init_tag_distro_space();
    init_random_indices();
    init_survivor_indices();
    srand((unsigned int)time(NULL));
    LoadCSVdata();
    init_GA_pool();

    printf("calc_cost test %d\n", calc_loss(GA_pool[0]));
    
    for(int epoch=0; epoch<PARAMS_epochs; ++epoch) {
        /* time starts */
        auto t1 = std::chrono::high_resolution_clock::now();

        /* evaluate and sort population */
        sort_population_by_cost();
        fittest_log[epoch] = cost_and_index[0].cost;
        if (fittest_log[epoch]<min_cost) {
            min_cost = fittest_log[epoch];
            copy_genotype(best_genotype, GA_pool[cost_and_index[0].index]);
        }

        /* time stops */
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        printf("sort time: %f", ((float)duration/1000000));

        /* time starts */
        t1 = std::chrono::high_resolution_clock::now();

        /* mating */
        int parent1, parent2;
        for (int child=PARAMS_survival_size; child<PARAMS_population_size; ++child) {
            yates_fischer_shuffle(survivor_indices, PARAMS_survival_size);
            parent1 = survivor_indices[(int)(child % PARAMS_survival_size)];
            parent2 = survivor_indices[parent1];
            num_crossover = (int)(rand()%5 + 3);
            mate(GA_pool[cost_and_index[parent1].index], GA_pool[cost_and_index[parent2].index], GA_pool[cost_and_index[child].index], num_crossover);
        }

        /* time stops */
        t2 = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        printf("mate time: %f", ((float)duration/1000000));

        /* time starts */
        t1 = std::chrono::high_resolution_clock::now();

        /* mutate all */
        for (int individual=0; individual<PARAMS_population_size; ++individual) {
            mutate(individual);
        }

        /* time stops */
        t2 = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        printf("mutate time: %f", ((float)duration/1000000));

        

        if (epoch%PRINT_info == 0) {
            printf("\nMin cost of epoch %d : %d, time/epoch: %f sec", epoch, fittest_log[epoch], ((float)duration/1000000));
        }
    }

    cost_curve_to_csv();
    best_genotype_to_csv();
    printf("\nDone.\n");
    return 0;
}