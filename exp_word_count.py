import csv
import re
import copy
import numpy as np
from numba import jit
'''
all_valid_phonemes = ['a', 'ai', 'an', 'ang', 'ao', 'ba', 'bai', 'ban', 'bang', 'bao', 'bei', 'ben', 'beng', 'bi', 'bian', 'biao', 'bie', 'bin', 'bing'
    'bo', 'bu', 'ca', 'cai', 'can', 'cang', 'cao', 'ce', 'cen', 'ceng', 'cha', 'chai', 'chan', 'chang', 'chao', 'che', 'chen', 'cheng',\
    'chi', 'chong', 'chou', 'chu', 'chua', 'chuai', 'chuan', 'chuang', 'chui', 'chun', 'chuo', 'ci', 'cong', 'cou', 'cu', 'cuan', 'cui', 'cun',\
    'cuo', 'da', 'dai', 'dan', 'dang', 'dao', 'de', 'dei', 'deng', 'di', 'dian', 'diang', 'diao']
'''

def dict_string2int(dicto, set_value=None):
    for key in dicto.keys():
        if set_value is None:
            dicto[key] = int(dicto[key])
        else:
            dicto[key] = 0

def sentence2stats(line, phones_dict_per_line):
    nummer, line = line.strip().split('|')
    split_line = line.strip().split('+')

    dicto = phones_dict_per_line.copy()

    for word in split_line:
        filtered_word = re.sub('[^a-z]+', '', word)
        if filtered_word in dicto:
            dicto[filtered_word] += 1
    
    index = [nummer]
    index.extend([v for v in dicto.values()])
    index = [index]
    
    return index

def sort_by_phoneme_neighborhood(lista, num_phonemes_care):
    len_line = len(lista[0])
    np_lista = np.asarray(lista, dtype=np.float)
    np_lista = np_lista[:, 1:min(num_phonemes_care, len_line)]

    sum_each_line = np.sum(np_lista, axis=1)
    print(sum_each_line)
    index_sorted = np.argsort(sum_each_line)
    print(index_sorted)
    index_sorted = np.flip(index_sorted, axis=0)
    print(index_sorted)
    #print(index_longest, sum_each_line[index_longest])

    temp_list = copy.deepcopy(lista)
    for i in range(0, len(lista)):
        lista[i] = temp_list[index_sorted[i]]

write_limit = 6000000
num_phonemes_care = 396
in_filename = 'BIG_PHONED.txt'
out_filename = 'exp_word_count_BIG_sentence_phoneDist.csv'
out_file = open(out_filename, 'w', newline ='')

cpp_filename = 'cpp_exp_word_count_BIG_sentence_phoneDist.csv'
cpp_file = open(cpp_filename, 'w', newline ='')

reader = csv.DictReader(open('ULTRA_phones_dist_master.csv'))
phones_dict_master = next(reader)

dict_string2int(phones_dict_master)
phones_dict_master = {k: v for k, v in sorted(phones_dict_master.items(), key=lambda item: item[1], reverse=True)}
phones_dict_per_line = phones_dict_master.copy()
dict_string2int(phones_dict_per_line, set_value=0)

print(phones_dict_master)
print(phones_dict_per_line)


all_sentences = []
# write index
with out_file:
    phone_index = ['tag']
    phone_index.extend([v for v in phones_dict_master.keys()])
    phone_index = [phone_index]
    print(phone_index)

    write = csv.writer(out_file)
    write.writerows(phone_index)

    with open(in_filename, 'r', encoding="utf8") as istr:
        count = 0
        for line in istr:
            if count >= write_limit:
                break
            phone_distro = sentence2stats(line=line, phones_dict_per_line=phones_dict_per_line)
            all_sentences.append(phone_distro[0])
            count += 1

        sort_by_phoneme_neighborhood(all_sentences, num_phonemes_care=num_phonemes_care)

        for line in all_sentences:
            write = csv.writer(out_file)
            write.writerows([line])

with cpp_file:
    for line in all_sentences:
        write = csv.writer(cpp_file)
        write.writerows([line[1:]])
            

#print(all_sentences)

print('Wrote {} lines.'.format(count))