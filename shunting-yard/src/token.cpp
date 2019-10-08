#include "token.hpp"

#ifdef LIB_SUPPORT
extern std::unordered_map<std::string, void*> func_map;
#endif

std::unordered_map<std::string, OPER_TUPLE> prior_table; // gets operator as key, returns information on operator

/* Do following to add new operator:
   1. Add operator in one of arrays right below
   2. Add entry in initOperatorsTable()
   3. Create new OPERATORS enum class member
   4. Adjust performOperation()
 */

const std::string one_oper_array[4]={"+", "-", "/", "*"}; // one-charachter operators
const std::string alpha_oper_array[1]={"xor"}; // operators consisting of alphabetic characters

const int getMaxLengthOper(){ // returns length of largest alphabetic operator
	unsigned max_length=0;
	for(auto oper : alpha_oper_array)
		if(oper.length() > max_length)
			max_length=oper.length();

	return max_length;
}

const int MAX_LENGTH_OPER=getMaxLengthOper();// number of characters taken by longest alphabetic operator, change it if 'alpha_oper_array' is expanded

Token::~Token() {}

bool Operator::isOperator(const std::string::iterator& it){
	if(isalpha(*it)){		
		std::string oper_name=std::string(it, it + MAX_LENGTH_OPER); // string containing operator's name
		for(auto tmp : alpha_oper_array)
			if(oper_name.find(tmp) != std::string::npos)
				return true;
		return false;
	}
	else{ // if iterator points to non-alphabetic operator, it points to one-symbol operator
		for(auto oper : one_oper_array)
			if( std::string(1, *it) == oper )
				return true;
		return false;
	}
}

void Operator::initOperatorsTable(){
	prior_table["+"]=OPER_TUPLE(OPERATORS::ADD, 1, false);
	prior_table["-"]=OPER_TUPLE(OPERATORS::SUBSTRACT, 1, false);
	prior_table["*"]=OPER_TUPLE(OPERATORS::MULTIPLY, 2, false);
	prior_table["/"]=OPER_TUPLE(OPERATORS::DIVIDE, 2, false);
	prior_table["xor"]=OPER_TUPLE(OPERATORS::XOR, 3, false);
}


bool Operator::operator<(const Operator& op) const{
	if(this->priority < op.priority)
		return true;
	else
		return false;
}

bool Operator::operator>(const Operator& op) const{
	if(this->priority > op.priority)
		return true;
	else
		return false;
}

bool Operator::operator==(const Operator& op) const{
	if(this->priority == op.priority)
		return true;
	else
		return false;
}

const TAG Operator::getTag() const { return TAG::OPERATOR; }

const bool Operator::getAssoc() const { return isRightAssociative; } // "true" if right-associative, "false" otherwise

const enum OPERATORS Operator::getOperatorTag() const { return oper; }

Operator::Operator(std::string::iterator& it)
{
	std::string key;
	if(isalpha(*it)){
		std::string oper_name=std::string(it, it + MAX_LENGTH_OPER); // string containing operator's name
		while( isalpha(*(++it)) ); // make iterator point at position after alphabetic operator
		for(auto tmp : alpha_oper_array)
			if(oper_name.find(tmp) != std::string::npos){
				key=tmp;
				break;
			}
	}
	else key=std::string(1, *it++); /* increment could lead to error in expression parsing if operator can't be matched at all
					   Operator::isOperator() handles the case if the symbol being processed is not operator */

	const OPER_TUPLE& tuple=prior_table[key];
	oper=std::get<0>(tuple);
	priority=std::get<1>(tuple);
	isRightAssociative=std::get<2>(tuple);
}


double Number::operator+(const Number& oper) const{
	return this->getNum() + oper.getNum();
}

double Number::operator-(const Number& oper) const{
	return this->getNum() - oper.getNum();
}

double Number::operator*(const Number& oper) const{
	return this->getNum() * oper.getNum();
}

double Number::operator/(const Number& oper) const{
	return this->getNum() / oper.getNum();
}

const TAG Number::getTag() const { return TAG::NUMBER; }

double Number::getNum() const { return num; }

Number::Number(double val){
	num=val;
}

const TAG Brace::getTag() const { return brace; }
	
Brace::Brace(const std::string::iterator& it){
	if(*it=='(')
		brace=TAG::LEFT_BRACE;
	else if(*it==')')
		brace=TAG::RIGHT_BRACE;
}

#ifdef LIB_SUPPORT
Function::Function(const std::string& name){
	try{
		func_map.at(name);
	} catch(const std::out_of_range& exc){
		std::cout << "No function loaded with name " << name << std::endl;
		exit(EXIT_FAILURE);
	}
	
	this->name = name;
	numberOfOperators = 0;
	memAddress = func_map[name];
}

const TAG Function::getTag() const { return TAG::FUNCTION; }

double Function::call() const {
	using func_t = double (*)();
	func_t func_pnt = (func_t) memAddress;
	double result = func_pnt();
# ifdef NEG_SUPPORT
	if(getNegated())
		result = -result;
# endif
	return result;
}

std::string Function::getName() const { return name; }
#endif

#ifdef NEG_SUPPORT
Negatable::Negatable() : isNegated(false) {}

Negatable::~Negatable() {}

bool Negatable::getNegated() const{
	return isNegated;
}

void Negatable::negate(){
	isNegated = !isNegated;
}

void Number::negate(){
	Negatable::negate();
	num = -num;
}

#endif

std::ostream& operator<<(std::ostream& ost, enum OPERATORS oper){
	std::string tag_print;
	switch(oper){
	case OPERATORS::ADD:
		tag_print=" + ";
		break;
	case OPERATORS::SUBSTRACT:
		tag_print=" - ";
		break;
	case OPERATORS::DIVIDE:
		tag_print=" / ";
		break;
	case OPERATORS::MULTIPLY:
		tag_print=" * ";
		break;
	case OPERATORS::XOR:
		tag_print=" xor ";
		break;
	}
	return ost << tag_print;
}

#ifndef NDEBUG
std::ostream& operator<<(std::ostream& ost, const Token& tok){
	std::string tag_print;
	switch(tok.getTag()){
	case TAG::OPERATOR:
		//tag_print="OPERATOR";
		switch(static_cast<const Operator&>(tok).getOperatorTag()){
		case OPERATORS::ADD:
			tag_print=" + ";
			break;
		case OPERATORS::SUBSTRACT:
			tag_print=" - ";
			break;
		case OPERATORS::DIVIDE:
			tag_print=" / ";
			break;
		case OPERATORS::MULTIPLY:
			tag_print=" * ";
			break;
		case OPERATORS::XOR:
			tag_print=" xor ";
			break;
		}
		break;
	case TAG::NUMBER:
		tag_print=std::to_string(static_cast<const Number&>(tok).getNum());
                //tag_print="NUMBER";
		break;
	case TAG::LEFT_BRACE:
		//tag_print="LEFT_BRACE";
		tag_print="(";
		break;
	case TAG::RIGHT_BRACE:
		//tag_print="RIGHT_BRACE";
		tag_print=")";
		break;
	case TAG::VARIABLE:
		//tag_print="VARIABLE";
		break;
	case TAG::FUNCTION:
		//tag_print="FUNCTION";
		tag_print = static_cast<const Function&>(tok).getName() + "()";
		break;
	case TAG::CONTROL:
		//tag_print="CONTROL";
		break;
	}
	return ost << tag_print;
}
#endif
