#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <indri/QueryEnvironment.hpp>
#include "ReScoringEnvironment.h"

int main(int argc, char *argv[]) {
	std::string query;
	int top_k;

	if (argc < 3) {
		std::cerr << "Wrong Arguments. Correct Usage: ./run_query <top_k> <query>" << std::endl;
		exit(1);
	} else {
		top_k = atoi(argv[1]);
		query = std::string(argv[2]);

		// append any extra terms
		for (int i = 3; i < argc; i++) {
			query += " " + std::string(argv[i]);
		}
	}


	indri::api::QueryEnvironment env;
	env.addIndex("./indri_cli/index");

	std::vector<indri::api::ScoredExtentResult> results = env.runQuery(query, top_k);
	std::vector<std::string> documentNames = env.documentMetadata(results, "docno");
	
	int numTerms = argc - 2;

	ReScoringEnvironment *scorer = new ReScoringEnvironment(env, results);
	SCOREOUTPUT *tfIdfScores = scorer->multiTermTfIdf(query, numTerms);
	SCOREOUTPUT *bm25Scores = scorer->bm25(query, numTerms);
	
	for(int i = 0; i < results.size(); i++) {
		double score = results[i].score;
		const char *docName = documentNames[i].c_str();
		
		printf("%s %.5f %.5f %.5f %.5f %.5f %.5f\n",
			docName,
			bm25Scores[i].totalScore,
			tfIdfScores[i].totalScore,
			bm25Scores[i].maxScore,
			tfIdfScores[i].maxScore,
			bm25Scores[i].avgScore,
			tfIdfScores[i].avgScore);
	}
}