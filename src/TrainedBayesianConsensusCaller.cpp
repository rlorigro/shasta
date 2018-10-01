#include "TrainedBayesianConsensusCaller.hpp"
#include "Coverage.hpp"
using namespace ChanZuckerberg;
using namespace shasta;

// The trained parameters should be provided in a file at this filepath in the
// run directory. The file is assumed to follow a columnar structure with columns
// separated by tabs:
//
// called_base called_len true_base true_len prob
// -           0          -         0        .00001
// -           0          A         1        .0000001
// etc.
//
// In addition, there is assumed to be a header line with column titles.
static const string trainedDistributionFilepath = "consensus_distribution";

TrainedBayesianConsensusCaller::TrainedBayesianConsensusCaller()
{
    std::ifstream trainedDistributionFile(trainedDistributionFilepath);
    
    std::string line;
    
    // Load the header line and ignore it
    std::getline(trainedDistributionFile, line);
    
    std::map<tuple<AlignedBase, size_t, AlignedBase, size_t>, double> distribution;
    
    while (std::getline(trainedDistributionFile, line)) {
        // Parse each line of the .tsv file generated by the training script
        
        // Strip the newline character
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        
        // Tokenize the line by tabs
        std::stringstream strm(line);
        vector<std::string> tokens;
        std::string buffer;
        while (std::getline(strm, buffer, '\t')) {
            buffer.erase(std::remove(buffer.begin(), buffer.end(), '\t'), buffer.end());
            tokens.push_back(buffer);
        }
        
        // Parse tokens and verify that data matches expected format
        assert(tokens.size() == 5);
        
        size_t parsed_size = 0;
        
        AlignedBase calledBase = AlignedBase::fromCharacter(tokens[0].front());
        assert(tokens[0].size() == 1);
        
        size_t calledRepeatCount = std::stoull(tokens[1], &parsed_size);
        assert(parsed_size == tokens[1].size());
        
        AlignedBase trueBase = AlignedBase::fromCharacter(tokens[2].front());
        assert(tokens[2].size() == 1);
        
        size_t trueRepeatCount = std::stoull(tokens[3], &parsed_size);
        assert(parsed_size == tokens[3].size());
        
        double probability = std::stod(tokens[4], &parsed_size);
        assert(parsed_size == tokens[4].size());
        
        maxRepeatCount = std::max(std::max(calledRepeatCount, trueRepeatCount), maxRepeatCount);
        
        distribution[make_tuple(calledBase, calledRepeatCount,
                                trueBase, trueRepeatCount)] = probability;
    }
    
    // Make vector of all possible homopolymer repeats
    repeatBases.reserve(4 * maxRepeatCount + 1);
    repeatBases.emplace_back(AlignedBase::fromCharacter('-'), 0);
    for (char base : "ACGT") {
        for (size_t len = 1; len <= maxRepeatCount; len++) {
            repeatBases.emplace_back(AlignedBase::fromCharacter(base), len);
        }
    }
    
    // Compute the conditional probability of each true run length base given each called run length base
    for (const Consensus& calledRepeatBase : repeatBases) {
        
        double normalizingFactor = 0.0;
        for (const auto& trueRepeatBase : repeatBases) {
            normalizingFactor += distribution[make_tuple(calledRepeatBase.base,
                                                         calledRepeatBase.repeatCount,
                                                         trueRepeatBase.base,
                                                         trueRepeatBase.repeatCount)];
        }
        double logNormalizingFactor = log(normalizingFactor);
        
        for (const Consensus& trueRepeatBase : repeatBases) {
            auto key = make_tuple(calledRepeatBase.base, calledRepeatBase.repeatCount,
                                  trueRepeatBase.base, trueRepeatBase.repeatCount);
            logConditionalProbabilities[key] = log(distribution[key]) - logNormalizingFactor;
        }
    }
}



Consensus TrainedBayesianConsensusCaller::operator()(
    const Coverage& coverage) const
{

    // Initialize the return value
    double maxLogLikelihood = std::numeric_limits<double>::lowest();
    Consensus consensus;
    
    // Check likelihood for each possible consensus call
    for (const Consensus& trueRepeatBase : repeatBases) {
        double logLikelihood = 0.0;
        for (const CoverageData& observation : coverage.getReadCoverageData()) {
            
            // Match either the bases or their complements depending on which one was the actual
            // called read sequence (i.e. the base that traversed the nanopore)
            AlignedBase trueLookupBase;
            AlignedBase calledLookupBase;
            if (observation.strand) {
                trueLookupBase = trueRepeatBase.base.complement();
                calledLookupBase = observation.base.complement();
            }
            else {
                trueLookupBase = trueRepeatBase.base;
                calledLookupBase = observation.base;
            }
            
            logLikelihood += logConditionalProbabilities.at(make_tuple(calledLookupBase,
                                                                       observation.repeatCount,
                                                                       trueLookupBase,
                                                                       trueRepeatBase.repeatCount));
        }
        
        // Identify the maximum likelihood consensus
        if (logLikelihood > maxLogLikelihood) {
            maxLogLikelihood = logLikelihood;
            consensus = trueRepeatBase;
        }
    }
    
    return consensus;
}
