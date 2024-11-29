#include "mem/cache/replacement_policies/lip_2.hh"
#include <memory>
#include "params/LIP2RP.hh"
#include "sim/cur_tick.hh"

namespace gem5
{
namespace replacement_policy
{
LIP2::LIP2(const Params &p)
    : LRU(p)
{
}

void LIP2::invalidate(const std::shared_ptr<ReplacementData>& replacement_data){  // Reset last touch timestamp
    std::static_pointer_cast<LRUReplData>(
        replacement_data)->lastTouchTick = Tick(0);
}

void LIP2::touch(const std::shared_ptr<ReplacementData>& replacement_data) const{  // Update last touch timestamp
    std::static_pointer_cast<LRUReplData>(
        replacement_data)->lastTouchTick = curTick();
}

void LIP2::reset(const std::shared_ptr<ReplacementData>& replacement_data) const{  // Set last touch timestamp
    std::static_pointer_cast<LRUReplData>(
        replacement_data)->lastTouchTick = pq.top() + 1;
}

ReplaceableEntry* LIP2::getVictim(const ReplacementCandidates& candidates) const{  // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    while (!pq.empty()) {  // clear the heap
        pq.pop();
    }

    ReplaceableEntry* victim = candidates[0];

    std::unordered_map<Tick, ReplaceableEntry*> tickToCandidate;

    for (const auto& candidate : candidates) {
        auto lru_data = std::static_pointer_cast<LRUReplData>(candidate->replacementData);

        Tick lastTouchTick = lru_data->lastTouchTick;

        // Map the Tick to the corresponding candidate.
        tickToCandidate[lastTouchTick] = candidate;

        if (std::static_pointer_cast<LRUReplData> (candidate->replacementData)->lastTouchTick <
                std::static_pointer_cast<LRUReplData> (victim->replacementData)->lastTouchTick) {  // Identify the LRU block

            victim = candidate;
        }

        // Track the two most recent access times
        if(pq.size()<2){
            pq.push(lru_data->lastTouchTick);
        }
        else{
            if(pq.top()>lru_data->lastTouchTick){
                pq.pop();
                pq.push(lru_data->lastTouchTick);
            }
        }
    }

    // At this point, pq contains the two smallest ticks.
    // The top of pq is the second smallest (SLRU).
    Tick secondLRUTick = pq.top();

    // Use the map to get the candidate corresponding to the second LRU tick.
    return tickToCandidate[secondLRUTick];
}

} // namespace replacement_policy
} // namespace gem5
