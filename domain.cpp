#include "domain.h"

namespace transport_routine::domain {

bool StopPtrComparatorLess::operator()(const Stop* lhs, const Stop* rhs) const {
    return lhs->name < rhs->name;
}

bool BusPtrComparatorLess::operator()(const Bus* lhs, const Bus* rhs) const {
    return lhs->name < rhs->name;
}

} // namespace transport_routine::domain
