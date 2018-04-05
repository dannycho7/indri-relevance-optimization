#include <vector>
#include <string>
#include <indri/QueryEnvironment.hpp>

struct SCOREOUTPUT {
	double minScore;
	double maxScore;
	double avgScore;
	double totalScore;
	SCOREOUTPUT() {
		minScore = INT_MAX;
		maxScore = 0;
		avgScore = 0;
		totalScore = 0;
	}
};

class ReScoringEnvironment {
public:
	ReScoringEnvironment(indri::api::QueryEnvironment &env, std::vector<indri::api::ScoredExtentResult> &results);	
	SCOREOUTPUT* multiTermTfIdf(std::string multiTerms, int numTerms);
	double* tfIdf(std::string term);
	SCOREOUTPUT* bm25(std::string qterms, int numTerms, double k1 = 1.2, double b = 0.75);

private:
	double getIdf(std::string term);
	double getTermFrequency(std::string searchTerm, std::string content);
	double getAvgDocLength(int numDocs);
	
	indri::api::QueryEnvironment queryenv;
	std::vector <indri::api::ParsedDocument *> documents;
	std::vector<indri::api::ScoredExtentResult> initialResults;
	double avgDocLength;
};
