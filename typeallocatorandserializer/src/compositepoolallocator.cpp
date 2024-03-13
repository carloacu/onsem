#include <onsem/typeallocatorandserializer/advanced/compositepoolallocator.hpp>
#include <onsem/typeallocatorandserializer/advanced/treememoryprettyprinter.hpp>
#include <string>
#include <onsem/typeallocatorandserializer/advanced/treememoryprettyprinter.hpp>

namespace onsem {

std::ostream& operator<<(std::ostream& pOs, const CompositePoolAllocator& pFPA) {
    TreeMemoryPrettyPrinter printer(pOs);
    pFPA.accept(printer);
    return pOs;
}

CompositePoolAllocator::CompositePoolAllocator(const std::string& pName)
    : ComponentPoolAllocator(pName)
    , fPoolAllocators() {}

void CompositePoolAllocator::clear() {
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i) {
        fPoolAllocators[i]->clear();
    }
}

std::size_t CompositePoolAllocator::getOccupatedSize() const {
    std::size_t res = 0;
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i) {
        res += fPoolAllocators[i]->getOccupatedSize();
    }
    return res;
}

std::size_t CompositePoolAllocator::getTotalSize() const {
    std::size_t res = 0;
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i) {
        res += fPoolAllocators[i]->getTotalSize();
    }
    return res;
}

void CompositePoolAllocator::addANewComposite(const std::string& pName) {
    fPoolAllocators.emplace_back(new CompositePoolAllocator(pName));
}

void CompositePoolAllocator::accept(TreeMemoryPrettyPrinter& pV) const {
    pV(*this);
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i) {
        fPoolAllocators[i]->accept(pV);
    }
}

void CompositePoolAllocator::xWriteMemoryInAStream(std::ostream& pOstr) const {
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i) {
        fPoolAllocators[i]->xWriteMemoryInAStream(pOstr);
    }
}

void CompositePoolAllocator::xReadMemoryFromAStream(std::set<FPAShift>& pShifts, std::istream& pIstr, float pCoef) {
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i) {
        fPoolAllocators[i]->xReadMemoryFromAStream(pShifts, pIstr, pCoef);
    }
}

void CompositePoolAllocator::xFindShiftsForDefragmentation(std::set<FPAShift>& pShifts) const {
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i) {
        fPoolAllocators[i]->xFindShiftsForDefragmentation(pShifts);
    }
}

void CompositePoolAllocator::xChangePointers(const std::set<FPAShift>& pShifts) {
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i) {
        fPoolAllocators[i]->xChangePointers(pShifts);
    }
}

CompositePoolAllocator* CompositePoolAllocator::getComposite(const std::string& pName) const {
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i) {
        CompositePoolAllocator* leafPool = dynamic_cast<CompositePoolAllocator*>(fPoolAllocators[i]);
        if (leafPool && leafPool->getName() == pName) {
            return leafPool;
        }
    }
    return nullptr;
}

void CompositePoolAllocator::getComposites(std::vector<CompositePoolAllocator*>& pComposites) {
    for (std::size_t i = 0; i < fPoolAllocators.size(); ++i) {
        CompositePoolAllocator* leafPool = dynamic_cast<CompositePoolAllocator*>(fPoolAllocators[i]);
        if (leafPool) {
            pComposites.emplace_back(leafPool);
        }
    }
}

}    // End of namespace onsem
