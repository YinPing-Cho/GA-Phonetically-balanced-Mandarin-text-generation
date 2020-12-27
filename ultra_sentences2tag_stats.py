import csv
import re

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

reader = csv.DictReader(open('ULTRA_phones_dist_master.csv'))
phones_dict_master = next(reader)
phones_dict_per_line = phones_dict_master.copy()

dict_string2int(phones_dict_master)
dict_string2int(phones_dict_per_line, set_value=0)

in_filename = 'ULTRA_PHONED.txt'
out_filename = 'ULTRA_sentence_phoneDist.csv'
out_file = open(out_filename, 'w', newline ='')


print(phones_dict_master)
print(phones_dict_per_line)

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
            #if count >= 100:
            #    break
            phone_distro = sentence2stats(line=line, phones_dict_per_line=phones_dict_per_line)

            write = csv.writer(out_file)
            write.writerows(phone_distro)
            count += 1

print('Wrote {} lines.'.format(count))