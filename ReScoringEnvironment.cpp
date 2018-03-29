#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <indri/QueryEnvironment.hpp>
#include "ReScoringEnvironment.h"

ReScoringEnvironment::ReScoringEnvironment(indri::api::QueryEnvironment &env, std::vector<indri::api::ScoredExtentResult> &results) {
	this->queryenv = env;
	this->initialResults = results;	
	this->documents = env.documents(results);
	this->avgDocLength = this->getAvgDocLength(env.documentCount());
}	

SCOREOUTPUT* ReScoringEnvironment::multiTermTfIdf(std::string multiTerms, int numTerms) {
	SCOREOUTPUT *scores = new SCOREOUTPUT[documents.size()];

	std::stringstream terms(multiTerms);
	std::string term;
	
	while (terms >> term) {
		double *termScore = this->tfIdf(term);
		for (int i = 0; i < documents.size(); i++) {
			scores[i].totalScore += termScore[i];
			scores[i].maxScore = (scores[i].maxScore > termScore[i]) ? scores[i].maxScore : termScore[i];
		}

		delete[] termScore;
	}

	for (int i = 0; i < this->documents.size(); i++) {
		scores[i].avgScore = scores[i].totalScore / numTerms;
	}

	return scores;
}

double* ReScoringEnvironment::tfIdf(std::string term) {
	double *scores = new double[documents.size()];
	double idf = this->getIdf(term);

	for (int i = 0; i < documents.size(); i++) {
		double tf = this->getTermFrequency(term, this->documents[i]->content);
		scores[i] = tf * idf;
	}

	return scores;
}

SCOREOUTPUT* ReScoringEnvironment::bm25(std::string qterms, int numTerms, double k1, double b) {
	SCOREOUTPUT *scores = new SCOREOUTPUT[documents.size()];
	std::stringstream terms(qterms);
	std::string term;		

	while (terms >> term) {
		double idf = getIdf(term);

		for (int i = 0; i < documents.size(); i++) {
			double tf = getTermFrequency(term, documents[i]->content);
			double docLength = this->queryenv.documentLength(this->initialResults[i].document);
			double bm25Score = idf * tf * (k1 + 1) / (tf + k1 * (1 - b + (b * docLength / this->avgDocLength)));
			
			scores[i].totalScore += bm25Score;
			scores[i].maxScore = (scores[i].maxScore > bm25Score) ? scores[i].maxScore : bm25Score;
		}
	}

	for (int i = 0; i < documents.size(); i++) {
		scores[i].avgScore = scores[i].totalScore / numTerms;
	}

	return scores;
}

double ReScoringEnvironment::getIdf(std::string term) {
	double df = (double) this->queryenv.documentCount(term);
	double numDocs = this->queryenv.documentCount();
	return log(numDocs / df);
}

double ReScoringEnvironment::getTermFrequency(std::string searchTerm, std::string content) {
	int occurrences = 0;
	std::string searchTermStem = this->queryenv.stemTerm(searchTerm);

	std::stringstream contentstream(content);
	std::string contentword;

	int terms = 0;

	while (contentstream >> contentword) {
		if (searchTermStem == this->queryenv.stemTerm(contentword)) {
			occurrences++;
		}
		terms++;
	}

	return (double) occurrences / terms;
}

double ReScoringEnvironment::getAvgDocLength(int numDocs) {
	int totalDocLength = 0;
	for (int i = 1; i <= numDocs; i++) {
		totalDocLength += queryenv.documentLength(i);
	}

	return (double) totalDocLength / numDocs;
}
