#include <iostream>
#include <vector>
#include <indri/QueryEnvironment.hpp>

int main() {
	indri::api::QueryEnvironment env;
	env.addIndex("./indri_cli/index");
	
	std::vector<indri::api::ScoredExtentResult> results = env.runQuery("international organized crime", 10);
	std::vector<std::string> documentNames = env.documentMetadata(results, "docno");
	
	for(int i = 0; i < 10; i++) {
		double score = results[i].score;
		std::string docName = documentNames[i];
		std::cout << docName << " " <<results[i].score <<std::endl;
	}
}
