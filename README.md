# GA-Phonetically-balanced-Mandarin-text-generation (BUILDING...)
Generate phonetically balanced Mandarin script with Wiki-Database and GA (genetic algorithm) for more efficient TTS (text-to-speech) training data preparation.

## Run instruction:
1. Sample a subset of the original corpus:
  - `:python3 get.py`

2. Convert the Chinese characters to phonetic representations:
  - `:python3 text2phones.py`
  
3. Convert each sentence to a distribution of CVC combinations:
  - `:python3 sentences2tag_stats.py`
  
4. Run GA:
  - `:make`
  - `:./main`
  
5. Assemble the sentences specified by the final genotype:
  - `:python3 assemble.py`
