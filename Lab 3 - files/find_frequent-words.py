from pathlib import Path
import time
import string

start_time_total = time.time()
start_time_seq_1 = time.time()
file_path = Path("lusiadas-words.txt")

file = open(file_path, 'r')

all_words = []
for line in file:
    all_words.append(line.lower())
    all_words.append(line.lower())
    all_words.append(line.lower())
    all_words.append(line.lower())
    # yes, we are inserting the words four times ;)

print(len(all_words))
words_lists = {}
for c in string.ascii_lowercase:
    words_lists[c] = {}
end_time_seq_1 = time.time()

start_time_par_1 = time.time()
for word in all_words:
    start_c = word[0]
    if start_c.isalpha():
        if word in words_lists[start_c ]:
            words_lists[start_c ][word] += 1
        else:
            words_lists[start_c ][word] = 1
end_time_par_1 = time.time()

start_time_par_2 = time.time()
for start_c in string.ascii_lowercase:
    more_freq_word = ""
    count = 0
    for word in words_lists[start_c ]:
        if words_lists[start_c ][word] > count:
            count = words_lists[start_c ][word]
            more_freq_word = word
    print (more_freq_word, count)
end_time_par_2 = time.time()

end_time_total = time.time()
print("\tseq 1 \t %f"%( end_time_seq_1 -  start_time_seq_1, ));
print("\tpar 1 \t %f"%( end_time_par_1 -  start_time_par_1, ));
print("\tpar 2 \t %f"%( end_time_par_2 -  start_time_par_2, ));

print("total \t %f"%( end_time_total-  start_time_total, ));