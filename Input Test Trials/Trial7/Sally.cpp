// File: Sally.cpp
//
// CMSC 341 Fall 2018 Project 2
//
// Implementation of member functions of Sally Forth interpreter
//

#include <iostream>
#include <string>
#include <list>
#include <stack>
#include <stdexcept>
#include <cstdlib>
using namespace std ;

#include "Sally.h"


// Basic Token constructor. Just assigns values.
//
Token::Token(TokenKind kind, int val, string txt) {
   m_kind = kind ;
   m_value = val ;
   m_text = txt ;
}


// Basic SymTabEntry constructor. Just assigns values.
//
SymTabEntry::SymTabEntry(TokenKind kind, int val, operation_t fptr) {
   m_kind = kind ;
   m_value = val ;
   m_dothis = fptr ;
}


// Constructor for Sally Forth interpreter.
// Adds built-in functions to the symbol table.
//
Sally::Sally(istream& input_stream) :
   istrm(input_stream)  // use member initializer to bind reference
{
   doSkip.push(false); // default option, should NOT come off
   lookingforKeyWord.push("~"); //i really hope this is as good as null

   symtab["DUMP"]    =  SymTabEntry(KEYWORD,0,&doDUMP) ;

   symtab["+"]    =  SymTabEntry(KEYWORD,0,&doPlus) ;
   symtab["-"]    =  SymTabEntry(KEYWORD,0,&doMinus) ;
   symtab["*"]    =  SymTabEntry(KEYWORD,0,&doTimes) ;
   symtab["/"]    =  SymTabEntry(KEYWORD,0,&doDivide) ;
   symtab["%"]    =  SymTabEntry(KEYWORD,0,&doMod) ;
   symtab["NEG"]  =  SymTabEntry(KEYWORD,0,&doNEG) ;

   symtab["."]    =  SymTabEntry(KEYWORD,0,&doDot) ;
   symtab["SP"]   =  SymTabEntry(KEYWORD,0,&doSP) ;
   symtab["CR"]   =  SymTabEntry(KEYWORD,0,&doCR) ;

   symtab["DUP"]  =  SymTabEntry(KEYWORD,0,&doDUP) ;
   symtab["DROP"] =  SymTabEntry(KEYWORD,0,&doDROP) ;
   symtab["SWAP"] =  SymTabEntry(KEYWORD,0,&doSWAP) ;
   symtab["ROT"]  =  SymTabEntry(KEYWORD,0,&doROT) ; //rot in Hell, that is.

   symtab["SET"]  =  SymTabEntry(KEYWORD,0,&doSET) ;
   symtab["@"]    =  SymTabEntry(KEYWORD,0,&doCALL) ;
   symtab["!"]    =  SymTabEntry(KEYWORD,0,&doREPL) ;

   symtab["<"]    =  SymTabEntry(KEYWORD,0,&doLESSDEN) ;
   symtab["<="]   =  SymTabEntry(KEYWORD,0,&doLESSDEN_OR_EQ) ;
   symtab[">"]    =  SymTabEntry(KEYWORD,0,&doGRDEN) ;
   symtab[">="]   =  SymTabEntry(KEYWORD,0,&doGRDEN_OR_EQ) ;
   symtab["=="]   =  SymTabEntry(KEYWORD,0,&doEQ) ;
   symtab["!="]   =  SymTabEntry(KEYWORD,0,&doDNEQ) ;

   symtab["AND"]  =  SymTabEntry(KEYWORD,0,&doAND) ;
   symtab["OR"]   =  SymTabEntry(KEYWORD,0,&doOR) ;
   symtab["NOT"]  =  SymTabEntry(KEYWORD,0,&doNOT) ;

   symtab["IFTHEN"]= SymTabEntry(KEYWORD,0,&doIFTHEN) ;
   symtab["ELSE"] =  SymTabEntry(KEYWORD,0,&doELSE);
   symtab["ENDIF"]=  SymTabEntry(KEYWORD,0,&doENDIF);


}


