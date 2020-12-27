#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <chrono>
#include <fstream>
#include <limits.h>
#include <pthread.h>

/*THIS IS FOR ULTRA*/
/*Hyperparams*/
#define PARAMS_population_size 24000
#define PARAMS_survival_size 7200
#define PARAMS_num_elites 2400
#define PARAMS_genotype_length 1000
#define PARAMS_mutation_rate_DEFAULT 0.0001
#define PARAMS_instance_per_phone_target 60
#define PARAMS_full_total_word_constraint_starts 0.05
#define PARAMS_full_total_word_constraint_steady 0.1
#define PARAMS_real_sentence_space 229343
#define PARAMS_zero_padded_sentence_space 250000
#define PARAMS_SAMPLING_FACTOR 50
#define PARAMS_HEAT_FACTOR 10
#define PARAMS_epochs 10000
#define PARAMS_MAX_NUM_crossover_points 5
#define PARAMS_MIN_NUM_crossover_points 2
#define OUTPUT_epochs 100
#define PRINT_info 1
#define NUM_phones 400
#define NUM_THREADS 12
#define NUM_syllables_target 24500
#define NUM_syllable_error_allowance 500
#define L1(X)            (abs(X-NUM_syllables_target))
#define L2(X)            (L1(X)*L1(X))
#define LooseL2loss(X)   ( L2(X)*(L1(X)>NUM_syllable_error_allowance) )

using namespace std;

/*Struct*/
struct COST_AND_INDEX {
    int cost;
    int index;
};

/*FunctionDeclaration*/
void reset_best_phenotype_distribution();
void init_tag_distro_space();
int chars2int(char ch[]);
void LoadCSVdata();
void init_GA_pool();
void init_random_indices();
void init_survivor_indices();
bool compare_cost(COST_AND_INDEX &a, COST_AND_INDEX &b);
void* cost2alias(void* arg);
int calc_loss(int genotype[]);
void sort_population_by_cost();
template <class T>
void GA_swap (T *a, T *b);
void yates_fischer_shuffle (int arr[], int n);
void mate(int (&parent1)[PARAMS_genotype_length], int (&parent2)[PARAMS_genotype_length], int (&parent3)[PARAMS_genotype_length], int (&child)[PARAMS_genotype_length],\
    int thread_part, int num_crossover_points, int rnd_index);
void* mutate_partial(void* arg);
void mutate();
int compare_ints(const void* a, const void* b);
void calc_best_phenotype_distribution(int genotype[]);
void cost_curve_to_csv();
void best_genotype_to_csv();
void best_phenotype_distribution_to_csv();
void copy_genotype(int geno_dst[PARAMS_genotype_length], int geno_src[PARAMS_genotype_length]);

/*GlobalVriables*/
int GLOBAL_EPOCH=0;
float PARAMS_mutation_rate = PARAMS_mutation_rate_DEFAULT;
int THREAD_PART = 0;
int tag_distro_space[PARAMS_zero_padded_sentence_space][NUM_phones];
int GA_pool[PARAMS_population_size][PARAMS_genotype_length];
int TEMP_pool[PARAMS_population_size][PARAMS_genotype_length];
int random_indices[NUM_THREADS][PARAMS_genotype_length-2];
int survivor_indices[NUM_THREADS][PARAMS_survival_size];
int fittest_log[PARAMS_epochs];
int min_cost=INT_MAX;
int best_genotype[PARAMS_genotype_length];
int best_phenotype_distribution[NUM_phones]={};
COST_AND_INDEX cost_and_index[PARAMS_population_size];

void reset_best_phenotype_distribution() {
    for (int i=0; i < NUM_phones; ++i) best_phenotype_distribution[i] = 0;
}

void init_tag_distro_space() {
    for (int i=0; i<PARAMS_zero_padded_sentence_space; ++i) {
        for (int j=0; j<NUM_phones; ++j) {
            tag_distro_space[i][j] = 0;
        }
    }
    printf("tag_distro_space init to %d\n", 0);
}

