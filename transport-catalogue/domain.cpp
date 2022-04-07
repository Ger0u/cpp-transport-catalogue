#include "domain.h"

using namespace std;

namespace domain {

Stop::Stop(string name, geo::Coordinates coordinates)
: name_(move(name))
, coordinates_(move(coordinates)) {
}
        
void Stop::AddDistance(string_view name, int distance) {
    distances_.insert({name, distance});
}
        
string_view Stop::GetName() const {
    return name_;
}

const geo::Coordinates& Stop::GetCoordinates() const {
    return coordinates_;
}
    
const unordered_map<string_view, int>& Stop::GetDistances() const {
    return distances_;
}
    
const set<Bus*, Stop::BusComparator>& Stop::GetBuses() const {
    return buses_;
}
    
bool Stop::BusComparator::operator()(Bus* lhs, Bus* rhs) const {
    return lhs->GetName() < rhs->GetName();
}
    
Bus::Bus(string name, vector<Stop*> stops, bool ring)
: name_(move(name))
, stops_(move(stops))
, ring_(ring)
, length_(LengthCalculation())
, ideal_length_(IdealLengthCalculation())
, count_stops_(CountStopsCalculation())
, count_unique_stops_(CountUniqueStopsCalculation()) {
    AddBusToStops();
}

string_view Bus::GetName() const {
    return name_;
}

const vector<Stop*>& Bus::GetStops() const {
    return stops_;
}

bool Bus::GetRing() const {
    return ring_;
}

int Bus::GetLength() const {
    return length_;
}

double Bus::GetIdealLength() const {
    return ideal_length_;
}
 
size_t Bus::GetCountStops() const {
    return count_stops_;
}

size_t Bus::GetCountUniqueStops() const {
    return count_unique_stops_;
}

void Bus::AddBusToStops() {
    for (auto stop : stops_) {
        stop->buses_.insert(this);
    }
}

int Bus::LengthCalculation() const {
    if (stops_.empty()) {
        return 0;
    }
    int result = 0;
    if (ring_) {
        auto previous_stop = stops_.cbegin();
        for (auto stop = stops_.cbegin() + 1; stop < stops_.cend(); ++stop) {
            {
                auto distance = (*previous_stop)->GetDistances().find((*stop)->GetName());
                result += distance != (*previous_stop)->GetDistances().end()
                        ? distance->second
                        : (*stop)->GetDistances().at((*previous_stop)->GetName());
            }
            previous_stop = stop;
        }
    } else {
        auto previous_stop = stops_.cbegin();
        for (auto stop = stops_.cbegin() + 1; stop < stops_.cend(); ++stop) {
            {
                auto distance1 = (*previous_stop)->GetDistances().find((*stop)->GetName());
                auto distance2 = (*stop)->GetDistances().find((*previous_stop)->GetName());
                result += distance1 != (*previous_stop)->GetDistances().end()
                        ? distance2 != (*stop)->GetDistances().end()
                        ? distance1->second + distance2->second
                        : 2 * distance1->second
                        : 2 * distance2->second;
            }
            previous_stop = stop;
        }
    }
    return result;
}

double Bus::IdealLengthCalculation() const {
    if (stops_.empty()) {
        return 0;
    }
    double result = 0;
    {
        auto previous_stop = stops_.cbegin();
        for (auto stop = stops_.cbegin() + 1; stop < stops_.cend(); ++stop) {
            result += geo::ComputeDistance(
                (*previous_stop)->GetCoordinates(),
                (*stop)->GetCoordinates());
            previous_stop = stop;
        }
    }
    return ring_ ? result : 2 * result;
}

size_t Bus::CountStopsCalculation() const {
    return stops_.empty() ? 0 : ring_ ? stops_.size() : 2 * stops_.size() - 1;
}

size_t Bus::CountUniqueStopsCalculation() const {
    const set<Stop*> unique_stops(stops_.begin(), stops_.end());
    return unique_stops.size();
}
    
} // namespace domain