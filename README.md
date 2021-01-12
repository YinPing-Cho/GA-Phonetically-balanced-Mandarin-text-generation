# GA-Phonetically-balanced-Mandarin-text-generation (BUILDING...)
Generate phonetically balanced Mandarin script with Wiki-Database and GA (genetic algorithm) for more efficient TTS (text-to-speech) training data preparation.

## Run instruction:
1. Sample a subset of the original corpus:
  - `:python3 sample_text.py`

2. Convert the Chinese characters to phonetic representations:
  - `:python3 text2phones.py`
  
3. Convert each sentence to a distribution of CVC combinations:
  - `:python3 sentences2tag_stats.py`

4. (Optional) Sort the search space by sentence length for faster and better GA convergence:
  - `:python3 exp_word_count.py`
  
5. Run GA:
  - `:make`
  - `:./main`
  
6. Assemble the sentences specified by the final genotype:
  - `:python3 assemble.py`

7. Verified the assembled text's CVC distribution:
  - `:python3 assmbd_text2phones.py`
