#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <chrono>

/*Hyperparams*/
#define PARAMS_population_size 100
#define PARAMS_survival_size 10
#define PARAMS_genotype_length 1000
#define PARAMS_mutation_rate 0.2
#define PARAMS_instance_per_phone_target 600
#define PARAMS_zero_padded_sentence_space 120000
#define PARAMS_epochs 10
#define PARAMS_parents 2
#define PARAMS_time_epochs 1
#define NUM_phones 399

using namespace std;

/*GlobalVriables*/
int tag_distro_space[PARAMS_zero_padded_sentence_space][NUM_phones] = {0};

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
            //if (row_buffer[i]==10) {
            //    printf("changeline\n");
            //}
            if (row_buffer[i] != 10 && row_buffer[i] != 32 && row_buffer[i] != 44 && row_buffer[i] != 13) {
                number_buffer[char_count] = row_buffer[i];
                char_count++;
            }
            else {
                number_buffer[char_count] = 'e';
                if (number_buffer[0] != 'e' && number_buffer[0] != 10 && number_buffer[0] != 32 && number_buffer[0] != 44 && number_buffer[0] != 13) {
                    //printf("int %d\n", chars2int(number_buffer));
                    tag_distro_space[row_count][row_index] = chars2int(number_buffer);
                    row_index++;
                }
                
                char_count = 0;

                if (row_buffer[i] == 10) {
                    printf("Row_num %d. Phones in this row: %d\n", row_count, row_index);
                    row_count++;
                    continue;
                }
            }
            //printf("char %c\n", row_buffer[i]);
        }

        //if (row_count==2) {
        //    break;
        //}
        //printf("%400d\n", *tag_distro_space[row_count]);
    }
    //char test[10];
    //test[0] = '1';
    //test[1] = '0';
    //test[2] = 'e';
    //printf("test %d", chars2int(test));
}

int main() {
    LoadCSVdata();
    
    return 0;
}