import re
import os
import numpy as np
import matplotlib.pyplot as plt

np.random.seed(1)

def listdir_nohidden(path):
    for f in os.listdir(path):
        if not f.startswith('.'):
            yield f

def Average(lst): 
    return sum(lst) / len(lst)

def There_is_English(sentence):
    return re.findall("[a-zA-Z]", sentence)
    
def splittext(texts):
    checklist=['。', '！', '？', '\n']
    output_texts = []
    texts = re.sub(' ', '', texts)
    texts = re.sub('　', '', texts)
    before = []
    for index, word in enumerate(texts):
        before.append(word)
        nextword = texts[(index + 1) % len(texts)]
        if (word in checklist and nextword != '」') or (word == '」' and texts[index-1] in checklist):
            if len(before) < 2:
                continue
            foo = ''
            for w in before:
                foo += w
            before.clear()
            output_texts.append(foo)
    return output_texts

sample_portion = 0
limit = np.inf
count = 1
sentence_length_upper_limit = 100
sentence_length_lower_limit = 10
sentence_lengths = []

text_dir = 'texts'
with open('ULTRA_SUBSET.txt', 'w', encoding="utf8") as ostr:
    for file in listdir_nohidden(text_dir):
        filename = os.fsdecode(file)
        if filename.endswith(".txt"):
            with open(os.path.join(text_dir, filename), 'r', encoding="utf8") as istr:
                        
                for paragraph in istr:
                    lines = splittext(paragraph)
                    for line in lines:

                        if (filename=='wiki.txt'):
                                sample_portion = 0.02
                        else:
                            sample_portion = 0.2
                                
                        if len(line) > sentence_length_lower_limit and np.random.rand() < sample_portion:
                            nummer = str(count).zfill(9)
                            line = line.rstrip("\n")

                            

                            if len(line) > sentence_length_upper_limit or There_is_English(line):
                                continue

                            sentence_lengths.append(len(line))

                            line = nummer + '|' + line

                            line += '\n'
                            ostr.write(str(line))
                            count += 1
                            print(line)
                        if count >= limit:
                            break
                    if count >= limit:
                        break

len_max = max(sentence_lengths)
len_min = min(sentence_lengths)
len_med = np.median(sentence_lengths)
len_std = np.std(sentence_lengths)

sum_l = 0
for l in sentence_lengths:
    sum_l += l

print("Total length: ", sum_l, " chars")
print("Average length: ", Average(sentence_lengths), " chars.")
print("Median length: ", len_med, " chars.")
print("STD length: ", len_std, " chars.")
print("Max length: ", len_max, " chars.")
print("Min length: ", len_min, " chars.")

fig, ax = plt.subplots()
num_bins = int(len_max-len_min)
n, bins, patches = ax.hist(sentence_lengths, num_bins)
plt.show()