# Datasets
## Wordcount
The currently chosen dataset is Amazon Reviews for Sentiment Analysis (
[Link](https://www.kaggle.com/bittlingmayer/amazonreviews) ).  
The dataset however has some unnecessary data and unicode characters, for this reason it is filtered using ```process_amazon.py```

Usage
```
python process_amazon.py test.ft.txt
python process_amaton.py train.ft.txt
```

## Hashjoin
Consists of two files:
 - ``` hash_join_smaller.txt ```
 - ``` hash_join_larger.txt ```

Both of these files can be generated from any text files (with multiple lines) using ``` .\create_hash_join_data.py <input_file> <smaller_num_lines> <larger_num_lines> ``` . By default uses the preprocessed amazon dataset.

The datasets follow these formats:
 - smaller: ``` PK1;DATA ``` | PK = int, DATA = string
 - larger:  ``` PK2;FK1;DATA ``` | (FK1 ∈ smaller(PK1))

