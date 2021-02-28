import re
import os
import numpy as np
import matplotlib.pyplot as plt
import docx

np.random.seed(1)

def listdir_nohidden(path):
    for f in os.listdir(path):
        if not f.startswith('.'):
            yield f

def Average(lst): 
    return sum(lst) / len(lst)

def There_is_English(sentence):
    return re.findall("[a-zA-Zａ-ｚＡ-Ｚ]", sentence)

def There_is_Japanese(sentence):
    return re.findall("[ぁ-ゟ゠-ヿ]", sentence)

    
def splittext(texts):
    checklist=['。', '！', '？', '；', '!', '?', ';', '\n']
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

wiki_sample_portion = 0.05
literature_sample_portion = 0.9
limit = 600000
count = 1
sentence_length_upper_limit = 100
sentence_length_lower_limit = 5
sentence_lengths = []

text_dir = r'.\docs'
with open(r'.\assets\KKBOOKS.txt', 'w', encoding="utf8") as ostr:
    stored_sentences = []
    for file in listdir_nohidden(text_dir):
        filename = os.fsdecode(file)
        if filename.endswith(".docx"):
            DOC = docx.Document(os.path.join(text_dir, filename))
                    
            for paragraph in DOC.paragraphs:
                lines = splittext(paragraph.text)
                for line in lines:

                    if filename == 'zz_wiki.txt':
                        sample_portion = wiki_sample_portion
                    else:
                        sample_portion = literature_sample_portion
                            
                    if len(line) > sentence_length_lower_limit and np.random.rand() < sample_portion:
                        nummer = str(count).zfill(9)
                        line = line.replace('\n','')

                        if len(line) < sentence_length_lower_limit or len(line) > sentence_length_upper_limit or There_is_English(line) or There_is_Japanese(line):
                            continue

                        sentence_lengths.append(len(line))

                        if line not in stored_sentences:
                            stored_sentences.append(line)
                            line = nummer + '|' + line

                            line += '\n'
                            ostr.write(str(line))
                            
                            count += 1
                            print(line)
                    if count > limit:
                        break
                if count > limit:
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