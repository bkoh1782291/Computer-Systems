// Brian Koh Lit Yang
// a1782291
// convert Hack VM Code into Hack Assembler

#include "iobuffer.h"
#include "tokeniser.h"
#include "assembler-internal.h"
#include <iostream>
#include <string>
#include <algorithm>

// to make out programs a bit neater
using namespace std ;

using namespace CS_IO_Buffers ;
using namespace Hack_Virtual_Machine ;

// The translation functions must output Hack Assembly language by calling the function, 
// output_assembler(), which is passed a single line of Hack Assembly language or a comment. 
// Each translation function starts and ends with start_* and end_* functions so that program 
// output will be in a form suitable for testing by the precompiled executable program bin/simulator.

////////////////////////////////////////////////////////////////
/************     MODIFY CODE BETWEEN HERE     **************/

// global variables

// stores the function name entirely (label)
string func_name = "";

// stores only the class name of the function
string class_name = "";

// counters for functions
int call_count = 0;
int eq_count = 0;
int gt_count = 0;
int lt_count = 0;

// Pop y, pop x, push x + y.
static void token_add()
{
    output_assembler("@SP"); 
    output_assembler("AM=M-1");
    output_assembler("D=M");
    output_assembler("A=A-1");
    output_assembler("M=D+M");
}

// Pop y, pop x, push x - y.
static void token_sub()
{
    output_assembler("@SP");
    output_assembler("AM=M-1");
    output_assembler("D=M");
    output_assembler("A=A-1");
    output_assembler("M=M-D");
}

// Pop x, push −x.
static void token_neg()
{
    output_assembler("@SP");
    output_assembler("A=M");
    output_assembler("A=A-1");
    output_assembler("M=-M");
}

// Pop x, push NOT(x).
static void token_not()
{
    output_assembler("@SP");
    output_assembler("A=M");
    output_assembler("A=A-1");
    output_assembler("M=!M");
}

// Pop y, pop x, push AND(x, y).
static void token_and()
{
    output_assembler("@SP");
    output_assembler("AM=M-1");
    output_assembler("D=M");
    output_assembler("A=A-1");
    output_assembler("M=D&M");
}

// Pop y, pop x, push OR(x, y).
static void token_or()
{
    output_assembler("@SP");
    output_assembler("AM=M-1");
    output_assembler("D=M");
    output_assembler("A=A-1");
    output_assembler("M=D|M");
}

// Pop y, pop x, push -1 if x = y, else 0.
static void token_eq()
{
    // labels for easier legibility
    string label_EQ = func_name + "$EQ" + to_string(eq_count);
    eq_count++;
    output_assembler("@SP");
    output_assembler("AM=M-1");
    output_assembler("D=M");
    output_assembler("@SP");
    output_assembler("A=M-1");
    output_assembler("D=M-D");

    // push -1
    output_assembler("M=-1");

    // goto EQ
    output_assembler("@" + label_EQ);

    // x != y 
    output_assembler("D;JEQ");
    output_assembler("@SP");
    output_assembler("A=M-1");

    // push 0
    output_assembler("M=0");

    // label EQ
    output_assembler("(" + label_EQ + ")");
}

