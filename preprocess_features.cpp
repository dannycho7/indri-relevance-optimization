#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <vector>
#include <indri/QueryEnvironment.hpp>
#include "ReScoringEnvironment.h"

void generateRelDocs(std::string filename, std::set<std::string> &reldocs) {
	std::ifstream qrelsfile;
	qrelsfile.open(filename.c_str());

	if (!qrelsfile) {
		std::cerr << "Incorrect qrels filename" << std::endl;
		exit(1);
	}

	std::string qrel_line;
	while (std::getline(qrelsfile, qrel_line)) {
		std::string docid;
		int relevance, qid, ignore;
		
		std::stringstream xtract_qrel(qrel_line);
		xtract_qrel >> qid >> ignore >> docid >> relevance;
		if (relevance == 1) {
			std::ostringstream reldocs_key;
			reldocs_key << qid << "|" << docid;
			reldocs.insert(reldocs_key.str());
		}
	}

}

int main(int argc, char *argv[]) {
	int top_k;
	std::string qrels_filename;

	if (argc != 2) {
		std::cerr << "You must specify the qrels file. Correct Usage: ./preprocess_features <qrels_filename> < <feature_list_filename>" << std::endl;
		exit(1);
	}

	qrels_filename = argv[1];

	std::set<std::string> reldocs;
	generateRelDocs(qrels_filename, reldocs);

	std::string in;
		
	while(std::getline(std::cin, in)) {
		std::string buf, docName, feature_scores;
		int qid, numTerms = 1;

		std::stringstream in_stream(in);
		in_stream >> qid;
		in_stream >> docName;
		std::getline(in_stream, feature_scores);		

		int rel = 0;

		std::ostringstream reldocs_key;

		reldocs_key << qid << "|" << docName;

		if (reldocs.find(reldocs_key.str()) != reldocs.end()) {
			rel = 1;
		}

		printf("%i qid:%i%s #%s \n",
			rel,
			qid,
			feature_scores.c_str(),
			docName.c_str());
	}
}
