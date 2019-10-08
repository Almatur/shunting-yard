#include "meta.hpp"
#include "token.hpp"
#include <list>
#include <stack>
#include <vector>
#include <unordered_map>
#include <utility>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <cstddef>

#ifdef LIB_SUPPORT
extern void importLibraries();
extern void closeLibraries();
#endif

std::list<Token*> tok_list; // list of tokens
std::stack<Token*> tok_stack; // in terms of shunting yard algorithm, it is operator stack
std::list<Token*> tok_queue; // in terms of shunting yard algorithm, this variable functions as operands-and-operators queue
std::string expr;

inline void input_error_detected(std::string&& message=""){ // use it to specify errors while processing expression and its elements
	std::cout << "Expression input error" << message << std::endl;
	exit(1);
}

inline void getInput(){
#ifdef LIB_SUPPORT
	importLibraries();
#endif
	std::cout << "Enter expression: ";
	std::getline(std::cin, expr);
}

void createList(){ // creates list of tokens(tok_list)
	bool incr_flag=false;
	for(auto string_it=expr.begin(); string_it<expr.end(); ){

		if(*string_it=='(' || *string_it==')')
			tok_list.push_back( new Brace(string_it) ), incr_flag=true;

		else if(Operator::isOperator(string_it))
			tok_list.push_back( new Operator(string_it) );

		else if(isdigit(*string_it)){
			auto substring_it=string_it;

			while(isdigit(*substring_it) || *substring_it=='.')
				substring_it++;

			std::string substring(string_it, substring_it);
			tok_list.push_back( new Number(std::stof(substring, nullptr)) );
			string_it=substring_it;
		}
#ifdef LIB_SUPPORT
		else if(isalpha(*string_it)){ // it is either variable or function
			std::string lex_name(1, *string_it);

			while(isalpha( *(++string_it)) )
				lex_name += *string_it;

			if( *string_it == '('){
				tok_list.push_back(new Function(lex_name));
				string_it += 2; // without function arguments support, it shifts for 2 symbols to cover parentheses
			}
			else{
				input_error_detected(": no parentheses detected for " + lex_name + "() function");
				exit(EXIT_FAILURE);
			}
		}
#endif

		if(*string_it==' ' || incr_flag) // this condition is put instead of loop increment
			string_it++, incr_flag=false;
	}
#ifndef NDEBUG
	for(auto tok : tok_list)
		std::cout << *tok;
	std::cout << std::endl;
#endif
}

void parseList(){ // shunting-yard algorithm implementation itself
	for(auto tok_it=tok_list.begin(); tok_it!=tok_list.end(); tok_it++){
		auto token=*tok_it;
		TAG token_tag=token->getTag();

		if(token_tag == TAG::NUMBER)
			tok_queue.push_back(token);
#ifdef LIB_SUPPORT
		else if(token_tag == TAG::FUNCTION)
			tok_stack.push(token);
#endif
		else if(token_tag == TAG::OPERATOR){
			Operator& op_token = *static_cast<Operator*>(token);
			while(!tok_stack.empty()){
				TAG stack_top_tag = tok_stack.top()->getTag();
				const Operator& top_oper = *dynamic_cast<Operator*>(tok_stack.top());

				if(&top_oper == nullptr)
					input_error_detected();

				if(  (stack_top_tag == TAG::FUNCTION ||
				     (stack_top_tag == TAG::OPERATOR && top_oper > op_token ) ||
				      (stack_top_tag == TAG::OPERATOR && top_oper == op_token && !top_oper.getAssoc() ) )){
					tok_queue.push_back(tok_stack.top());
					tok_stack.pop();
				}
				else
					break;
			}
			tok_stack.push(token);
		}
		else if(token_tag == TAG::LEFT_BRACE){
#ifdef NEG_SUPPORT
			auto saved_tok_it = tok_it; //we have to remove excessive tokens so that they don't interfere operations on stack/queue
			tok_it++;
			auto next_token=*tok_it;
			if(next_token->getTag()==TAG::OPERATOR &&
			   static_cast<Operator*>(next_token)->getOperatorTag()==OPERATORS::SUBSTRACT){

				tok_list.erase(saved_tok_it);

				saved_tok_it = tok_it;
				tok_it++;
				next_token=*tok_it;
				tok_list.erase(saved_tok_it);

				if(next_token->getTag()==TAG::NUMBER){
					static_cast<Number*>(next_token)->negate();
					tok_queue.push_back(next_token);

					saved_tok_it = ++tok_it;
					tok_it--;
					tok_list.erase(saved_tok_it);
                                                    // in case you want to handle parenthesis mismatch, you have to specify that you might encounter
					            // input error here					
				}
				else if(next_token->getTag()==TAG::FUNCTION){
					static_cast<Function*>(next_token)->negate();
					tok_stack.push(next_token);

					saved_tok_it = ++tok_it;
					tok_it--;
					tok_list.erase(saved_tok_it);
                                                    // in case you want to handle parenthesis mismatch, you have to specify that you might encounter
					            // input error here
				}
				else{
					input_error_detected(": minus sign before unallowed token");
					exit(EXIT_FAILURE);
				}
			}
			else{
				tok_it--;
				tok_stack.push(token);
			}
#else
			tok_stack.push(token);
#endif
		}
		else if(token_tag == TAG::RIGHT_BRACE){

			while(!tok_stack.empty() && tok_stack.top()->getTag() != TAG::LEFT_BRACE){
				tok_queue.push_back(tok_stack.top());
				tok_stack.pop();
			}
			if(!tok_stack.empty() && tok_stack.top()->getTag() == TAG::LEFT_BRACE)
				tok_stack.pop();			
		}
	}
	while(!tok_stack.empty()){
		tok_queue.push_back(tok_stack.top());
		tok_stack.pop();
	}
}