// Pop y, pop x, push -1 if x > y, else 0.
static void token_gt()
{
    // labels for easier legibility
    string label_POS    =  func_name + "$GT_POS" + to_string(gt_count);
    string label_NEG    =  func_name + "$GT_NEG" + to_string(gt_count);
    string label_TRUE   =  func_name + "$GT_TRUE" + to_string(gt_count);
    string label_FALSE  =  func_name + "$GT_FALSE" + to_string(gt_count);
    string label_NORM   =  func_name + "$GT_NORM" + to_string(gt_count);
    string label_END    =  func_name + "$GT_END" + to_string(gt_count);

    gt_count++;
    output_assembler("@SP");
    output_assembler("AM=M-1");
    output_assembler("D=M");

    // pop y
    output_assembler("@R14");
    output_assembler("M=D");
    output_assembler("@SP");
    output_assembler("A=M-1");
    output_assembler("D=M");

    // pop x
    output_assembler("@R13");
    output_assembler("M=D");
    output_assembler("@R13");
    output_assembler("D=M");

    // goto GT_POS 
    output_assembler("@" + label_POS);
    output_assembler("D;JGE");

    // goto GT_NEG
    output_assembler("@" + label_NEG);
    output_assembler("0;JMP");

    // label GT_POS
    output_assembler("(" + label_POS + ")");
    output_assembler("@R14");
    output_assembler("D=M");

    // if positive, and less than
    output_assembler("@" + label_TRUE);
    output_assembler("D;JLT");

    // jump
    output_assembler("@" + label_NORM);
    output_assembler("0;JMP");

    // label negative
    output_assembler("(" + label_NEG + ")");
    output_assembler("@R14");
    output_assembler("D=M");

    // goto GT_NEG
    output_assembler("@" + label_FALSE);
    output_assembler("D;JGE");

    // goto GT_NORM
    output_assembler("@" + label_NORM);
    output_assembler("0;JMP"); 

    // label NORM-al
    output_assembler("(" + label_NORM + ")");
    output_assembler("@R13");
    output_assembler("D=M");
    output_assembler("@R14");
    output_assembler("D=D-M");

    // if x > y
    output_assembler("@" + label_TRUE);
    output_assembler("D;JGT");

    // else jump
    output_assembler("@" + label_FALSE);
    output_assembler("0;JMP");

    // label true if JGT = 1
    output_assembler("(" + label_TRUE + ")");
    output_assembler("@SP");
    output_assembler("A=M-1");

    // push -1 if x > y
    output_assembler("M=-1");
    output_assembler("@" + label_END);
    output_assembler("0;JMP");

    // label false (else if x < y then 0)
    output_assembler("(" + label_FALSE + ")");
    output_assembler("@SP");
    output_assembler("A=M-1");

    // push 0 if x < y
    output_assembler("M=0");

    // goto GT_END
    output_assembler("@" + label_END);

    // jump
    output_assembler("0;JMP");

    // label end
    output_assembler("(" + label_END + ")");
}

// Pop y, pop x, push -1 if x < y, else 0.
static void token_lt()
{
    // labels for easier legibility
    // words after the '$" dont really matter, as long as they are different and no duplicates occur.
    string label_POS    =  func_name + "$LT_POS" + to_string(gt_count);
    string label_NEG    =  func_name + "$LT_NEG" + to_string(gt_count);
    string label_TRUE   =  func_name + "$LT_TRUE" + to_string(gt_count);
    string label_FALSE  =  func_name + "$LT_FALSE" + to_string(gt_count);
    string label_NORM   =  func_name + "$LT_NORM" + to_string(gt_count);
    string label_END    =  func_name + "$LT_END" + to_string(gt_count);

    gt_count++;
    output_assembler("@SP");
    output_assembler("AM=M-1");
    output_assembler("D=M");

    // pop x
    output_assembler("@R13");
    output_assembler("M=D");
    output_assembler("@SP");
    output_assembler("A=M-1");
    output_assembler("D=M");

    // pop y
    output_assembler("@R14");
    output_assembler("M=D");
    output_assembler("@R13");
    output_assembler("D=M");

    // goto GT_POS
    output_assembler("@" + label_POS);
    output_assembler("D;JGE");

    // goto GT_NEG
    output_assembler("@" + label_NEG);
    output_assembler("0;JMP");

    // label GT_POS
    output_assembler("(" + label_POS + ")");
    output_assembler("@R14");
    output_assembler("D=M");

    // if positive, and less than
    output_assembler("@" + label_TRUE);
    output_assembler("D;JLT");

    // jump
    output_assembler("@" + label_NORM);
    output_assembler("0;JMP");

    // label negative
    output_assembler("(" + label_NEG + ")");
    output_assembler("@R14");
    output_assembler("D=M");

    // goto GT_NEG
    output_assembler("@" + label_FALSE);
    output_assembler("D;JGE");

    // goto GT_NORM
    output_assembler("@" + label_NORM);
    output_assembler("0;JMP"); 

    // label NORM-al
    output_assembler("(" + label_NORM + ")");
    output_assembler("@R13");
    output_assembler("D=M");
    output_assembler("@R14");
    output_assembler("D=D-M");

    // if x > y 
    output_assembler("@" + label_TRUE);
    output_assembler("D;JGT");

    // else jump
    output_assembler("@" + label_FALSE);
    output_assembler("0;JMP");

    // label true if JGT = 1
    output_assembler("(" + label_TRUE + ")");
    output_assembler("@SP");
    output_assembler("A=M-1");

    // push -1 if x > y
    output_assembler("M=-1");
    output_assembler("@" + label_END);
    output_assembler("0;JMP");

    // label false (else if x < y then 0)
    output_assembler("(" + label_FALSE + ")");
    output_assembler("@SP");
    output_assembler("A=M-1");

    // push 0 if x < y
    output_assembler("M=0");

    // goto GT_END
    output_assembler("@" + label_END);

    // jump
    output_assembler("0;JMP");

    // label end
    output_assembler("(" + label_END + ")");
}

