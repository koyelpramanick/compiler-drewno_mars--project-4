#include "ast.hpp"
#include "symbol_table.hpp"
#include "errName.hpp"

namespace drewno_mars{

bool ASTNode::nameAnalysis(SymbolTable * symTab){
	throw new ToDoError("This function should have"
		"been overriden in the subclass!");
}

bool ProgramNode::nameAnalysis(SymbolTable * symTab){
    symTab->enterScope();
	bool res = true;
	for (auto global : *myGlobals){
		res = global->nameAnalysis(symTab) && res;
	}
    symTab->leaveScope();
	return res;
}

bool IDNode::nameAnalysis(SymbolTable *symTab) {
    SemSymbol * symbol = symTab->lookup(name);
    if (symbol == nullptr){
        return NameErr::undeclID(pos());
    }
    this->attachSymbol(symbol);
    return true;
}

void IDNode::attachSymbol(SemSymbol * symbolIn){
    this->mySymbol = symbolIn;
}

bool ClassDefnNode::nameAnalysis(SymbolTable *symTab) {
    std::string className = this->ID()->getName();

    ScopeTable * oldScope = symTab->getScope();
    ScopeTable * newScope = symTab->enterScope();

    bool noCollision = true;
    if (oldScope->collision(className)){
        NameErr::multiDecl(ID()->pos());
        noCollision = false;
    }

    if (noCollision){
        auto * symbol = new SemSymbol(className, "class", className, newScope);
        oldScope->insert(symbol);
        this->ID()->attachSymbol(symbol);
    }

    bool goodMemberDecls = true;
    std::list<DeclNode *> * decls = this->getMembers();
    for (auto decl : *decls) {
        goodMemberDecls = decl->nameAnalysis(symTab) && goodMemberDecls;
    }

    symTab->leaveScope();
    return (noCollision && goodMemberDecls);
}

bool VarDeclNode::nameAnalysis(SymbolTable * symTab){
    std::string type = this->getTypeNode()->getType();
    std::string name = this->ID()->getName();


    bool goodType = type != "void";
    if (!goodType){
        NameErr::badVarType(ID()->pos());
    }

    bool noCollision = !symTab->collision(name);
    if (!noCollision){
        NameErr::multiDecl(ID()->pos());
    }

    bool goodAssignment = true;
    if(myInit != nullptr) {
        goodAssignment = myInit->nameAnalysis(symTab);
    }

    bool goodClass = this->myType->nameAnalysis(symTab);

    if (!goodType || !noCollision || !goodAssignment || !goodClass){
        return false;
    } else {
        SemSymbol * classSymbol = myType->getSymbol();
        SemSymbol * symbol;
        if (classSymbol != nullptr) {
            ScopeTable * classScope = classSymbol->getScopeTable();
            symbol = new SemSymbol(name, "var", type, classScope);

        } else {
            symbol = new SemSymbol(name, "var", type);
        }
        symTab->insert(symbol);
        this->ID()->attachSymbol(symbol);
        return true;
    }
}

bool FnDeclNode::nameAnalysis(SymbolTable * symTab){
    std::string funcName = this->ID()->getName();

    bool goodReturnType = this->myRetType->nameAnalysis(symTab);

    ScopeTable * oldFuncScope = symTab->getScope();
    ScopeTable * newFuncScope = symTab->enterScope();

    bool noCollision = true;
    if (oldFuncScope->collision(funcName)){
        NameErr::multiDecl(ID()->pos());
        noCollision = false;
    }

    bool goodFormals = true;

    std::string type = "(";
    std::list<FormalDeclNode *> * formals = this->getFormals();

    bool firstFormal = true;
    for (auto formal : *formals) {
        goodFormals = formal->nameAnalysis(symTab) && goodFormals;
        if (firstFormal) {
            firstFormal = false;
        } else {
            type += ",";
        }
        type += formal->getTypeNode()->getType();
    }
    type += ")->";
    type += this->getTypeNode()->getType();

    if (noCollision){
        auto * symbol = new SemSymbol(funcName, "fn", type);
        oldFuncScope->insert(symbol);
        this->ID()->attachSymbol(symbol);
    }

    bool goodBody = true;
    for (auto stmt : *myBody){
        goodBody = stmt->nameAnalysis(symTab) && goodBody;
    }

    symTab->leaveScope();
    return (goodReturnType && goodFormals && noCollision && goodBody);
}

bool AssignStmtNode::nameAnalysis(SymbolTable * symTab){
    bool result = myDst->nameAnalysis(symTab);
    result = mySrc->nameAnalysis(symTab) && result;
    return result;
}

bool TakeStmtNode::nameAnalysis(SymbolTable *symTab) {
    return myDst->nameAnalysis(symTab);
}

bool GiveStmtNode::nameAnalysis(SymbolTable *symTab) {
    return mySrc->nameAnalysis(symTab);
}

bool ExitStmtNode::nameAnalysis(SymbolTable *symTab) {
    return true;
}

bool PostDecStmtNode::nameAnalysis(SymbolTable *symTab) {
    return myLoc->nameAnalysis(symTab);
}

bool PostIncStmtNode::nameAnalysis(SymbolTable *symTab) {
    return myLoc->nameAnalysis(symTab);
}

bool IfStmtNode::nameAnalysis(SymbolTable *symTab) {
    bool result = myCond->nameAnalysis(symTab);
    symTab->enterScope();
    for (auto stmt : *myBody){
        result = stmt->nameAnalysis(symTab) && result;
    }
    symTab->leaveScope();
    return result;
}

bool IfElseStmtNode::nameAnalysis(SymbolTable *symTab) {
    bool result = myCond->nameAnalysis(symTab);
    symTab->enterScope();
    for (auto stmt : *myBodyTrue){
        result = stmt->nameAnalysis(symTab) && result;
    }
    symTab->leaveScope();
    symTab->enterScope();
    for (auto stmt : *myBodyFalse){
        result = stmt->nameAnalysis(symTab) && result;
    }
    symTab->leaveScope();
    return result;
}

bool WhileStmtNode::nameAnalysis(SymbolTable *symTab) {
    bool result = myCond->nameAnalysis(symTab);
    symTab->enterScope();
    for (auto stmt : *myBody){
        result = stmt->nameAnalysis(symTab) && result;
    }
    symTab->leaveScope();
    return result;
}

bool ReturnStmtNode::nameAnalysis(SymbolTable *symTab) {
    if (myExp == nullptr) {
        return true;
    }
    return myExp->nameAnalysis(symTab);
}

bool CallExpNode::nameAnalysis(SymbolTable* symTab){
    bool result = myCallee->nameAnalysis(symTab);
    for (auto arg : *myArgs){
        result = arg->nameAnalysis(symTab) && result;
    }
    return result;
}

bool MemberFieldExpNode::nameAnalysis(SymbolTable *symTab) {
    bool result = myBase->nameAnalysis(symTab);
    SemSymbol * symbol = getSymbol();
    if(symbol != nullptr) {
        ScopeTable * scope = symbol->getScopeTable();
        symTab->enterScope(scope);
        result = myField->nameAnalysis(symTab) && result;
        symTab->leaveScope();
        return result;
    }
    return false;
}

bool BinaryExpNode::nameAnalysis(SymbolTable *symTab) {
    bool result = myExp1->nameAnalysis(symTab);
    result = myExp2->nameAnalysis(symTab) && result;
    return result;
}

bool UnaryExpNode::nameAnalysis(SymbolTable *symTab) {
    return myExp->nameAnalysis(symTab);
}

bool VoidTypeNode::nameAnalysis(SymbolTable *symTab) {
    return true;
}

bool ClassTypeNode::nameAnalysis(SymbolTable *symTab) {
    return myID->nameAnalysis(symTab);
}

bool PerfectTypeNode::nameAnalysis(SymbolTable *symTab) {
    return true;
}

bool IntTypeNode::nameAnalysis(SymbolTable* symTab){
    // Name analysis may never even recurse down to IntTypeNode,
    // but if it does, just return true to indicate that
    // name analysis has not failed, and add nothing to the symbol table
    return true;
}

bool BoolTypeNode::nameAnalysis(SymbolTable *symTab) {
    return true;
}

bool IntLitNode::nameAnalysis(SymbolTable *symTab) {
    return true;
}

bool StrLitNode::nameAnalysis(SymbolTable *symTab) {
    return true;
}

bool TrueNode::nameAnalysis(SymbolTable *symTab) {
    return true;
}

bool FalseNode::nameAnalysis(SymbolTable *symTab) {
    return true;
}

bool MagicNode::nameAnalysis(SymbolTable *symTab) {
    return true;
}

bool CallStmtNode::nameAnalysis(SymbolTable* symTab){
    return myCallExp->nameAnalysis(symTab);
}
}
