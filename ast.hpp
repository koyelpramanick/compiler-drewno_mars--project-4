#ifndef DREWNO_MARS_AST_HPP
#define DREWNO_MARS_AST_HPP

#include <ostream>
#include <sstream>
#include <string.h>
#include <list>
#include "tokens.hpp"

namespace drewno_mars {

class NameAnalysis;

class SymbolTable;
class SemSymbol;

class DeclNode;
class VarDeclNode;
class StmtNode;
class FormalDeclNode;
class TypeNode;
class ExpNode;
class IDNode;

class ASTNode{
public:
	ASTNode(const Position * pos) : myPos(pos){ }
	virtual void unparse(std::ostream&, int) = 0;
	const Position * pos() { return myPos; };
	std::string posStr(){ return pos()->span(); }
	virtual bool nameAnalysis(SymbolTable *);
protected:
	const Position * myPos = nullptr;
};

class ProgramNode : public ASTNode{
public:
	ProgramNode(std::list<DeclNode *> * globalsIn);
	void unparse(std::ostream&, int) override;
	virtual bool nameAnalysis(SymbolTable *) override;
private:
	std::list<DeclNode *> * myGlobals;
};

class ExpNode : public ASTNode{
protected:
	ExpNode(const Position * p) : ASTNode(p){ }
public:
	virtual void unparseNested(std::ostream& out);
    virtual bool nameAnalysis(SymbolTable * symTab) override = 0;
};

class LocNode : public ExpNode{
public:
	LocNode(const Position * p)
	: ExpNode(p){}
    bool nameAnalysis(SymbolTable * symTab) override { return false; }
    virtual SemSymbol * getSymbol() = 0;
};

class IDNode : public LocNode{
public:
	IDNode(const Position * p, std::string nameIn)
	: LocNode(p), name(nameIn), mySymbol(nullptr){}
	std::string getName(){ return name; }
	void unparse(std::ostream& out, int indent) override;
	void unparseNested(std::ostream& out) override;
	void attachSymbol(SemSymbol * symbolIn);
	SemSymbol * getSymbol() override { return mySymbol; }
    bool nameAnalysis(SymbolTable * symTab) override;
private:
	std::string name;
	SemSymbol * mySymbol;
};

class TypeNode : public ASTNode{
public:
	TypeNode(const Position * p) : ASTNode(p){ }
	void unparse(std::ostream&, int) override = 0;
    bool nameAnalysis(SymbolTable *) override = 0;
    virtual std::string getType() = 0;
    virtual SemSymbol * getSymbol() {
        return nullptr;
    }
};

class StmtNode : public ASTNode{
public:
	StmtNode(const Position * p) : ASTNode(p){ }
	virtual void unparse(std::ostream& out, int indent) override = 0;
};

class DeclNode : public StmtNode{
public:
	DeclNode(const Position * p) : StmtNode(p){ }
	void unparse(std::ostream& out, int indent) override =0;
    virtual TypeNode* getTypeNode() = 0;
    virtual std::list<FormalDeclNode *> * getFormals() = 0;
    virtual IDNode * ID() = 0;
};

class ClassDefnNode : public DeclNode{
public:
	ClassDefnNode(const Position * p, IDNode * inID, std::list<DeclNode *> * inMembers)
	: DeclNode(p), myID(inID), myMembers(inMembers){ }
	void unparse(std::ostream& out, int indent) override;
	IDNode * ID() override { return myID; }
    bool nameAnalysis(SymbolTable * symTab) override;
    TypeNode * getTypeNode() override {return nullptr;}
    std::list<FormalDeclNode *> * getFormals() override {
        return formals;
    }
    std::list<DeclNode *> * getMembers() {
        return myMembers;
    }
private:
	IDNode * myID;
	std::list<DeclNode *> * myMembers;
    std::list<FormalDeclNode *> * formals;
};

class VarDeclNode : public DeclNode{
public:
	VarDeclNode(const Position * p, IDNode * inID,
	TypeNode * inType, ExpNode * inInit)
	: DeclNode(p), myID(inID), myType(inType), myInit(inInit){ }
	void unparse(std::ostream& out, int indent) override;
	IDNode * ID() override { return myID; }
	TypeNode * getTypeNode() override { return myType; }
    std::list<FormalDeclNode *> * getFormals() override {
        return formals;
    }
	bool nameAnalysis(SymbolTable * symTab) override;
private:
	IDNode * myID;
	TypeNode * myType;
	ExpNode * myInit;
    std::list<FormalDeclNode *> * formals;
};

class FormalDeclNode : public VarDeclNode{
public:
	FormalDeclNode(const Position * p, IDNode * id, TypeNode * type)
	: VarDeclNode(p, id, type, nullptr){ }
	void unparse(std::ostream& out, int indent) override;
};

class FnDeclNode : public DeclNode{
public:
	FnDeclNode(const Position * p,
	  IDNode * inID,
	  std::list<FormalDeclNode *> * inFormals,
	  TypeNode * retTypeIn,
	  std::list<StmtNode *> * inBody)
	: DeclNode(p), myID(inID),
	  myFormals(inFormals), myRetType(retTypeIn),
	  myBody(inBody){
	}
	IDNode * ID() override { return myID; }
    TypeNode * getTypeNode() override { return myRetType; }
	std::list<FormalDeclNode *> * getFormals() override{
		return myFormals;
	}
	void unparse(std::ostream& out, int indent) override;
	virtual bool nameAnalysis(SymbolTable * symTab) override;
private:
	IDNode * myID;
	std::list<FormalDeclNode *> * myFormals;
	TypeNode * myRetType;
	std::list<StmtNode *> * myBody;
};

class AssignStmtNode : public StmtNode{
public:
	AssignStmtNode(const Position * p, LocNode * inDst, ExpNode * inSrc)
	: StmtNode(p), myDst(inDst), mySrc(inSrc){ }
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
private:
	LocNode * myDst;
	ExpNode * mySrc;
};

class TakeStmtNode : public StmtNode{
public:
	TakeStmtNode(const Position * p, LocNode * inDst)
	: StmtNode(p), myDst(inDst){ }
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
private:
	LocNode * myDst;
};

class GiveStmtNode : public StmtNode{
public:
	GiveStmtNode(const Position * p, ExpNode * inSrc)
	: StmtNode(p), mySrc(inSrc){ }
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
private:
	ExpNode * mySrc;
};

class ExitStmtNode : public StmtNode{
public:
	ExitStmtNode(const Position * p) : StmtNode(p) { }
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
};

class PostDecStmtNode : public StmtNode{
public:
	PostDecStmtNode(const Position * p, LocNode * inLoc)
	: StmtNode(p), myLoc(inLoc){ }
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
private:
	LocNode * myLoc;
};

class PostIncStmtNode : public StmtNode{
public:
	PostIncStmtNode(const Position * p, LocNode * inLoc)
	: StmtNode(p), myLoc(inLoc){ }
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
private:
	LocNode * myLoc;
};

class IfStmtNode : public StmtNode{
public:
	IfStmtNode(const Position * p, ExpNode * condIn,
	  std::list<StmtNode *> * bodyIn)
	: StmtNode(p), myCond(condIn), myBody(bodyIn){ }
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
private:
	ExpNode * myCond;
	std::list<StmtNode *> * myBody;
};

class IfElseStmtNode : public StmtNode{
public:
	IfElseStmtNode(const Position * p, ExpNode * condIn,
	  std::list<StmtNode *> * bodyTrueIn,
	  std::list<StmtNode *> * bodyFalseIn)
	: StmtNode(p), myCond(condIn),
	  myBodyTrue(bodyTrueIn), myBodyFalse(bodyFalseIn) { }
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
private:
	ExpNode * myCond;
	std::list<StmtNode *> * myBodyTrue;
	std::list<StmtNode *> * myBodyFalse;
};

class WhileStmtNode : public StmtNode{
public:
	WhileStmtNode(const Position * p, ExpNode * condIn,
	  std::list<StmtNode *> * bodyIn)
	: StmtNode(p), myCond(condIn), myBody(bodyIn){ }
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
private:
	ExpNode * myCond;
	std::list<StmtNode *> * myBody;
};

class ReturnStmtNode : public StmtNode{
public:
	ReturnStmtNode(const Position * p, ExpNode * exp)
	: StmtNode(p), myExp(exp){ }
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
private:
	ExpNode * myExp;
};

class CallExpNode : public ExpNode{
public:
	CallExpNode(const Position * p, LocNode * inCallee,
	  std::list<ExpNode *> * inArgs)
	: ExpNode(p), myCallee(inCallee), myArgs(inArgs){ }
	void unparse(std::ostream& out, int indent) override;
	void unparseNested(std::ostream& out) override;
    bool nameAnalysis(SymbolTable * symTab) override;
private:
	LocNode * myCallee;
	std::list<ExpNode *> * myArgs;
};

class MemberFieldExpNode : public LocNode {
public:
	MemberFieldExpNode(const Position * p, LocNode * inBase,
	IDNode * inField)
	: LocNode(p), myBase(inBase), myField(inField) { }
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
    SemSymbol * getSymbol() override { return myBase->getSymbol();}
private:
	LocNode * myBase;
	IDNode * myField;
};

class BinaryExpNode : public ExpNode{
public:
	BinaryExpNode(const Position * p, ExpNode * lhs, ExpNode * rhs)
	: ExpNode(p), myExp1(lhs), myExp2(rhs) { }
    bool nameAnalysis(SymbolTable * symTab) override;
protected:
	ExpNode * myExp1;
	ExpNode * myExp2;
};

class PlusNode : public BinaryExpNode{
public:
	PlusNode(const Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	void unparse(std::ostream& out, int indent) override;
};

class MinusNode : public BinaryExpNode{
public:
	MinusNode(const Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	void unparse(std::ostream& out, int indent) override;
};

class TimesNode : public BinaryExpNode{
public:
	TimesNode(const Position * p, ExpNode * e1In, ExpNode * e2In)
	: BinaryExpNode(p, e1In, e2In){ }
	void unparse(std::ostream& out, int indent) override;
};

class DivideNode : public BinaryExpNode{
public:
	DivideNode(const Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	void unparse(std::ostream& out, int indent) override;
};

class AndNode : public BinaryExpNode{
public:
	AndNode(const Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	void unparse(std::ostream& out, int indent) override;
};

class OrNode : public BinaryExpNode{
public:
	OrNode(const Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	void unparse(std::ostream& out, int indent) override;
};

class EqualsNode : public BinaryExpNode{
public:
	EqualsNode(const Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	void unparse(std::ostream& out, int indent) override;
};

class NotEqualsNode : public BinaryExpNode{
public:
	NotEqualsNode(const Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	void unparse(std::ostream& out, int indent) override;
};

class LessNode : public BinaryExpNode{
public:
	LessNode(const Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	void unparse(std::ostream& out, int indent) override;
};

class LessEqNode : public BinaryExpNode{
public:
	LessEqNode(const Position * pos, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(pos, e1, e2){ }
	void unparse(std::ostream& out, int indent) override;
};

class GreaterNode : public BinaryExpNode{
public:
	GreaterNode(const Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	void unparse(std::ostream& out, int indent) override;
};

class GreaterEqNode : public BinaryExpNode{
public:
	GreaterEqNode(const Position * p, ExpNode * e1, ExpNode * e2)
	: BinaryExpNode(p, e1, e2){ }
	void unparse(std::ostream& out, int indent) override;
};

class UnaryExpNode : public ExpNode {
public:
	UnaryExpNode(const Position * p, ExpNode * expIn)
	: ExpNode(p){
		this->myExp = expIn;
	}
	virtual void unparse(std::ostream& out, int indent) override = 0;
    bool nameAnalysis(SymbolTable * symTab) override;
protected:
	ExpNode * myExp;
};

class NegNode : public UnaryExpNode{
public:
	NegNode(const Position * p, ExpNode * exp)
	: UnaryExpNode(p, exp){ }
	void unparse(std::ostream& out, int indent) override;
};

class NotNode : public UnaryExpNode{
public:
	NotNode(const Position * p, ExpNode * exp)
	: UnaryExpNode(p, exp){ }
	void unparse(std::ostream& out, int indent) override;
};

class VoidTypeNode : public TypeNode{
public:
	VoidTypeNode(const Position * p) : TypeNode(p){}
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
    std::string getType() override {
        return "void";
    }
};

class ClassTypeNode : public TypeNode{
public:
	ClassTypeNode(const Position * p, IDNode * inID)
	: TypeNode(p), myID(inID){}
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
    std::string getType() override {
        return myID->getName();
    }
    SemSymbol * getSymbol() override {
        return myID->getSymbol();
    }

private:
	IDNode * myID;
};

class PerfectTypeNode : public TypeNode{
public:
	PerfectTypeNode(const Position * p, TypeNode * inSub)
	: TypeNode(p), mySub(inSub){}
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
    std::string getType() override {
        std::string result = "perfect ";
        result += mySub->getType();
        return result;
    }
private:
	TypeNode * mySub;
};

class IntTypeNode : public TypeNode{
public:
	IntTypeNode(const Position * p): TypeNode(p){}
	void unparse(std::ostream& out, int indent) override;
	bool nameAnalysis(SymbolTable *) override;
    std::string getType() override {
        return "int";
    }
};

class BoolTypeNode : public TypeNode{
public:
	BoolTypeNode(const Position * p): TypeNode(p) { }
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
    std::string getType() override {
        return "bool";
    }
};

class IntLitNode : public ExpNode{
public:
	IntLitNode(const Position * p, const int numIn)
	: ExpNode(p), myNum(numIn){ }
	virtual void unparseNested(std::ostream& out) override{
		unparse(out, 0);
	}
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
private:
	const int myNum;
};

class StrLitNode : public ExpNode{
public:
	StrLitNode(const Position * p, const std::string strIn)
	: ExpNode(p), myStr(strIn){ }
	virtual void unparseNested(std::ostream& out) override{
		unparse(out, 0);
	}
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
private:
	 const std::string myStr;
};

class TrueNode : public ExpNode{
public:
	TrueNode(const Position * p): ExpNode(p){ }
	virtual void unparseNested(std::ostream& out) override{
		unparse(out, 0);
	}
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
};

class FalseNode : public ExpNode{
public:
	FalseNode(const Position * p): ExpNode(p){ }
	virtual void unparseNested(std::ostream& out) override{
		unparse(out, 0);
	}
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
};

class MagicNode : public ExpNode{
public:
	MagicNode(const Position * p): ExpNode(p){ }
	virtual void unparseNested(std::ostream& out) override{
		unparse(out, 0);
	}
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
};

class CallStmtNode : public StmtNode{
public:
	CallStmtNode(const Position * p, CallExpNode * expIn)
	: StmtNode(p), myCallExp(expIn){ }
	void unparse(std::ostream& out, int indent) override;
    bool nameAnalysis(SymbolTable * symTab) override;
private:
	CallExpNode * myCallExp;
};

} //End namespace drewno_mars

#endif