// function to implement return command 
static void token_return()
{
    // local temp variable
    output_assembler("@LCL");
    output_assembler("D=M");

    // R13 points to temp
    output_assembler("@R13");
    output_assembler("M=D");

    // put the return address to a temporary variable
    output_assembler("@5");
    output_assembler("A=D-A");
    output_assembler("D=M");

    // R14 points to return
    output_assembler("@R14");
    output_assembler("M=D");

    // reposition the return value for the callee
    output_assembler("@SP");
    output_assembler("AM=M-1");
    output_assembler("D=M");

    // copies the return value onto argument 0
    output_assembler("@ARG");
    output_assembler("A=M");
    output_assembler("M=D");

    // restore SP
    output_assembler("D=A");
    output_assembler("@SP");
    output_assembler("M=D+1");

    // restore THAT
    output_assembler("@R13");
    output_assembler("AM=M-1");
    output_assembler("D=M");
    output_assembler("@THAT");
    output_assembler("M=D");

    // restore THIS
    output_assembler("@R13");
    output_assembler("AM=M-1");
    output_assembler("D=M");
    output_assembler("@THIS");
    output_assembler("M=D");

    // restore ARG
    output_assembler("@R13");
    output_assembler("AM=M-1");
    output_assembler("D=M");
    output_assembler("@ARG");
    output_assembler("M=D");

    // restore LOCAL
    output_assembler("@R13");
    output_assembler("AM=M-1");
    output_assembler("D=M");
    output_assembler("@LCL");
    output_assembler("M=D");

    // jump to return
    output_assembler("@R14");
    output_assembler("A=M");
    output_assembler("0;JMP");
}

// translate vm operator command into assembly language
static void translate_vm_operator(TokenKind the_op)
{
    start_of_vm_operator_command(the_op) ;

    // calls for all the operator functions
    switch (the_op)
    {
        case tk_add:
            token_add();
            break;

        case tk_sub:
            token_sub();
            break;

        case tk_neg:
            token_neg();
            break;

        case tk_and:
            token_and();
            break;

        case tk_or:
            token_or();
            break;

        case tk_not:
            token_not();
            break;

        case tk_eq:
            token_eq();
            break;
        
        case tk_gt:
            token_gt();
            break;

        case tk_lt:
            token_lt();
            break;

        case tk_return:
            token_return();
            break;

        default: ;
    }
    end_of_vm_command() ;
}

// translate vm jump command into assembly language
static void translate_vm_jump(TokenKind jump, string label)
{
    start_of_vm_jump_command(jump,label) ;

    // code for goto, if-goto and label
    if (jump == tk_goto)
    {
        output_assembler("@" + func_name + "$" + label);
        output_assembler("0;JMP");
    } 
    else if (jump == tk_if_goto)
    {
        output_assembler("@SP");
        output_assembler("AM=M-1");
        output_assembler("D=M");
        output_assembler("@" + func_name + "$" + label);
        output_assembler("D;JNE");
    }
    else if (jump == tk_label)
    {
        output_assembler("(" + func_name + "$" + label + ")");
    }
    end_of_vm_command() ;
}

