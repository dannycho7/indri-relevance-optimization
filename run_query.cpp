#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <indri/QueryEnvironment.hpp>
#include <indri/TFIDFTermScoreFunction.hpp>

class ReScoringEnvironment {
public:
	ReScoringEnvironment(indri::api::QueryEnvironment &env,
				std::vector <indri::api::ParsedDocument *> &documents,
				std::vector<indri::api::ScoredExtentResult> &results) {
		this->queryenv = env;
		this->documents = documents;
		this->initialResults = results;
	}	

	double* multiTermTfIdf(std::string multiTerms) {
		double *scores = new double[documents.size()];

		std::stringstream terms(multiTerms);
		std::string term;
		
		while (terms >> term) {
			double *termScore = this->tfIdf(term);
			for (int i = 0; i < documents.size(); i++) {
				scores[i] += termScore[i];
			}

			delete[] termScore;
		}
		return scores;
	}

	double* tfIdf(std::string term) {
		double *scores = new double[documents.size()];
		double idf = this->getIdf(term);

		for (int i = 0; i < documents.size(); i++) {
			double tf = this->getTermFrequency(term, this->documents[i]->content);
			scores[i] = tf * idf;
		}

		return scores;
	}

	double* bm25(std::string qterms, double k1 = 1.2, double b = 0.75) {
		double *scores = new double[documents.size()];
		std::stringstream terms(qterms);
		std::string term;		
		double avgLen = this->getAvgDocLength();

		while (terms >> term) {
			double idf = getIdf(term);

			for (int i = 0; i < documents.size(); i++) {
				double tf = getTermFrequency(term, documents[i]->content);
				double docLength = this->queryenv.documentLength(this->initialResults[i].document);
				scores[i] += idf * tf * (k1 + 1) / (tf + k1 * (1 - b + (b * docLength / avgLen)));
			}	
		}

		return scores;
	}

private:
	double getIdf(std::string term) {
		double df = (double) this->queryenv.documentCount() / this->queryenv.documentCount(term);
		return -log(1/df);
	}

	double getTermFrequency(std::string searchTerm, std::string content) {
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
	
	// Though this gets the average doc length of the resulting documents, it should suffice since we're looking at relative rankings
	double getAvgDocLength() {
		int totalDocLength = 0;
		for (int i = 0; i < this->documents.size(); i++) {
			totalDocLength += queryenv.documentLength(this->initialResults[i].document);
		}

		return (double) totalDocLength / this->documents.size();
	}

	indri::api::QueryEnvironment queryenv;
	std::vector <indri::api::ParsedDocument *> documents;
	std::vector<indri::api::ScoredExtentResult> initialResults; 
};


int main() {
	std::string query = "international organized crime";

	indri::api::QueryEnvironment env;
	env.addIndex("./indri_cli/index");

	std::vector<indri::api::ScoredExtentResult> results = env.runQuery(query, 10);
	std::vector<std::string> documentNames = env.documentMetadata(results, "docno");
	
	std::vector<indri::api::ParsedDocument *> documents = env.documents(results);
	ReScoringEnvironment *scorer = new ReScoringEnvironment(env, documents, results);
	double *tfIdfScores = scorer->multiTermTfIdf(query);
	double *bm25Scores = scorer->bm25(query);
	
	for(int i = 0; i < results.size(); i++) {
		double score = results[i].score;
		std::string docName = documentNames[i];
		std::cout << docName << " " << score << " " << tfIdfScores[i] << " " << bm25Scores[i] << std::endl;
	}
}