// This function should be called when tkBuffer is empty.
// It adds tokens to tkBuffer.
//
// This function returns when an empty line was entered
// or if the end-of-file has been reached.
//
// This function returns false when the end-of-file was encountered.
//
// Processing done by fillBuffer()
//   - detects and ignores comments.
//   - detects string literals and combines as 1 token
//   - detetcs base 10 numbers
//
//
bool Sally::fillBuffer() {
   string line ;     // single line of input
   int pos ;         // current position in the line
   int len ;         // # of char in current token
   long int n ;      // int value of token
   char *endPtr ;    // used with strtol()


   while(true) {    // keep reading until empty line read or eof

      // get one line from standard in
      //
      getline(istrm, line) ;

      // if "normal" empty line encountered, return to mainLoop
      //
      if ( line.empty() && !istrm.eof() ) {
         return true ;
      }

      // if eof encountered, return to mainLoop, but say no more
      // input available
      //
      if ( istrm.eof() )  {
         return false ;
      }


      // Process line read

      pos = 0 ;                      // start from the beginning

      // skip over initial spaces & tabs
      //
      while( line[pos] != '\0' && (line[pos] == ' ' || line[pos] == '\t') ) {
         pos++ ;
      }

      // Keep going until end of line
      //
      while (line[pos] != '\0') {

         // is it a comment?? skip rest of line.
         //
         if (line[pos] == '/' && line[pos+1] == '/') break ;

         // is it a string literal?
         //
         if (line[pos] == '.' && line[pos+1] == '"') {

            pos += 2 ;  // skip over the ."
            len = 0 ;   // track length of literal

            // look for matching quote or end of line
            //
            while(line[pos+len] != '\0' && line[pos+len] != '"') {
               len++ ;
            }

            // make new string with characters from
            // line[pos] to line[pos+len-1]
            string literal(line,pos,len) ;  // copy from pos for len chars

            // Add to token list
            //
            tkBuffer.push_back( Token(STRING,0,literal) ) ;

            // Different update if end reached or " found
            //
            if (line[pos+len] == '\0') {
               pos = pos + len ;
            } else {
               pos = pos + len + 1 ;
            }

         } else {  // otherwise "normal" token

            len = 0 ;  // track length of token

            // line[pos] should be an non-white space character
            // look for end of line or space or tab
            //
            while(line[pos+len] != '\0' && line[pos+len] != ' ' && line[pos+len] != '\t') {
               len++ ;
            }

            string literal(line,pos,len) ;   // copy form pos for len chars
            pos = pos + len ;

            // Try to convert to a number
            //
            n = strtol(literal.c_str(), &endPtr, 10) ;

            if (*endPtr == '\0') {
               tkBuffer.push_back( Token(INTEGER,n,literal) ) ;
            } else {
               tkBuffer.push_back( Token(UNKNOWN,0,literal) ) ;
            }
         }

         // skip over trailing spaces & tabs
         //
         while( line[pos] != '\0' && (line[pos] == ' ' || line[pos] == '\t') ) {
            pos++ ;
         }

      }
   }
}



// Return next token from tkBuffer.
// Call fillBuffer() if needed.
// Checks for end-of-file and throws exception
//
Token Sally::nextToken() {
      Token tk ;
      bool more = true ;

      while(more && tkBuffer.empty() ) {
         more = fillBuffer() ;
      }

      if ( !more && tkBuffer.empty() ) {
         throw EOProgram("End of Program") ;
      }

      tk = tkBuffer.front() ;
      tkBuffer.pop_front() ;
      return tk ;
}


// The main interpreter loop of the Sally Forth interpreter.
// It gets a token and either push the token onto the parameter
// stack or looks for it in the symbol table.
//
//
void Sally::mainLoop() {

   Token tk ;
   map<string,SymTabEntry>::iterator it ;
   int count = 0;

   try {
      while( 1 ) {
        count++;
        //cout << "I am round " << count << "." << endl;

        if (doSkip.top() == true)
        {
            //cout << "I am here now!" << endl;
            if (tk.m_text == lookingforKeyWord.top() || tk.m_text == "ENDIF") //this can be ENDIF if skipping ELSE, ELSE if skipping IF.
            {
                doSkip.top() = false;
                it = symtab.find(tk.m_text);
                it->second.m_dothis(this);
                //<Call the function at the map keyword>
            }
            else {
                tk = nextToken();
            }
        }
      else {
         tk = nextToken() ;

         if (tk.m_kind == INTEGER || tk.m_kind == STRING) {

            // if INTEGER or STRING just push onto stack
            params.push(tk) ;

         } else {
            it = symtab.find(tk.m_text) ;

            if ( it == symtab.end() )  {   // not in symtab

               params.push(tk) ;

            } else if (it->second.m_kind == KEYWORD)  {

               // invoke the function for this operation
               //
               it->second.m_dothis(this) ;

            } else if (it->second.m_kind == VARIABLE) {

               // variables are pushed as tokens
               //
               tk.m_kind = VARIABLE ;
               params.push(tk) ;

            } else {

               // default action
               //
               params.push(tk) ;

            }
         }
      }
      //doDUMP(this);
    }

   } catch (EOProgram& e) {

      cerr << "End of Program\n" ;
      if ( params.size() == 0 ) {
         cerr << "Parameter stack empty.\n" ;
      } else {
         cerr << "Parameter stack has " << params.size() << " token(s).\n" ;
      }

   } catch (out_of_range& e) {

      cerr << "Parameter stack underflow??\n" ;

   } catch (...) {

      cerr << "Dylan, you made a biiiig error. Go back to work.\n" ;

   }
}

