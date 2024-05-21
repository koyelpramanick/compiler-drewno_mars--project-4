#include "symbol_table.hpp"
namespace drewno_mars{

ScopeTable::ScopeTable(){
	symbols = new HashMap<std::string, SemSymbol *>();
}

bool ScopeTable::collision(std::string name) {
    SemSymbol * collisionFound = lookup(name);
    if (collisionFound != nullptr){
        return true;
    }
    return false;
}

SemSymbol * ScopeTable::lookup(std::string name){
    auto symbolFound = symbols->find(name);
    if (symbolFound == symbols->end()){
        return nullptr;
    }
    return symbolFound->second;
}

bool ScopeTable::insert(SemSymbol * symbol){
    std::string symbolName = symbol->getName();
    bool inCurrentScope = (this->lookup(symbolName) != nullptr);
    if (inCurrentScope){
        return false;
    }
    this->symbols->insert(std::make_pair(symbolName, symbol));
    return true;
}

SymbolTable::SymbolTable(){
	scopeTableChain = new std::list<ScopeTable *>();
}

ScopeTable * SymbolTable::enterScope(ScopeTable *scope) {
    ScopeTable * newScopeTable;
    if(scope == nullptr){
        newScopeTable = new ScopeTable();
        scopeTableChain->push_front(newScopeTable);
    } else {
        newScopeTable = scope;
        scopeTableChain->push_front(newScopeTable);
    }

    return newScopeTable;
}

void SymbolTable::leaveScope() {
    if (!scopeTableChain->empty()) {
        scopeTableChain->pop_front();
    }
}

ScopeTable * SymbolTable::getScope() {
    return scopeTableChain->front();
}

bool SymbolTable::collision(std::string name) {
    return getScope()->collision(name);
}

SemSymbol * SymbolTable::lookup(std::string name) {
    for (ScopeTable * scopeTable : *scopeTableChain) {
        SemSymbol * symbol = scopeTable->lookup(name);
        if (symbol != nullptr) {
            return symbol;
        }
    }
    return nullptr;
}

bool SymbolTable::insert(SemSymbol * symbol) {
    return scopeTableChain->front()->insert(symbol);
}
}