Number& performOperation(Number& first_operand, Number& second_operand, Operator& oper){
	double garbage_ptr;

	switch(oper.getOperatorTag()){
	case OPERATORS::ADD:
		return *new Number(first_operand + second_operand);
		break;
	case OPERATORS::SUBSTRACT:
		return *new Number(first_operand - second_operand);
		break;
	case OPERATORS::MULTIPLY:
		return *new Number(first_operand * second_operand);
		break;
	case OPERATORS::DIVIDE:
		return *new Number(first_operand / second_operand);
		break;
	case OPERATORS::XOR:
		if( modf(first_operand.getNum(), &garbage_ptr) == 0.0 &&
		    modf(second_operand.getNum(), &garbage_ptr ) == 0.0 )
			return *new Number(static_cast<int>(first_operand.getNum()) ^
					   static_cast<int>(second_operand.getNum()) );
		else
			input_error_detected(": xor can't be applied to non-integral values");
		break;
	}
	std::cout << "Cannot perform operation on " << first_operand.getNum() << " " << second_operand.getNum()
		  << " " << oper.getOperatorTag() << std::endl;
	exit(EXIT_FAILURE);
	return *new Number(NAN);
}

void parseRPN(){ // parses RPN queue(tok_queue) and prints the final result of expression
	using LIST_IT = std::list<Token*>::iterator;

	LIST_IT first_operand,
		second_operand,
		it;

#ifdef LIB_SUPPORT
	LIST_IT func;
#endif

	for(it=tok_queue.begin(); it != tok_queue.end(); it++){
		if( (*it)->getTag() == TAG::OPERATOR){

			first_operand=std::prev(it, 2);
			second_operand=std::prev(it, 1);


			if( (*first_operand)->getTag() != TAG::NUMBER ||
			    (*second_operand)->getTag() != TAG::NUMBER)
				input_error_detected();

			Number& result=performOperation(static_cast<Number&>(**first_operand),
							static_cast<Number&>(**second_operand),
							static_cast<Operator&>(**it) );
			/*it_copy=it;
			  it_copy++;

			bool tmp = (first_operand == tok_queue.begin());*/

			tok_queue.erase(first_operand);
			tok_queue.erase(second_operand);
			it=tok_queue.erase(it);

			/*if(tmp){
				tok_queue.push_front(static_cast<const Token*>(&result));
				it=tok_queue.begin();
			}
			else
			it=tok_queue.insert(it_copy, static_cast<const Token*>(&result));*/

			it=tok_queue.insert(it, static_cast<Token*>(&result) );
		}
#ifdef LIB_SUPPORT
		else if( (*it)->getTag() == TAG::FUNCTION){
			Function& func_tok = static_cast<Function&>(**it);
			double result = func_tok.call();
			it = tok_queue.erase(it);
			it = tok_queue.insert(it, static_cast<Token*>(new Number(result)));
		}
#endif
	}

	if(tok_queue.size() != 1)
		input_error_detected();
	else
		std::cout << static_cast<Number&>(*tok_queue.front()).getNum() << std::endl;
}

int main(){
	Operator::initOperatorsTable();
	getInput();
	createList();
	parseList();
	parseRPN();
#ifdef LIB_SUPPORT
	closeLibraries();
#endif
}
