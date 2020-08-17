#include <queue>
#include <set>
#include "utils/Search.hpp"

namespace Utils::Search {
    // Combination of values to store in priority queue
    struct QueueEntry {
        ScoredString phrase;            // Phrase formed from words
        std::vector<size_t> indices;    // Vector of indices that words were grabbed from
    };

    // Custom operator for priority queue which places lower scores at the top
    struct MIN_QUEUEENTRY {
        bool operator()(const QueueEntry & lhs, const QueueEntry & rhs) {
            return lhs.phrase.score > rhs.phrase.score;
        }
    };

    // Helper function to form QueueEntry
    // Expect words.size() == indices.size()
    static QueueEntry getQueueEntry(const std::vector< std::vector<ScoredString> > & words, const std::vector<size_t> & indices) {
        // Initialize as empty
        QueueEntry tmp;
        tmp.phrase.string = "";
        tmp.phrase.score = 0;

        // Iterate over vector and populate
        for (size_t k = 0; k < words.size(); k++) {
            size_t index = indices[k];

            // Concatenate phrase
            tmp.phrase.string += words[k][index].string;
            if (k != words.size() - 1) {
                tmp.phrase.string += " ";
            }

            // Sum up scores
            tmp.phrase.score += words[k][index].score;
            tmp.indices.push_back(index);
        }

        return tmp;
    }

    std::vector<std::string> getPhrases(std::vector< std::vector<ScoredString> > & words, size_t num) {
        // Vector of phrases to return
        std::vector<std::string> phrases;

        // Remove any empty 'columns'
        size_t i = 0;
        while (i < words.size()) {
            if (words[i].empty()) {
                words.erase(words.begin() + i);
            } else {
                i++;
            }
        }

        // Simply return if only one 'column' or none
        if (words.empty()) {
            return phrases;

        } else if (words.size() == 1) {
            // Push back required number of words
            for (size_t i = 0; i < words[0].size(); i++) {
                if (phrases.size() >= num) {
                    break;
                }
                phrases.push_back(words[0][i].string);
            }
            return phrases;
        }

        // Priority queue used to grab highest scoring phrases
        std::priority_queue<QueueEntry, std::vector<QueueEntry>, MIN_QUEUEENTRY> pq;
        std::set< std::vector<size_t> > indices;

        // Create an entry using the best phrase
        std::vector<size_t> zeroes;
        zeroes.resize(words.size(), 0);
        QueueEntry entry = getQueueEntry(words, zeroes);

        // Push above entry on queue
        pq.push(entry);
        indices.insert(entry.indices);

        // Pick up to num best scoring phrases
        for (i = 0; i < num; i++) {
            // Stop if queue empty
            if (pq.empty()) {
                break;
            }

            // Get top element
            QueueEntry top = pq.top();
            pq.pop();
            phrases.push_back(top.phrase.string);

            // Form phrases using next index in each column
            for (size_t j = 0; j < top.indices.size(); j++) {
                QueueEntry tmp = top;

                // Increment the jth index and keep within bounds
                if (tmp.indices[j] < words[j].size()-1) {
                    tmp.indices[j]++;
                }

                // Get new entry and insert if not already inserted
                QueueEntry pot = getQueueEntry(words, tmp.indices);
                if (indices.find(pot.indices) == indices.end()) {
                    pq.push(pot);
                    indices.insert(pot.indices);
                }
            }
        }

        return phrases;
    }
};