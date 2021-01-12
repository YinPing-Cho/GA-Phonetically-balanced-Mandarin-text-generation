from ckiptagger import data_utils, construct_dictionary, WS
#data_utils.download_data_gdown("./") # gdrive-ckip
import re
import os
import pandas as pd
import numpy as np
from opencc import OpenCC
from pypinyin import pinyin, lazy_pinyin, Style
import matplotlib.pyplot as plt

import argparse

def num2pinyin(line):
    line = re.sub('[1]', '一', line)
    line = re.sub('[2]', '二', line)
    line = re.sub('[3]', '三', line)
    line = re.sub('[4]', '四', line)
    line = re.sub('[5]', '五', line)
    line = re.sub('[6]', '六', line)
    line = re.sub('[7]', '七', line)
    line = re.sub('[8]', '八', line)
    line = re.sub('[9]', '九', line)
    line = re.sub('[0]', '零', line)

    return line

def read_text2phones(in_filename, out_filename, num_lines_limit):
    cc = OpenCC('s2t')
    tscc = OpenCC('t2s')

    ws = WS("./data", disable_cuda=True)

    count = 0
    vowels = ['a', 'i', 'u', 'e', 'o']
    tones = ['1', '2', '3', '4']

    phone_combo_dict = {}

    with open(out_filename, 'w', encoding="utf8") as ostr:
        with open(in_filename, 'r', encoding="utf8") as istr:

            for line in istr:
                line = line.strip().split('|')
                nummer = line[0]
                line = line[1]

                line = re.sub('[ 	_*()!@#$abcdefghijklmnopqrstuvwxyz。（）！？＊＆＃＠「」———";‘’《》“”；]', '', line)
                line = num2pinyin(line)

                if line == '\n':
                    continue
                line = re.sub('\n', '', line)
                text = ''
                for word in line:
                    sub = ''
                    for w in word:
                        sub += w
                    text += sub
                line = cc.convert(text)
                foo = []
                foo.append(line)
                line = foo
                if (count == 3):
                    print(line)
                if line == '\n':
                    continue
                word_line_list = ws(line)

                text = nummer+'|'
                for word_line in word_line_list:
                    for word in word_line:
                        if word != '\n':
                            py = pinyin(word, style=Style.TONE3, heteronym=False)

                            combo_word = ''

                            for index, p in enumerate(py):
                                worda = ''
                                for psub in p:
                                    worda += psub

                                # check with phone alias
                                filtered_worda = re.sub('[^a-z]+', '', worda)
                                if len(filtered_worda) > 1:
                                    if filtered_worda not in phone_combo_dict:
                                        phone_combo_dict[filtered_worda] = 1
                                    else:
                                        phone_combo_dict[filtered_worda] += 1

                                # reconstruct word
                                if not(worda == ',' or worda == '，'):
                                    combo_word += worda+'+'

                            text += combo_word + ' '

                if count >= num_lines_limit:
                    break

                if (count % 100 == 0):
                    print(line)
                    print(text)

                line = text
                line = line.rstrip("\n")
                line += '\n'
                ostr.write(str(line))

                count+=1

    print(count, " lines.")
    fig, ax = plt.subplots()
    num_bins = len(phone_combo_dict)
    plt.bar(list(phone_combo_dict.keys()), phone_combo_dict.values())
    plt.show()

    print('There are {} unique CVC combos.'.format(num_bins))

    return phone_combo_dict

def Process(args):
    phone_combo_dict = read_text2phones(in_filename=args.in_filename, out_filename=args.out_filename, num_lines_limit=args.num_lines_limit)
    print(phone_combo_dict)
    df = pd.DataFrame(phone_combo_dict, index=[0])
    df.to_csv('phenotype_phones_dist_FINAL.csv', index=False)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--in_filename', type=str, default='1227_ULTRA_assembled_text.txt',
                        help="name of the input file (contents Chinese characters)")
    parser.add_argument('-o', '--out_filename', type=str, default='ASSMBD_PHONED.txt',
                        help="name of the output file (will content pinyins)")
    parser.add_argument('-n', '--num_lines_limit', type=int, default=np.Inf,
                        help="limit of number of lines to process")

    args = parser.parse_args()

    Process(args)