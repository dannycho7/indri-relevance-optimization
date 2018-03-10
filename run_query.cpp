#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <indri/QueryEnvironment.hpp>
#include <indri/TFIDFTermScoreFunction.hpp>

struct SCOREOUTPUT {
	double maxScore;
	double avgScore;
	double totalScore;
	SCOREOUTPUT() {
		maxScore = 0;
		avgScore = 0;
		totalScore = 0;
	}
};

class ReScoringEnvironment {
public:
	ReScoringEnvironment(indri::api::QueryEnvironment &env,
				std::vector <indri::api::ParsedDocument *> &documents,
				std::vector<indri::api::ScoredExtentResult> &results) {
		this->queryenv = env;
		this->documents = documents;
		this->initialResults = results;
	}	

	SCOREOUTPUT* multiTermTfIdf(std::string multiTerms, int numTerms) {
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

	double* tfIdf(std::string term) {
		double *scores = new double[documents.size()];
		double idf = this->getIdf(term);

		for (int i = 0; i < documents.size(); i++) {
			double tf = this->getTermFrequency(term, this->documents[i]->content);
			scores[i] = tf * idf;
		}

		return scores;
	}

	SCOREOUTPUT* bm25(std::string qterms, int numTerms, double k1 = 1.2, double b = 0.75) {
		SCOREOUTPUT *scores = new SCOREOUTPUT[documents.size()];
		std::stringstream terms(qterms);
		std::string term;		
		double avgLen = this->getAvgDocLength();

		while (terms >> term) {
			double idf = getIdf(term);

			for (int i = 0; i < documents.size(); i++) {
				double tf = getTermFrequency(term, documents[i]->content);
				double docLength = this->queryenv.documentLength(this->initialResults[i].document);
				double bm25Score = idf * tf * (k1 + 1) / (tf + k1 * (1 - b + (b * docLength / avgLen)));
				
				scores[i].totalScore += bm25Score;
				scores[i].maxScore = (scores[i].maxScore > bm25Score) ? scores[i].maxScore : bm25Score;
			}
		}

		for (int i = 0; i < documents.size(); i++) {
			scores[i].avgScore = scores[i].totalScore / numTerms;
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
	
	std::vector<indri::api::ParsedDocument *> documents = env.documents(results);
	
	int numTerms = argc - 2;
	ReScoringEnvironment *scorer = new ReScoringEnvironment(env, documents, results);
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
