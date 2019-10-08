#include "meta.hpp"
#include <tuple>
#include <list>
#include <string>
#include <unordered_map>
#include <iostream>
#include <locale>
#include <cctype>
#include <algorithm>

enum class TAG{
	OPERATOR,
	NUMBER,
	LEFT_BRACE,
	RIGHT_BRACE,
	VARIABLE,
	FUNCTION,
	CONTROL  // this is assigned for control flow tokens like WHILE, IF...
};

enum class OPERATORS{
	ADD,
	SUBSTRACT,
	DIVIDE,
	MULTIPLY,
	XOR
};

using OPER_TUPLE=std::tuple<OPERATORS, int, bool>;

class Token{
public:
	virtual ~Token();
	virtual const TAG getTag() const=0;
};

#ifdef NEG_SUPPORT
class Negatable{

private:
	bool isNegated;
protected:
	Negatable();
public:
	virtual ~Negatable();
	virtual bool getNegated() const;
	virtual void negate();
};
#endif

class Operator : public Token{
	int priority;
	bool isRightAssociative;
	enum OPERATORS oper;
public:
	static bool isOperator(const std::string::iterator& it);
	static void initOperatorsTable();
	bool operator<(const Operator& op) const;
	bool operator>(const Operator& op) const;
	bool operator==(const Operator& op) const;
	const TAG getTag() const override;
	const bool getAssoc() const;
	const enum OPERATORS getOperatorTag() const;
	Operator(std::string::iterator& it);
};

class Number : public Token
#ifdef NEG_SUPPORT
	     , public Negatable
#endif
{
	double num;
public:
	double operator+(const Number& oper) const;
	double operator-(const Number& oper) const;
	double operator*(const Number& oper) const;
	double operator/(const Number& oper) const;
	const TAG getTag() const override;
	double getNum() const;
	Number(double val);
#ifdef NEG_SUPPORT
	void negate();
#endif
};

class Brace : public Token{
	TAG brace;
public:
	const TAG getTag() const override;
	Brace(const std::string::iterator& it);
};

class Function : public Token
#ifdef NEG_SUPPORT
	     , public Negatable
#endif
{
	std::string name;
	int numberOfOperators;
	void* memAddress;
public:
	Function(const std::string& name);
	const TAG getTag() const override;
	std::string getName() const;
	double call() const;
};

std::ostream& operator<<(std::ostream& ost, enum OPERATORS oper);

#ifndef NDEBUG
std::ostream& operator<<(std::ostream& ost, const Token& tok);
#endif
