#pragma once

#include "geo.h"
#include <string>
#include <string_view>
#include <unordered_map>
#include <set>
#include <vector>

namespace domain {
    
class Bus;
    
class Stop final {
    friend class Bus;
private:
    struct BusComparator {
        bool operator()(Bus* lhs, Bus* rhs) const;
    };
    
public:
    Stop(std::string name, geo::Coordinates coordinates);

    void AddDistance(std::string_view name, int distance);

    std::string_view GetName() const;
    const geo::Coordinates& GetCoordinates() const;
    const std::unordered_map<std::string_view, int>& GetDistances() const;
    const std::set<Bus*, BusComparator>& GetBuses() const;

private:
    std::string name_;
    geo::Coordinates coordinates_;
    std::unordered_map<std::string_view, int> distances_;

    std::set<Bus*, BusComparator> buses_;
};

class Bus final {
public:
    Bus(std::string name, std::vector<Stop*> stops, bool ring);

    std::string_view GetName() const;
    const std::vector<Stop*>& GetStops() const;
    bool GetRing() const;
    int GetLength() const;
    double GetIdealLength() const;
    size_t GetCountStops() const;
    size_t GetCountUniqueStops() const;

private:
    std::string name_;
    std::vector<Stop*> stops_;
    bool ring_;

    int length_;
    double ideal_length_;
    size_t count_stops_;
    size_t count_unique_stops_;

    void AddBusToStops();

    int LengthCalculation() const;
    double IdealLengthCalculation() const;
    size_t CountStopsCalculation() const;
    size_t CountUniqueStopsCalculation() const;
};
    
} // namespace domain