void init_random_indices() {
    for (int j=0; j<NUM_THREADS; ++j) {
        for (int i=0; i<PARAMS_genotype_length; ++i) {
        random_indices[j][i] = i;
        }
    }
    printf("random_indices MAX_val is %d\n", random_indices[NUM_THREADS-1][PARAMS_genotype_length-1]);
}

void init_survivor_indices() {
    for (int j=0; j<NUM_THREADS; ++j) {
        for (int i=0; i<PARAMS_survival_size; ++i) {
            survivor_indices[j][i] = i;
        }
    }
    printf("survivor_indices MAX_val is %d\n", survivor_indices[NUM_THREADS-1][PARAMS_survival_size-1]);
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
    
    FILE* stream = fopen("ULTRA_cpp_sentence_phoneDist.csv", "r");

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
        cost += abs(PARAMS_instance_per_phone_target-pheno_distribution[i]);
    }
    float suppress_ratio = ((float)GLOBAL_EPOCH/(float)PARAMS_epochs*PARAMS_full_total_word_constraint_starts);
    cost += (int)( ((suppress_ratio>1.0) ? 1.0 : suppress_ratio) * LooseL2loss(num_syl) );
    return cost;
}

void* cost2alias(void* arg) { 
    int thread_part = THREAD_PART++; 
  
    for (int i = thread_part * (PARAMS_population_size / NUM_THREADS); i < (thread_part + 1) * (PARAMS_population_size / NUM_THREADS); i++) {
        cost_and_index[i].index = i;
        cost_and_index[i].cost = calc_loss(GA_pool[i]);
    }

    return(NULL);
}

void sort_population_by_cost() {
    /*write cost2individual alias*/
    /*create threads and assign task*/
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, cost2alias, (void*)NULL);
  
    /* join threads */
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    THREAD_PART = 0;

    /*SortAlias*/
    qsort(cost_and_index, PARAMS_population_size, sizeof(COST_AND_INDEX), compare_cost);
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

void mate(int (&parent1)[PARAMS_genotype_length], int (&parent2)[PARAMS_genotype_length], int (&parent3)[PARAMS_genotype_length], int (&child)[PARAMS_genotype_length],\
int thread_part, int num_crossover_points, int rnd_index) {
    
    /*init crossover-points indices*/
    int points[num_crossover_points+2]; //plus 0 and end
    points[0] = 0;
    points[num_crossover_points+1] = PARAMS_genotype_length-1;
    memcpy(&points[1], &random_indices[thread_part][rnd_index % (PARAMS_genotype_length-2) - num_crossover_points], num_crossover_points*sizeof(int));

    /*sort the points to small-to-big order*/
    qsort(points, sizeof(points)/sizeof(*points), sizeof(points[0]), compare_ints);
    
    /*crossover*/
    for (int i=0; i<num_crossover_points+1; ++i) {
        if (i%3 == 0) {
            memcpy(&child[points[i]], &parent1[points[i]], (points[i+1]-points[i])*sizeof(int));
        }
        else if (i%3 == 1) {
            memcpy(&child[points[i]], &parent2[points[i]], (points[i+1]-points[i])*sizeof(int));
        }
        else {
            memcpy(&child[points[i]], &parent3[points[i]], (points[i+1]-points[i])*sizeof(int));
        }
    }
}

