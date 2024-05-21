#ifndef DREWNO_MARS_SYMBOL_TABLE_HPP
#define DREWNO_MARS_SYMBOL_TABLE_HPP
#include <string>
#include <unordered_map>
#include <list>
#include "ast.hpp"

//Use an alias template so that we can use
// "HashMap" and it means "std::unordered_map"
template <typename K, typename V>
using HashMap = std::unordered_map<K, V>;

using namespace std;

namespace drewno_mars{

class ScopeTable;
//A semantic symbol, which represents a single
// variable, function, etc. Semantic symbols 
// exist for the lifetime of a scope in the 
// symbol table. 
class SemSymbol {
public:
    SemSymbol(std::string nameIn, std::string kindIn, std::string typeIn,
              ScopeTable * scpTabIn = nullptr) :
    name(nameIn), kind(kindIn), type(typeIn),  scpTab(scpTabIn) { }
    std::string getKind() {
        return kind;
    }
    std::string getName() {
        return name;
    }
    std::string getType() {
        return type;
    }
    ScopeTable * getScopeTable() {
        return scpTab;
    }


private:
    std::string name;
    std::string kind;
    std::string type;
    ScopeTable * scpTab;
};



//A single scope. The symbol table is broken down into a 
// chain of scope tables, and each scope table holds 
// semantic symbols for a single scope. For example,
// the globals scope will be represented by a ScopeTable,
// and the contents of each function can be represented by
// a ScopeTable.
class ScopeTable {
	public:
		ScopeTable();
        SemSymbol * lookup(std::string name);
        bool insert(SemSymbol * symbol);
        bool collision(std::string name);

	private:
		HashMap<std::string, SemSymbol *> * symbols;
};

class SymbolTable{
	public:
		SymbolTable();
        ScopeTable * enterScope(ScopeTable * scope = nullptr);
        void leaveScope();
        ScopeTable * getScope();
        bool insert(SemSymbol * symbol);
        SemSymbol * lookup(std::string name);
        bool collision(std::string name);
	private:
		std::list<ScopeTable *> * scopeTableChain;
};

	
}

#endif