// translate vm call or function command into assembly language
static void translate_vm_function(TokenKind func, string label, int n)
{
    start_of_vm_func_command(func,label,n) ;
    // code for either function or call command
    if ( func == tk_function)
    {
        output_assembler("(" + label + ")");

        // update label when a new function is called
        // assigns label to a new string
        func_name = label;

        // find the "." and remove the prefixes at the end
        int flag = label.find(".");

        // used in 'static' operations
        class_name = label.substr(0, flag);

        // push on n 0's to the stack (push constant 0)
        for (int i=0; i < n; i++)
        {
            output_assembler("@SP");
            output_assembler("AMD=M+1");
            output_assembler("AD=A-1");
            output_assembler("M=0");
        }
    } 
    else if (func == tk_call) 
    {
        // implementing call with n arguments
        string returnSymbol = func_name + "$RETURN" + to_string(call_count);

        call_count++;

        // push return-addr
        output_assembler("@" + returnSymbol);
        output_assembler("D=A");
        output_assembler("@SP");
        output_assembler("A=M");
        output_assembler("M=D");
        output_assembler("@SP");
        output_assembler("M=M+1");

        // push LCL
        output_assembler("@LCL");
        output_assembler("D=M");
        output_assembler("@SP");
        output_assembler("A=M");
        output_assembler("M=D");
        output_assembler("@SP");
        output_assembler("M=M+1");

        // push ARG
        output_assembler("@ARG");
        output_assembler("D=M");
        output_assembler("@SP");
        output_assembler("A=M");
        output_assembler("M=D");
        output_assembler("@SP");
        output_assembler("M=M+1");

        // push THIS
        output_assembler("@THIS");
        output_assembler("D=M");
        output_assembler("@SP");
        output_assembler("A=M");
        output_assembler("M=D");
        output_assembler("@SP");
        output_assembler("M=M+1");

        // push THAT
        output_assembler("@THAT");
        output_assembler("D=M");
        output_assembler("@SP");
        output_assembler("A=M");
        output_assembler("M=D");
        output_assembler("@SP");
        output_assembler("M=M+1");

        // reposition ARG
        output_assembler("@" + to_string(n));

        // ARG = SP-n-5
        output_assembler("D=A");
        output_assembler("@5");
        output_assembler("D=D+A");
        output_assembler("@SP");
        output_assembler("D=M-D");
        output_assembler("@ARG");
        output_assembler("M=D");

        // reposition LCL
        output_assembler("@SP");
        output_assembler("D=M");

        // LCL = SP
        output_assembler("@LCL");
        output_assembler("M=D");

        // goto g
        output_assembler("@" + label) ;

        // jump to execute call
        output_assembler("0;JMP"); 

        // declare label
        output_assembler("(" + returnSymbol + ")") ;
    }
    end_of_vm_command() ;
}

// translate vm push or pop command into assembly language
static void translate_vm_stack(TokenKind stack,TokenKind segment,int offset)
{
    start_of_vm_stack_command(stack,segment,offset) ;

    // operations for pop
    if (stack == tk_pop)
    {
        if (segment == tk_pointer)
        {
            output_assembler("@SP");
            output_assembler("AM=M-1");
            output_assembler("D=M");
            output_assembler("@R" + to_string(3 + offset));
            output_assembler("M=D");
        }
        else if (segment == tk_temp)
        {
            output_assembler("@SP");
            output_assembler("AM=M-1");
            output_assembler("D=M");
            output_assembler("@R" + to_string(5 + offset));
            output_assembler("M=D");
        }
        else if (segment == tk_static)
        {
            output_assembler("@SP");
            output_assembler("AM=M-1");
            output_assembler("D=M");
            output_assembler("@" + class_name + "." + to_string(offset));
            output_assembler("M=D");
        }
        else
        {
            if (segment == tk_argument)
            {
                output_assembler("@ARG");
            }
            else if (segment == tk_local)
            {
                output_assembler("@LCL");
            }
            else if (segment == tk_that)
            {
                output_assembler("@THAT");
            }
            else if (segment == tk_this)
            {
                output_assembler("@THIS");
            }
            output_assembler("D=M");
            output_assembler("@" + to_string(offset));
            output_assembler("D=D+A");
            output_assembler("@R13");
            output_assembler("M=D");
            output_assembler("@SP");
            output_assembler("AM=M-1");
            output_assembler("D=M");
            output_assembler("@R13");
            output_assembler("A=M");
            output_assembler("M=D");
        }
    }
    else if (stack == tk_push)
    {
        if (segment == tk_constant)
        {
            output_assembler("@" + to_string(offset));
            output_assembler("D=A");
            output_assembler("@SP");
            output_assembler("A=M");
            output_assembler("M=D");
            output_assembler("@SP");
            output_assembler("M=M+1");
        }
        else if (segment == tk_static)
        {
            output_assembler("@" + class_name + "." + to_string(offset)); 
            output_assembler("D=M");
            output_assembler("@SP");
            output_assembler("A=M");
            output_assembler("M=D");
            output_assembler("@SP");
            output_assembler("M=M+1");
        }
        else if (segment == tk_temp)
        {
            output_assembler("@R" + to_string(5 + offset)); 
            output_assembler("D=M");
            output_assembler("@SP");
            output_assembler("A=M");
            output_assembler("M=D");
            output_assembler("@SP");
            output_assembler("M=M+1");
        }
        else if (segment == tk_pointer)
        {
            output_assembler("@R" + to_string(3 + offset)); 
            output_assembler("D=M");
            output_assembler("@SP");
            output_assembler("A=M");
            output_assembler("M=D");
            output_assembler("@SP");
            output_assembler("M=M+1");
        }
        else
        {
            if (segment == tk_argument)
            {
                output_assembler("@ARG");
            }
            else if (segment == tk_local)
            {
                output_assembler("@LCL");
            }
            else if (segment == tk_that)
            {
                output_assembler("@THAT");
            }
            else if (segment == tk_this)
            {
                output_assembler("@THIS");
            }
            output_assembler("D=M");
            output_assembler("@" + to_string(offset));
            output_assembler("A=D+A");
            output_assembler("D=M");
            output_assembler("@SP");
            output_assembler("A=M");
            output_assembler("M=D");
            output_assembler("@SP");
            output_assembler("M=M+1");
        }
    }
    end_of_vm_command() ;
}


