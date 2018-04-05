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

	if (argc < 3) {
		std::cerr << "Wrong Arguments. Correct Usage: ./run_query <top_k> <qrels_filename> < <queries_file_name>" << std::endl;
		exit(1);
	} else {
		top_k = atoi(argv[1]);
		qrels_filename = argv[2];
	}

	std::set<std::string> reldocs;
	generateRelDocs(qrels_filename, reldocs);

	indri::api::QueryEnvironment env;
	env.addIndex("./indri_cli/index");
	std::string in;
		
	while(std::getline(std::cin, in)) {
		std::string buf, query;
		int qid, numTerms = 1;

		std::stringstream in_stream(in);
		in_stream >> qid;
		in_stream >> query;

		while (in_stream >> buf) {
			query += " " + buf;
			numTerms++;
		}

		std::vector<indri::api::ScoredExtentResult> results = env.runQuery(query, top_k);
		std::vector<std::string> documentNames = env.documentMetadata(results, "docno");
		
		ReScoringEnvironment *scorer = new ReScoringEnvironment(env, results);
		SCOREOUTPUT *tfIdfScores = scorer->multiTermTfIdf(query, numTerms);
		SCOREOUTPUT *bm25Scores = scorer->bm25(query, numTerms);

		for(int i = 0; i < results.size(); i++) {
			int rel = 0;
			double score = results[i].score;
			const char *docName = documentNames[i].c_str();
			
			double totalScore = bm25Scores[i].totalScore + tfIdfScores[i].totalScore + bm25Scores[i].maxScore + tfIdfScores[i].maxScore + bm25Scores[i].avgScore + tfIdfScores[i].avgScore;

			if (isnan(totalScore)) continue;

			std::ostringstream reldocs_key;

			reldocs_key << qid << "|" << docName;

			if (reldocs.find(reldocs_key.str()) != reldocs.end()) {
				rel = 1;
			}

			printf("%i qid:%i 1:%.5f 2:%.5f 3:%.5f 4:%.5f 5:%.5f 6:%.5f 7:%.5f 8:%.5f #%s \n",
				rel,
				qid,
				bm25Scores[i].totalScore,
				tfIdfScores[i].totalScore,
				bm25Scores[i].maxScore,
				tfIdfScores[i].maxScore,
				bm25Scores[i].minScore,
				tfIdfScores[i].minScore,
				bm25Scores[i].avgScore,
				tfIdfScores[i].avgScore,
				docName);
		}
	}
}