// -------------------------------------------------------


void Sally::doPlus(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for +.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value + p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


void Sally::doMinus(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for -.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value - p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


void Sally::doTimes(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for *.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value * p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


void Sally::doDivide(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for /.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value / p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


void Sally::doMod(Sally *Sptr) {
   Token p1, p2 ;

   if ( Sptr->params.size() < 2 ) {
      throw out_of_range("Need two parameters for %.") ;
   }
   p1 = Sptr->params.top() ;
   Sptr->params.pop() ;
   p2 = Sptr->params.top() ;
   Sptr->params.pop() ;
   int answer = p2.m_value % p1.m_value ;
   Sptr->params.push( Token(INTEGER, answer, "") ) ;
}


void Sally::doNEG(Sally *Sptr) {
   Token p ;

   if ( Sptr->params.size() < 1 ) {
      throw out_of_range("Need one parameter for NEG.") ;
   }
   p = Sptr->params.top() ;
   Sptr->params.pop() ;
   Sptr->params.push( Token(INTEGER, -p.m_value, "") ) ;
}


void Sally::doDot(Sally *Sptr) {

   Token p ;
   if ( Sptr->params.size() < 1 ) {
      throw out_of_range("Need one parameter for .") ;
   }

   p = Sptr->params.top() ;
   Sptr->params.pop() ;

   if (p.m_kind == INTEGER) {
      cout << p.m_value ;
   } else {
      cout << p.m_text ;
   }
}


void Sally::doSP(Sally *Sptr) {
   cout << " " ;
}


void Sally::doCR(Sally *Sptr) {
   cout << endl ;
}

void Sally::doDUMP(Sally *Sptr) {
   // do whatever for debugging
   Token toke;
   stack<Token> gerard = Sptr->params;

   cout << "PARAM STACK: ";
   if (gerard.empty())
   {
       cout << "|(VOID)|" ;
   }
   while (!(gerard.empty()))
   {
       cout << "|";
       toke = gerard.top();
       gerard.pop();
       cout << toke.m_text << "," << toke.m_value;
   }
   cout << endl;

}

void Sally::doDUMP_ITSTACK (Sally *Sptr) {

    stack<int> dupCurrTruth = Sptr->currtruthStack;
    stack<string> dupLookFor = Sptr->lookingforKeyWord;
    stack<bool> dupDoSkip = Sptr->doSkip;

    cout << "CURR TRUTH: ";
    while (!(dupCurrTruth.empty()))
    {
        int dummy = dupCurrTruth.top();
        dupCurrTruth.pop();
        cout << dummy << ", ";
    }
    cout << endl;

        cout << "LOOKING FOR ";
    while (!(dupLookFor.empty()))
    {
        string dummy = dupLookFor.top();
        dupLookFor.pop();
        cout << dummy << ", ";
    }
    cout << endl;

        cout << "DO SKIP? ";
    while (!(dupDoSkip.empty()))
    {
        int dummy = dupDoSkip.top();
        dupDoSkip.pop();
        cout << dummy << ", ";
    }
    cout << endl;
}
void Sally::doDUP (Sally *Sptr) {

    Token parm;
    if (Sptr->params.size() < 1)
    {
        throw out_of_range("ERR: Need one parameter for DUP.");
    }
    parm = Sptr->params.top() ;
    Sptr->params.push( Token(INTEGER, parm.m_value, "") ) ;
}

void Sally::doDROP (Sally *Sptr) {

    if (Sptr->params.size() < 1)
    {
        throw out_of_range("ERR: You're a sad person and will die alone. \nAlso, need one parameter for DROP.");
    }
    Sptr->params.pop();
}

void Sally::doSWAP (Sally *Sptr) {

    if (Sptr->params.size() < 2)
        throw out_of_range("ERR: Need two parameters for SWAP.");

    Token oldTop, belowTop;
    oldTop = Sptr->params.top();
    Sptr->params.pop();
    belowTop = Sptr->params.top();
    Sptr->params.pop();

    Sptr->params.push(Token(INTEGER, oldTop.m_value, ""));
    Sptr->params.push(Token(INTEGER, belowTop.m_value, ""));

}

void Sally::doROT (Sally *Sptr) {

    if (Sptr->params.size() < 3)
        throw out_of_range("ERR: Need three parameters for ROT.");

    Token oldTop, belowTop, lastfromTop;

    oldTop = Sptr->params.top();
    Sptr->params.pop();
    belowTop = Sptr->params.top();
    Sptr->params.pop();
    lastfromTop = Sptr->params.top();
    Sptr->params.pop();

    Sptr->params.push(Token(INTEGER, belowTop.m_value, ""));
    Sptr->params.push(Token(INTEGER, oldTop.m_value, ""));
    Sptr->params.push(Token(INTEGER, lastfromTop.m_value, ""));
}

void Sally::doSET (Sally *Sptr)
{
    Token varsity, aint;
    map<string,SymTabEntry>::iterator itr ;

    if (Sptr->params.size() < 2)
        throw out_of_range("ERR: Need two parameters for SET.");

    varsity = Sptr->params.top();
    Sptr->params.pop();
    aint = Sptr->params.top();
    Sptr->params.pop();

    itr = Sptr->symtab.find(varsity.m_text);

    if (itr == Sptr->symtab.end())
    {
        Sptr->symtab[varsity.m_text] = SymTabEntry(VARIABLE,aint.m_value, NULL) ; //NULL, i think?
    }
    else {
        throw("ERR: This codeword is already in use!");
    }
    //i think that's it? i dunno...
}

void Sally::doCALL (Sally *Sptr) {

    Token varb;
    map<string,SymTabEntry>::iterator itr ;
    if (Sptr->params.size() < 1)
    {
        throw out_of_range("ERR: Need one parameter for CALL.");
    }

    varb = Sptr->params.top();
    Sptr->params.pop();

    itr = Sptr->symtab.find(varb.m_text) ;
    if ( itr == Sptr->symtab.end() )
    {
        throw("ERR: This variable was not found!");
    }
    varb.m_value = Sptr->symtab[varb.m_text].m_value;

    Sptr->params.push(Token(INTEGER, varb.m_value, ""));
}

void Sally::doREPL (Sally *Sptr) {

    Token var, aint;
    map<string,SymTabEntry>::iterator itr ;
    if (Sptr->params.size() < 2)
    {
        throw out_of_range("ERR: Need two parameters for REPL.");
    }

    var = Sptr->params.top();
    Sptr->params.pop();
    aint = Sptr->params.top();
    Sptr->params.pop();

    itr = Sptr->symtab.find(var.m_text) ;
    if ( itr == Sptr->symtab.end() )
    {
        throw("ERR: This variable was not found!");
    }

    Sptr->symtab[var.m_text].m_value = aint.m_value;

}


void Sally::doLESSDEN(Sally *Sptr) {

    Token firstElem, secondElem;
    if (Sptr->params.size() < 2)
        throw out_of_range("ERR: Comparisons require two parameters.");

    secondElem = Sptr->params.top();
    Sptr->params.pop();
    firstElem = Sptr->params.top();
    Sptr->params.pop();

    if (firstElem.m_value < secondElem.m_value){
        Sptr->params.push(Token(INTEGER, 1, "")); //true
    }
    else{
        Sptr->params.push(Token(INTEGER, 0, "")); //false
    }

}

void Sally::doGRDEN(Sally *Sptr) {

    Token firstElem, secondElem;
    if (Sptr->params.size() < 2)
        throw out_of_range("ERR: Comparisons require two parameters.");

    secondElem = Sptr->params.top();
    Sptr->params.pop();
    firstElem = Sptr->params.top();
    Sptr->params.pop();

    if (firstElem.m_value > secondElem.m_value){
        Sptr->params.push(Token(INTEGER, 1, "")); //true
    }
    else{
        Sptr->params.push(Token(INTEGER, 0, "")); //false
    }

}

void Sally::doLESSDEN_OR_EQ(Sally *Sptr) {

    Token firstElem, secondElem;
    if (Sptr->params.size() < 2)
        throw out_of_range("ERR: Comparisons require two parameters.");

    secondElem = Sptr->params.top();
    Sptr->params.pop();
    firstElem = Sptr->params.top();
    Sptr->params.pop();

    if (firstElem.m_value <= secondElem.m_value){
        Sptr->params.push(Token(INTEGER, 1, "")); //true
    }
    else{
        Sptr->params.push(Token(INTEGER, 0, "")); //false
    }

}

void Sally::doGRDEN_OR_EQ(Sally *Sptr) {

    Token firstElem, secondElem;
    if (Sptr->params.size() < 2)
        throw out_of_range("ERR: Comparisons require two parameters.");

    secondElem = Sptr->params.top();
    Sptr->params.pop();
    firstElem = Sptr->params.top();
    Sptr->params.pop();

    if (firstElem.m_value >= secondElem.m_value){
        Sptr->params.push(Token(INTEGER, 1, "")); //true
    }
    else{
        Sptr->params.push(Token(INTEGER, 0, "")); //false
    }

}

void Sally::doEQ(Sally *Sptr) {

    Token firstElem, secondElem;
    if (Sptr->params.size() < 2)
        throw out_of_range("ERR: Comparisons require two parameters.");

    secondElem = Sptr->params.top();
    Sptr->params.pop();
    firstElem = Sptr->params.top();
    Sptr->params.pop();

    if (firstElem.m_value == secondElem.m_value){
        Sptr->params.push(Token(INTEGER, 1, "")); //true
    }
    else{
        Sptr->params.push(Token(INTEGER, 0, "")); //false
    }

}

void Sally::doDNEQ(Sally *Sptr) {

    Token firstElem, secondElem;
    if (Sptr->params.size() < 2)
        throw out_of_range("ERR: Comparisons require two parameters.");

    secondElem = Sptr->params.top();
    Sptr->params.pop();
    firstElem = Sptr->params.top();
    Sptr->params.pop();

    if (firstElem.m_value != secondElem.m_value){
        Sptr->params.push(Token(INTEGER, 1, "")); //true
    }
    else{
        Sptr->params.push(Token(INTEGER, 0, "")); //false
    }

}

void Sally::doAND(Sally *Sptr) {

    Token firstElem, secondElem;
    if (Sptr->params.size() < 2)
        throw out_of_range("ERR: Comparisons require two parameters.");

    secondElem = Sptr->params.top();
    Sptr->params.pop();
    firstElem = Sptr->params.top();
    Sptr->params.pop();

    if (firstElem.m_value != 0 && secondElem.m_value != 0) //if both are true
    {
        Sptr->params.push(Token(INTEGER, 1, "")); //true
    }
    else {
        Sptr->params.push(Token(INTEGER, 0, "")); //false
    }
}

void Sally::doOR(Sally *Sptr) {

    Token firstElem, secondElem;
    if (Sptr->params.size() < 2)
        throw out_of_range("ERR: Comparisons require two parameters.");

    secondElem = Sptr->params.top();
    Sptr->params.pop();
    firstElem = Sptr->params.top();
    Sptr->params.pop();

    if (firstElem.m_value != 0 || secondElem.m_value != 0) //if either are true
    {
        Sptr->params.push(Token(INTEGER, 1, "")); //true
    }
    else {
        Sptr->params.push(Token(INTEGER, 0, "")); //false
    }
}

void Sally::doNOT(Sally *Sptr) {

    Token elem;
    if (Sptr->params.size() < 1)
        throw out_of_range("ERR: Nothing on stack to negate!");

    elem = Sptr->params.top();
    Sptr->params.pop();

    if (elem.m_value != 0) //is true
    {
        Sptr->params.push(Token(INTEGER, 0, "")); //false
    }
    else { //is false
        Sptr->params.push(Token(INTEGER, 1, "")); //true
    }
}

void Sally::doIFTHEN(Sally *Sptr) {

    Token truth;
    if (Sptr->params.size() < 1)
        throw out_of_range("ERR: There is No Truth to be found!");

    truth = Sptr->params.top();
    Sptr->params.pop();

    //idea: implement a stack of integers that holds 1 if statement was true,
    //0 if it was false. Peek every time you hit an if or else, and pop
    //once you hit ENDIF.

    if (truth.m_value != 0) //if the given statement was true, so do IFTHEN
    {
        Sptr->doSkip.push(false);
        Sptr->lookingforKeyWord.push("ELSE"); //just to prevent confusion.
        Sptr->currtruthStack.push(1);
    }
    else { //means truth is = 0, so is false; skip to ELSE
        Sptr->doSkip.push(true);
        Sptr->lookingforKeyWord.push("ELSE");
        Sptr->currtruthStack.push(0);
    }

    //How to solve the skip problem?
    //idea: have a bool called "doSkip", == false at start.
    //flip on whenever you hit a block opposite the truth value.
    //for all iterations of mainLoop, check doSkip first
    /*
     * if (doSkip == true)
     *   {
     *      if (thisToken == lookingforKeyWord) //this can be ENDIF if skipping ELSE, ELSE if skipping IF.
     *      {
     *          doSkip = false;
     *          <Call the function at the map keyword>
     *      }
     *      else {
     *          currToken.next();
     *      }
     *   }
     * else {
     *  ...etc
     * }
     */
    //this could throw a lot of errors if done wrong though...

}

void Sally::doELSE(Sally *Sptr) {

    int truth;
    //Sptr->doDUMP(Sptr);
    //Sptr->doDUMP_ITSTACK(Sptr);
    if(Sptr->currtruthStack.size() < 1 || Sptr->doSkip.size() < 2 || Sptr->lookingforKeyWord.size() < 2)
    {
        cerr << "Broke at doELSE." << endl;
        throw out_of_range("ERR: How the heck did you get this far with No Truths?");
    }

    truth = Sptr->currtruthStack.top();

    if (truth == 0) //initial statement was false
    {
        Sptr->doSkip.pop();
        Sptr->doSkip.push(false);
        Sptr->lookingforKeyWord.pop();
        Sptr->lookingforKeyWord.push("ENDIF");
    }
    else { //initial statement was true
        Sptr->doSkip.pop();
        Sptr->doSkip.push(true);
        Sptr->lookingforKeyWord.pop();
        Sptr->lookingforKeyWord.push("ENDIF");
    }

}

void Sally::doENDIF(Sally *Sptr) {

    //Sptr->doDUMP(Sptr);
    //Sptr->doDUMP_ITSTACK(Sptr);
    if(Sptr->currtruthStack.size() < 1 || Sptr->doSkip.size() < 2 || Sptr->lookingforKeyWord.size() < 2)
        {
            /*if (Sptr->currtruthStack.size() < 1)
                cerr << "The truth stack broke. ";
            if (Sptr->doSkip.size() < 2)
                cerr << "The skip stack broke. ";
            if (Sptr->lookingforKeyWord.size() < 2)
                cerr << "The keyword stack broke. ";*/
            cerr << "Broke at doENDIF" << endl;
            throw out_of_range("ERR: How the heck did you get this far with No Truths?");
        }

    Sptr->doSkip.pop();
    Sptr->currtruthStack.pop();
    Sptr->lookingforKeyWord.pop();
    //cout << "Got to the end!" << endl;

        //Assures a hard reset
        if(Sptr->currtruthStack.size() < 1 || Sptr->doSkip.size() < 2 || Sptr->lookingforKeyWord.size() < 2)
        {
            while (!(Sptr->currtruthStack.empty()))
            {
                Sptr->currtruthStack.pop();
            }
            while (!(Sptr->doSkip.empty()))
            {
                Sptr->doSkip.pop();
            }
            Sptr->doSkip.push(false);
            while (!(Sptr->lookingforKeyWord.empty()))
            {
                Sptr->lookingforKeyWord.pop();
            }
            Sptr->lookingforKeyWord.push("~");

            Sptr->nextToken();
        }
}