/************            AND HERE              **************/
///////////////////////////////////////////////////////////////

// grammar to be parsed:
// vm_class ::= command* eoi
// command ::= vm_operator | vm_jump | vm_func | vm_stack
// vm_operator ::= 'add' | 'and' | 'eq' | 'gt' | 'lt' | 'neg' | 'not' | 'or' | 'sub' | 'return'
// vm_jump ::= jump label
// vm_func ::= func label number
// vm_stack ::= stack segment number

// forward declare parsing functions - one per rule
static void parse_class() ;
static void parse_command() ;
static void parse_op() ;
static void parse_jump() ;
static void parse_func() ;
static void parse_stack() ;

// Note: always read one token after the one recognised

// vm_class ::= command* eoi
static void parse_class()
{
    // tell the output system we are starting to translate VM commands for a Jack class
    start_of_vm_class() ;

    while ( have(tk_vm_command) ) parse_command() ;
    mustbe(tk_eoi) ;

    // tell the output system we have just finished translating VM commands for a Jack class
    end_of_vm_class() ;
}

// command ::= vm_operator | vm_jump | vm_func | vm_stack
static void parse_command()
{
    if ( have(tk_vm_operator) ) parse_op() ; else
    if ( have(tk_vm_jump) )     parse_jump() ; else
    if ( have(tk_vm_func) )     parse_func() ; else
    if ( have(tk_vm_stack) )    parse_stack() ; else
    mustbe(tk_vm_command) ;
}

// vm_operator ::= 'add' | 'and' | 'eq' | 'gt' | 'lt' | 'neg' | 'not' | 'or' | 'sub' | 'return'
static void parse_op()
{
    TokenKind the_op = token_kind(mustbe(tk_vm_operator)) ;

    translate_vm_operator(the_op) ;
}

// vm_jump ::= jump label
// jump command - label, goto label or if-goto label
static void parse_jump()
{
    TokenKind command = token_kind(mustbe(tk_vm_jump)) ;
    string label = token_spelling(mustbe(tk_a_label)) ;

    translate_vm_jump(command,label) ;
}

// vm_func ::= func label number
// call function-name #args or function function-name #locals
static void parse_func()
{
    TokenKind command = token_kind(mustbe(tk_vm_func)) ;
    string label = token_spelling(mustbe(tk_a_label)) ;
    int number = token_ivalue(mustbe(tk_a_number)) ;

    translate_vm_function(command,label,number) ;
}

// vm_stack ::= stack segment number
// push segment offset or pop segment offset
static void parse_stack()
{
    TokenKind stack = token_kind(mustbe(tk_vm_stack)) ;
    TokenKind segment = token_kind(mustbe(tk_vm_segment)) ;
    int number = token_ivalue(mustbe(tk_a_number)) ;

    translate_vm_stack(stack,segment,number) ;
}

// main program
int main(int argc,char **argv)
{
    // initialise the tokeniser by reading the first token
    next_token() ;

    // parse a class and output VM code
    parse_class() ;

    // flush the output and any errors
    print_output() ;
    print_errors() ;
}



