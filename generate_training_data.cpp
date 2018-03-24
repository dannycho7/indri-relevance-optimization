#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <indri/QueryEnvironment.hpp>
#include "ReScoringEnvironment.h"

int main(int argc, char *argv[]) {
	int top_k;

	if (argc < 2) {
		std::cerr << "Wrong Arguments. Correct Usage: ./run_query <top_k> < <queries_file_name>" << std::endl;
		exit(1);
	} else {
		top_k = atoi(argv[1]);
	}

	indri::api::QueryEnvironment env;
	env.addIndex("./indri_cli/index");
	std::string in;
	
	while(std::getline(std::cin, in)) {
		std::string buf, query;
		int qid, numTerms = 1, rel = 0;

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
			double score = results[i].score;
			const char *docName = documentNames[i].c_str();
			
			double totalScore = bm25Scores[i].totalScore + tfIdfScores[i].totalScore + bm25Scores[i].maxScore + tfIdfScores[i].maxScore + bm25Scores[i].avgScore + tfIdfScores[i].avgScore;

			if (isnan(totalScore)) continue;

			printf("%i qid:%i 1:%.5f 2:%.5f 3:%.5f 4:%.5f 5:%.5f 6:%.5f #%s \n",
				rel,
				qid,
				bm25Scores[i].totalScore,
				tfIdfScores[i].totalScore,
				bm25Scores[i].maxScore,
				tfIdfScores[i].maxScore,
				bm25Scores[i].avgScore,
				tfIdfScores[i].avgScore,
				docName);
		}
	}
}