void* mate_partial(void* arg) {
    int thread_part = THREAD_PART++;
    int parent1, parent2, parent3;
    int num_crossover = 0;

    yates_fischer_shuffle(survivor_indices[thread_part], PARAMS_survival_size);

    /* replace non-survivors */
    for (int child = (thread_part+1) * ((PARAMS_population_size-PARAMS_survival_size) / NUM_THREADS) + PARAMS_survival_size-1; \
    child >= (thread_part) * ((PARAMS_population_size-PARAMS_survival_size) / NUM_THREADS) + PARAMS_survival_size; --child) {
        parent1 = (int)(rand() % PARAMS_survival_size);
        parent2 = survivor_indices[thread_part][child % PARAMS_survival_size];
        parent3 = survivor_indices[thread_part][parent2];
        num_crossover = (int)(rand()%(PARAMS_MAX_NUM_crossover_points-PARAMS_MIN_NUM_crossover_points) + PARAMS_MIN_NUM_crossover_points);

        if (child % 10 == 0) yates_fischer_shuffle(random_indices[thread_part], PARAMS_genotype_length);

        mate(GA_pool[cost_and_index[parent1].index], GA_pool[cost_and_index[parent2].index], GA_pool[cost_and_index[parent3].index],\
         GA_pool[cost_and_index[child].index], thread_part, num_crossover, child);
    }

    yates_fischer_shuffle(survivor_indices[thread_part], PARAMS_survival_size);

    /* replace survivors */
    for (int child = (thread_part+1) * ((PARAMS_survival_size-PARAMS_num_elites) / NUM_THREADS) + PARAMS_num_elites-1; \
    child >= (thread_part) * ((PARAMS_survival_size-PARAMS_num_elites) / NUM_THREADS) + PARAMS_num_elites; --child) {
        parent1 = (int)(rand() % PARAMS_num_elites);
        parent2 = survivor_indices[thread_part][child % PARAMS_survival_size];
        parent3 = survivor_indices[thread_part][parent2];
        num_crossover = (int)(rand()%(PARAMS_MAX_NUM_crossover_points-PARAMS_MIN_NUM_crossover_points) + PARAMS_MIN_NUM_crossover_points);

        if (child % 10 == 0) yates_fischer_shuffle(random_indices[thread_part], PARAMS_genotype_length);

        mate(GA_pool[cost_and_index[parent1].index], GA_pool[cost_and_index[parent2].index], GA_pool[cost_and_index[parent3].index],\
         GA_pool[cost_and_index[child].index], thread_part, num_crossover, child);
    }
    return(NULL);
}

void mate_task() {
    /*create threads and assign task*/
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, mate_partial, (void*)NULL);
  
    /* join threads */
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    THREAD_PART = 0;
}

void* mutate_partial(void* arg) { 
    int thread_part = THREAD_PART++;
        
    for (int i = thread_part * (PARAMS_population_size / NUM_THREADS); i < (thread_part + 1) * (PARAMS_population_size / NUM_THREADS); i++) {
        float local_mutation_rate = PARAMS_mutation_rate * (float)(rand()%PARAMS_SAMPLING_FACTOR);
        int random_index = rand()%((int)(PARAMS_genotype_length-PARAMS_genotype_length*local_mutation_rate));
        int points[(int)(PARAMS_genotype_length*local_mutation_rate)];

        if (i % 10 == 0) yates_fischer_shuffle(random_indices[thread_part], PARAMS_genotype_length-2);

        memcpy(&points[0], &random_indices[thread_part][random_index], sizeof(points));

        for (int j=0; j<(int)(sizeof(points)/sizeof(int)); ++j) {
            GA_pool[i][points[j]] = (int)(rand()%PARAMS_zero_padded_sentence_space);
        }
    }

    return(NULL);
}

void mutate() {
    /*create threads and assign task*/
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&threads[i], NULL, mutate_partial, (void*)NULL);
  
    /* join threads */
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
    THREAD_PART = 0;
}

void calc_best_phenotype_distribution(int genotype[]) {
    reset_best_phenotype_distribution();
    int num_syl = 0;
    int num_null_sentences = 0;

    for (int i=0; i<PARAMS_genotype_length; ++i) {
        num_null_sentences += (genotype[i]>PARAMS_real_sentence_space-1);

        for (int j=0; j<NUM_phones; ++j) {
            num_syl += tag_distro_space[(int)genotype[i]][j];
            best_phenotype_distribution[j] += tag_distro_space[(int)genotype[i]][j];
        }
        if (genotype[i]>PARAMS_zero_padded_sentence_space-1) {
            printf("genotype overflow");
        }
    }
    printf("\nBest phenotype has %d syllables.\n", num_syl);
    printf("Num null sentences: %d", num_null_sentences);
}

