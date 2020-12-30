import csv
import numpy as numpy

import csv


genotype_csv_filename = 'ULTRA_cpp_best_genotype.csv'
all_sentences_filename = 'ULTRA_SUBSET.txt'
out_filename = 'ULTRA_assembled_text.txt'

genotype_best = []
all_sentences_dict = {}
num_valid_sentences = 0
num_output_sentences = 0
num_chars = 0

with open(genotype_csv_filename, newline='') as csvfile:
    genotype_csv = csv.reader(csvfile, delimiter=' ', quotechar='|')
    for row in genotype_csv:
        nummer = str(int(row[0])-1).zfill(9)
        print(nummer)
        genotype_best.append(nummer)

print("Genotype length: {}".format(len(genotype_best)))

with open(all_sentences_filename, 'r', encoding="utf8") as istr:
    for line in istr:
        line = line.strip().split('|')
        tag= line[0]
        sentence = line[1]

        if (tag in all_sentences_dict):
            raise RuntimeError('Duplicated sentence tag')

        all_sentences_dict[tag] = sentence
        num_valid_sentences += 1

print("Valid sentences: {}".format(num_valid_sentences))

with open(out_filename, 'w', encoding="utf8") as ostr:
    for geno in genotype_best:
        tag = geno

        if tag not in all_sentences_dict:
            continue

        sentence = all_sentences_dict[tag]
        num_chars += len(sentence)
        ostr.write(tag+'|'+sentence+'\n')
        num_output_sentences += 1

print("Final text contains {} sentences, with {} chaaracters.".format(num_output_sentences, num_chars))
