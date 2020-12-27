import numpy as np
import csv

phenotype_distro_csv_filename = 'ULTRA_cpp_best_pheno_distro.csv'
info_filename = 'phenotype_info.txt'

phenotype_distro = []
num_discard_head_tail = [0, 10, 25, 50, 75, 100, 125, 150]

def discard_samples(array_in, num_discard):
    foo = array_in[num_discard:]
    foo = foo[:len(foo)-num_discard]
    return foo


with open(phenotype_distro_csv_filename, newline='') as csvfile:
    phenodistro_csv = csv.reader(csvfile, delimiter=' ', quotechar='|')
    for row in phenodistro_csv:
        phenotype_distro.append(int(row[0]))

phenotype_distro.sort()


with open(info_filename, 'w', encoding="utf8") as ostr:
    for d_num in num_discard_head_tail:
        phenotype_distro_truncated = np.asarray(discard_samples(phenotype_distro, d_num))
        ostr.write("Mean after discard {} phones: {}.\n".format(2*d_num, np.mean(phenotype_distro_truncated)))
        ostr.write("STD after discard {} phones: {}.\n".format(2*d_num, np.std(phenotype_distro_truncated)))
        ostr.write("MAX_num_phone after discard {} phones: {}.\n".format(2*d_num, np.max(phenotype_distro_truncated)))
        ostr.write("MIN_num_phone after discard {} phones: {}.\n\n".format(2*d_num, np.min(phenotype_distro_truncated)))