void cost_curve_to_csv() {
    std::ofstream cost_curve_file;
    cost_curve_file.open ("ULTRA_cpp_GA_cost_log.csv");
    for (int i=0; i<PARAMS_epochs; ++i) {
        cost_curve_file << fittest_log[i] << '\n';
    }
    cost_curve_file.close();
    printf("\nCost curve output done.\n");
}

void best_genotype_to_csv() {
    std::ofstream genotype_file;
    genotype_file.open ("ULTRA_cpp_best_genotype.csv");
    for (int i=0; i<PARAMS_genotype_length; ++i) {
        genotype_file << best_genotype[i] << '\n';
    }
    genotype_file.close();
    printf("Best genotype output done.\n");
}

void best_phenotype_distribution_to_csv() {
    std::ofstream phenotype_distribution_file;
    phenotype_distribution_file.open ("ULTRA_cpp_best_pheno_distro.csv");
    for (int i=0; i<NUM_phones; ++i) {
        phenotype_distribution_file << best_phenotype_distribution[i] << '\n';
    }
    phenotype_distribution_file.close();
    printf("Best phenotype distribution output done.\n");
}

void copy_genotype(int geno_dst[PARAMS_genotype_length], int geno_src[PARAMS_genotype_length]) {
    memcpy(&geno_dst[0], &geno_src[0], PARAMS_genotype_length*sizeof(geno_dst[0]));
}

int main() {
    init_tag_distro_space();
    init_random_indices();
    init_survivor_indices();
    srand((unsigned int)time(NULL));
    LoadCSVdata();
    init_GA_pool();

    printf("calc_cost test %d\n", calc_loss(GA_pool[0]));
    copy_genotype(best_genotype, GA_pool[0]);
    printf("Epoch 0 status: ");
    calc_best_phenotype_distribution(best_genotype);
    printf("\n");

    printf("\n** This is the ULTRA version. **\n** MAY THE FORCE BE WITH YOUR INITIALIZATION **\n");
    for(int epoch=0; epoch<PARAMS_epochs; ++epoch) {
        GLOBAL_EPOCH = epoch;
        /* time starts */
        auto t1 = std::chrono::high_resolution_clock::now();

        /* evaluate and sort population */
        sort_population_by_cost();
        fittest_log[epoch] = cost_and_index[0].cost;
        
        if (epoch > PARAMS_epochs*PARAMS_full_total_word_constraint_steady) {
            if (fittest_log[epoch]<min_cost) {
                min_cost = fittest_log[epoch];
                copy_genotype(best_genotype, GA_pool[cost_and_index[0].index]);
            }
        }

        /* mating */
        mate_task();

        /* mutate all */
        if (epoch>(int)((float)PARAMS_epochs*(float)PARAMS_full_total_word_constraint_starts)) {
            PARAMS_mutation_rate = PARAMS_mutation_rate_DEFAULT;
        }
        else {
            PARAMS_mutation_rate = PARAMS_mutation_rate_DEFAULT*PARAMS_HEAT_FACTOR;
        }
        mutate();

        /* time stops */
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();

        if (epoch%PRINT_info == 0) {
            printf(" Min cost of epoch %d: %d, secs/epoch: %f\n", epoch, fittest_log[epoch], ((float)duration/1000000));
        }

        if (epoch != 0 && epoch%OUTPUT_epochs == 0) {
            calc_best_phenotype_distribution(best_genotype);
            cost_curve_to_csv();
            best_genotype_to_csv();
            best_phenotype_distribution_to_csv();
        }
    }

    calc_best_phenotype_distribution(best_genotype);
    cost_curve_to_csv();
    best_genotype_to_csv();
    best_phenotype_distribution_to_csv();
    printf("\nDone.\n");
    return 0;
}
