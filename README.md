# Indri Relevance Score Optimization

## Installation

1. Install indri and set packages/ as the install directory
2. Install the RankLib Java jar 

## Usage

Sample generation of training data:
```bash
make && ./generate_training_data <num_doc_results> <qrels_file> < <query_list_file_path> > <save_file_path>
```

Generating new NDCG@20 scores based on training data:
```bash
java -jar ./packages/RankLib.jar -train <training_data_path> -silent -kcv 5 -metric2t NDCG@20 -qrel <qrel_file_path> -ranker 0 > ./sample_data/results/MART
```
