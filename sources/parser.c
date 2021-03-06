#include "../includes/parser.h"

// structure to hold all the necessary information while parsing
struct parseHandler {

  FILE *tokenFP;
  
  int hangingBraces; // record the number of open braces in order to check for more input after processing
  
  int numberOfTokens; // the total number of tokens scanned from the file
  int currentTokenIndex; // the index in the token array of the current token that is being processed
  
  char *token;
  char **tokenArray; // an array containing all tokens that have been scanned so far. Used in DO & WHILE loops
  
  double val; // the current value that is being processed
  Clr colour; // the current colour that is being processed
  
  mathSymbol currentOperation; // the current mathematical operation
  
  int showSyntaxErrors; // a flag for whether or not to display syntax errors in the terminal. Used for testing
  int interpret; // a flag for whether to interpret the file or just parse the text
} ;




//  SETUP/SHUTDOWN FUNCTIONS  ////////////////////////////////////////////////////////////////
/*..........................................................................................*/

// sets up all parsing structures and then calls setUpInterpreting() to continue initialising in interpreter.c
void setUpForParsing(char *filePath, int testMode, int interpretMode)
{
    createParseHandler();
    initialiseParseHandler(filePath, testMode);
    
    ParseHandler pH = getParseHandlerPointer(NULL);
    pH->interpret = interpretMode;
    setUpForInterpreting(testMode, pH->interpret);
    
}

void createParseHandler()
{
    ParseHandler pH = (ParseHandler) malloc(sizeof(struct parseHandler));
    if(pH == NULL) {
        fprintf(stderr, "ERROR: Unable to malloc space for parseHandler structure in createParseHandler()");
        exit(1);
    }
    getParseHandlerPointer(pH);
}

// if passed NULL, returns pointer to the ParseHandler. If passed pointer, sets static pointer to the new pointer
ParseHandler getParseHandlerPointer(ParseHandler newHandler)
{
    static ParseHandler pH;
    
    if(newHandler != NULL) {
        pH = newHandler;
    }
    return pH;
}

// sets all starting values for parse handler and opens file to read from. If not testing, sets the showSyntaxErrors flag to true. If testing, the syntax error flag is determined by a #define in parser.h
void initialiseParseHandler(char *filePath, int testMode)
{
    ParseHandler pH = getParseHandlerPointer(NULL);
    
    pH->tokenFP = fopen(filePath, "r");
    
    pH->hangingBraces = 0;
    pH->currentTokenIndex = 0;
    pH->numberOfTokens = 0;
    
    if(testMode == NO_TESTING) {
        pH->showSyntaxErrors = 1;
    } else {
        pH->showSyntaxErrors = TEST_WITH_SYNTAX_ERRORS;
    }
    
    pH->tokenArray = NULL;
}

void shutDownParsing()
{
    freeValStack();
    freeParseHandler();
    shutDownInterpreting();
}




//  PARSE HANDLER FUNCTIONS  /////////////////////////////////////////////////////////////////
/*..........................................................................................*/


// realocs the token array to fit in the next token
void resizeTokenArray(ParseHandler pH)
{
    pH->tokenArray = (char**)realloc(pH->tokenArray,pH->numberOfTokens * sizeof(char*));
    if(pH->tokenArray == NULL) {
        fprintf(stderr, "ERROR - realloc failed in getToken()\n");
        exit(1);
    }
    pH->tokenArray[pH->numberOfTokens-1] = (char*)calloc(TOKEN_LENGTH, sizeof(char));
    if(pH->tokenArray[pH->numberOfTokens-1] == NULL) {
        fprintf(stderr, "ERROR - malloc failed in getToken()\n");
        exit(1);
    }
}

void freeParseHandler()
{
    ParseHandler pH = getParseHandlerPointer(NULL);
    fclose(pH->tokenFP);
    
    for(int i = 0; i < pH->numberOfTokens; i++) {
        free(pH->tokenArray[i]);
    }
    free(pH->tokenArray);
    free(pH);
}



//  PARSING FUNCTIONS  ///////////////////////////////////////////////////////////////////////
/*..........................................................................................*/


int parse(char * filePath, int testMode)
{
    setUpForParsing(filePath, testMode, DONT_INTERPRET);
    
    ParseHandler pH = getParseHandlerPointer(NULL);
    int parsed = processMain(pH);
    return parsed;
}

int interpret(char *filePath, int testMode)
{
    setUpForParsing(filePath, testMode, INTERPRET);
    
    ParseHandler pH = getParseHandlerPointer(NULL);
    int interpreted = processMain(pH);
    return interpreted;
}

// reads the next token and sets it as the ParseHandler's current token
int getToken(ParseHandler pH)
{
      // if not at the end of the token array, advance token index along the array
    if(pH->currentTokenIndex < pH->numberOfTokens-1) {
        pH->currentTokenIndex++;
        pH->token = pH->tokenArray[pH->currentTokenIndex];
        return 1;
    }
    
      // if at the end of the array, increase the number of tokens processed & set the token index to be at the end of the array
    pH->numberOfTokens++;
    pH->currentTokenIndex = pH->numberOfTokens-1;
    resizeTokenArray(pH);
    
      // scan in next token from file and insert it into the array
    if(fscanf(pH->tokenFP, "%s", pH->tokenArray[pH->numberOfTokens-1]) != 1) {
        if(fgetc(pH->tokenFP) == EOF) {
              // if reached the end of file then there are not enough closing braces
            return syntaxError(pH, "closing braces do not match opening braces. Are you missing a closing brace?");
        } else {
            return syntaxError(pH, "invalid character in file");
        }
    } else {
          // set the current token pointer to the end of the array
        pH->token = pH->tokenArray[pH->numberOfTokens-1];
        return 1;
    }
}

        
// analyses the token string and returns an enum for the type of token.
TokenType whatToken(char *token)
{
    ParseHandler pH = getParseHandlerPointer(NULL);
    
    if(sameString(token, "FD"))    {return fd;}
    if(sameString(token, "LT"))    {return lt;}
    if(sameString(token, "RT"))    {return rt;}
    if(sameString(token, "SET"))   {return set;}
    if(sameString(token, "DO"))    {return doToken;}
    if(sameString(token, "BKSTP")) {return bkStep;}
    if(sameString(token, "PN"))    {return penChange;}
    if(sameString(token, "CLR"))   {return colour;}
    if(sameString(token, "FROM"))  {return from;}
    if(sameString(token, "TO"))    {return to;}
    if(sameString(token, "{"))     {return openBrace;}
    if(sameString(token, "}"))     {return closeBrace;}
    if(sameString(token, ";"))     {return semicolon;}
    if(sameString(token, ":="))    {return equals;}
    if(sameString(token, "WHILE")) {return whileToken;}
    if(sameString(token, "<"))     {return lessThan;}
    if(sameString(token, ">"))     {return moreThan;}
    if(sameString(token, "RAND"))  {return randomColour;}
    if(sameString(token, "ADV"))   {return advanceColour;}
    
    if(checkForValidOperator(token, pH) == 1) {return op;}
    
      // if a single character, check for variable. If it is a variable, check whether it has been assigned
    if(strlen(token) == 1) {
        if(checkValidVariable(token[0])) {
            if(checkVariableAssigned(token[0], pH->interpret, &pH->val)) {
                return assignedVar;
            } else {
                return unassignedVar;
            }
        }
    }
    if(checkForNumber(token, &pH->val)) {return num;}
    
    else {return noToken;}
}

// returns 1 if two strings are identical, ese returns 0
int sameString(char *a, char *b)
{
    if(strcmp(a,b) == 0) {
        return 1;
    } else {
        return 0;
    }
}
        
// used at the end of parsing. Returns 0 if there is anything other than spaces or newlines before the end of the file 
int checkForEndOfCode(ParseHandler pH)
{
    char c;
    while((c = fgetc(pH->tokenFP) ) != EOF) {
        if(c != ' ' && c != '\n') {
            return syntaxError(pH, "additional input detected. Are you missing an opening brace?");
        }
    }
    return 1;
}    
        

int syntaxError(ParseHandler pH, char *message)
{
    if(pH->showSyntaxErrors) {
        fprintf(stderr, "Syntax error - %s\nError at: %s\n", message, pH->token);
    }
    return 0;
}        



//  RECURSIVE DESCENT FUNCTIONS  /////////////////////////////////////////////////////////////
/*..........................................................................................*/

// "{" <INSTRCTLIST>
int processMain(ParseHandler pH)
{
    if(!getToken(pH)) {return 0;}
    
      // "{"
    if(whatToken(pH->token) != openBrace ) {
        return syntaxError(pH, "all code sections should begin with an opening brace");
    }
    pH->hangingBraces++;
    
      // <INSTRCTLIST>
    if(processInstructionList(pH) ) {
        if(pH->hangingBraces == 0) {
            return checkForEndOfCode(pH);
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}
     

/* <INSTRCTLIST>: */
// <INSTRUCTION> <INSTRCTLIST> | "}"
int processInstructionList(ParseHandler pH)
{
    if(!getToken(pH)) {return 0;}
    
      // <INSTRUCTON>
    if(checkForInstruction(pH->token) ){
        if(!processInstruction(pH)) {
            return 0;
        }
          // <INSTRCTLIST>
        return processInstructionList(pH);
    }
    
      // "}"
    if(whatToken(pH->token) == closeBrace) {
        pH->hangingBraces--;
        return 1;
    }
        
    return syntaxError(pH, "Invalid instruction");
}

/* <INSTRUCTION>: */
// <FD> | <LT> | <RT> | <BKSTP> | <SET> | <DO> | <WHILE> | <CLR> | <PN>
int processInstruction(ParseHandler pH)
{
    
    TokenType t = whatToken(pH->token);
    
    switch(t) {
          // <FD> | <LT> | <RT> | <BKSTP>
        case fd :
        case lt :
        case rt :
        case bkStep :
            if(!processAction(pH)) {
                return 0;
            }
            if(pH->interpret) {
                doAction(t, pH->val);
            }
            return 1;
          // <SET>
        case set :
            return processSet(pH);
          // <DO>
        case doToken :
            return processDo(pH);
          // <WHILE>
        case whileToken :
            return processWhile(pH);
          // <CLR>
        case colour :
            return processColour(pH);
          // <PN>
        case penChange :
            if(pH->interpret) {
                switchPenStatus();
             }
            return 1;
        default :
            fprintf(stderr,"ERROR - invalid token (%s) passed to processInstruction()\n", pH->token);
            exit(1);
    }
}
            
// "FD" | "LT" | "RT" | "BKSTP" (already asserted in processInstruction() )
// <VARNUM>
int processAction(ParseHandler pH)
{
    if(!getToken(pH)) {return 0;}
      // <VARNUM>
    if(checkForVarNum(pH->token)) {
        return 1;
    }
    if(whatToken(pH->token) == unassignedVar) {
        return syntaxError(pH, "attempted to process an unassigned variable");
    }
    return 0;
    
}

/* <SET>: */
// "SET" <VAR> ":=" <POLISH> ("SET" already asserted in processInstruction() )
int processSet(ParseHandler pH)
{
    if(!getToken(pH)) {return 0;}
      // <VAR>
    if(!checkForAnyVar(pH->token)) {
        return 0;
    }
    char varToSet = pH->token[0];
    
    if(!getToken(pH)) {return 0;}
      // ":="
    if(whatToken(pH->token) != equals) {
        return 0;
    }
      // <POLISH>
    if(!processPolish(pH)) {
        return 0;
    }
    assignValToVariable(varToSet, pH->val, pH->interpret);
    return 1;
    
}


/* <POLISH>: */

// <VARNUM> <POLISH> |
// <OP> <POLISH> |
// ";"
int processPolish(ParseHandler pH)
{
    if(!getToken(pH)) {return 0;}
    
      // <VARNUM>
    if(checkForVarNum(pH->token)) {
        pushToValStack(pH->val);
           // <POLISH>
        return processPolish(pH);
    }
    
    if(strlen(pH->token) != 1) {
        return syntaxError(pH, "all reverse polish operators/variables should only be 1 character long separated by spaces");
    }
    
      // <OP>
    if(whatToken(pH->token) == op) {
        if(!processOperator(pH) ) {
            return 0;
        }
          // <POLISH>
        return processPolish(pH);
    }
    
      // ";"
    if(whatToken(pH->token) == semicolon) {
        return finishPolish(pH);
    }
    
    return syntaxError(pH, "reverse polish expression not completed properly");
}


/* <OP>: */

// "+" | "-" | "*" | "/"
int processOperator(ParseHandler pH)
{
    double a, b;
    if(popFromValStack(&b) == 0 || popFromValStack(&a) == 0) {
        return syntaxError(pH, "too few variables/constants for operators in reverse polish expression");
    }
    
    pH->val = doMaths(a, b, pH->currentOperation);
    
    pushToValStack(pH->val);
    return 1;
}

// assigns final value from reverse polish expression to current val in ParseHandler and checks there are no values left over in expression
int finishPolish(ParseHandler pH)
{
    popFromValStack(&pH->val);
    if(getNumberOfValsOnStack() != 0) {
        return syntaxError(pH, "more input than required operators in reverse polish expression\n");
    } else {
        return 1;
    }
}


/* <DO>: */

// "DO" <VAR> "FROM" <VARNUM> "TO" <VARNUM> "{" <INSTRCTLST> ("DO" already asserted in processInstruction() )
int processDo(ParseHandler pH)
{
      // <VAR>
    if(!getToken(pH)) {return 0;}
    if(!checkForAnyVar(pH->token)) {
        return syntaxError(pH, "invalid variable following DO command");
    }
    char loopVariable = pH->token[0];
    
      // "FROM"
    if(!getToken(pH)) {return 0;}
    if(whatToken(pH->token) != from) {
        return syntaxError(pH, "missing FROM in DO command");
    }
    
      // <VARNUM>
    if(!getToken(pH)) {return 0;}
    if(!checkForVarNum(pH->token)) {
        return syntaxError(pH, "invalid variable/number following FROM in DO command");
    }
    int loopVal = (int) getCurrentTokenVal(pH);
    assignValToVariable(loopVariable, (double)loopVal, pH->interpret);
    
      // "TO"
    if(!getToken(pH)) {return 0;}
    if(whatToken(pH->token) != to) {
        return syntaxError(pH, "missing TO in DO command");
    }
    
      // <VARNUM>
    if(!getToken(pH)) {return 0;}
    if(!checkForVarNum(pH->token)) {
        return syntaxError(pH, "invalid variable/number following TO in DO command");
    }
    int loopTargetVal = (int) getCurrentTokenVal(pH);
    
      // "{" (hanging braces is incremented within the loop)
    if(!getToken(pH)) {return 0;}
    if(whatToken(pH->token) != openBrace) {
        return syntaxError(pH, "missing opening brace in DO loop initialisation");
    }
    
    // record token index to restart loop
    int doLoopStartIndex = pH->currentTokenIndex;
    
      // <INSTRCTLST>
    if(loopVal <= loopTargetVal) {
        return executeUpwardsDoLoop(pH, doLoopStartIndex, loopVariable, loopVal, loopTargetVal);
    } else {
        return executeDownwardsDoLoop(pH, doLoopStartIndex, loopVariable, loopVal, loopTargetVal);
    }
    

}


int executeUpwardsDoLoop(ParseHandler pH, int doLoopStartIndex, char loopVariable, int loopVal, int loopTargetVal)
{
    for(int i = loopVal; i <= loopTargetVal; i++) {
          // if interpreting, assign incremented value to loop variable
        assignValToVariable(loopVariable, (double)i, pH->interpret);
        
          // reset token location and increase number of hanging braces
        pH->currentTokenIndex = doLoopStartIndex;
        pH->hangingBraces++;
        
          // carry out the embedded instruction list
        if(!processInstructionList(pH)) {
            return syntaxError(pH, "error within DO loop");
        }
    }
    return 1;
}

int executeDownwardsDoLoop(ParseHandler pH, int doLoopStartIndex, char loopVariable, int loopVal, int loopTargetVal)
{
    for(int i = loopVal; i >= loopTargetVal; i--) {
          // if interpreting, assign decremented value to loop variable
        assignValToVariable(loopVariable, (double)i, pH->interpret);
        
          // reset token location and increase number of hanging braces
        pH->currentTokenIndex = doLoopStartIndex;
        pH->hangingBraces++;
        
          // carry out the embedded instruction list
        if(!processInstructionList(pH)) {
            return syntaxError(pH, "error within DO loop");
        }
    }
    return 1;
}

/* <WHILE>: */

// "WHILE" <ASSIGNEDVAR> <COMPARATOR> <VARNUM> "{" <INTRCTLST> ("WHILE" already asserted in processInstruction() )
int processWhile(ParseHandler pH)
{
      // <ASSIGNEDVAR>
    if(!getToken(pH)) {return 0;}
    if(whatToken(pH->token) != assignedVar) {
        return syntaxError(pH, "unassigned variable in WHILE command declaration");
    }
    char loopVariable = pH->token[0];
    
      // <COMPARATOR>
    if(!getToken(pH)) {return 0;}
    TokenType loopType = whatToken(pH->token);
    if(loopType != lessThan && loopType != moreThan) {
        return syntaxError(pH, "missing comparator in WHILE command");
    }
    
      // <VARNUM>
    if(!getToken(pH)) {return 0;}
    if(!checkForVarNum(pH->token)) {
        return syntaxError(pH, "invalid variable/number following operator in WHILE command");
    }
    double loopTargetVal = getCurrentTokenVal(pH);
    
      // "{" (hanging braces increased within loop or during loop bypass)
    if(!getToken(pH)) {return 0;}
    if(whatToken(pH->token) != openBrace) {
        return syntaxError(pH, "missing opening brace in WHILE loop initialisation");
    }
    
    // record token index to restart loop
    int loopStartIndex = pH->currentTokenIndex;
    
      // <INSTRCTLST>
    if(!pH->interpret) {
        return skipLoop(pH);
    } else {
        return executeWhileLoop(pH, loopType, loopVariable, loopTargetVal, loopStartIndex);
    }
}

// processes while loop until while condition is met. If condition is met on instigation, parses loop once but doesn't set any values
int executeWhileLoop(ParseHandler pH, TokenType loopType, char loopVariable, double loopTargetVal, int loopStartIndex)
{
    if(loopType == lessThan) {
          //check if condition is already met
        if(getVariableVal(loopVariable) > loopTargetVal) {
            return skipLoop(pH);
        }
          // if not, do the loop
        while(getVariableVal(loopVariable) < loopTargetVal) {
            pH->currentTokenIndex = loopStartIndex;
            pH->hangingBraces++;
            
            if(!processInstructionList(pH)) {
                return syntaxError(pH, "error within WHILE loop");
            }
        }
      
    } else {
          //check if condition is already met
        if(getVariableVal(loopVariable) < loopTargetVal) {
            return skipLoop(pH);
        }
          // if not, do the loop
        while(getVariableVal(loopVariable) > loopTargetVal) {
            pH->currentTokenIndex = loopStartIndex;
            pH->hangingBraces++;
            
            if(!processInstructionList(pH)) {
                return syntaxError(pH, "error within WHILE loop");
            }
        }
    }
    return 1;
}

// parses a loop but doesn't alter any values from SET commands within the loop. Used when not interpretting or when interpreter comes accross a loop it should never enter
int skipLoop(ParseHandler pH)
{
    // switch off interpretting, but record interpretting status to return later
    int returnToInterpretting;
    if(pH->interpret) {
        returnToInterpretting = 1;
        pH->interpret = 0;
    } else {
        returnToInterpretting = 0;
    }
    
    pH->hangingBraces++;
    
    int successfulParse = processInstructionList(pH);
    
    pH->interpret = returnToInterpretting;
    
    // if program is interpretting, warn user of error but don't exit
    if(!successfulParse) {
      if(pH->interpret) {
          if(pH->showSyntaxErrors) {
              printf("Warning: bypassed syntax error within while loop\n");
          }
          return 1;
      } else{
          return syntaxError(pH, "error within WHILE loop");
      }
    }
    return successfulParse;
}
    
    
/* <CLR>: */

// "CLR" "ADV" | ("CLR" already asserted in processInstruction()"
// "CLR" "RAND" |
// "CLR" colour
int processColour(ParseHandler pH)
{
    if(!getToken(pH)) {return 0;}
    
      // "ADV"
    if(whatToken(pH->token) == advanceColour) {
        if(pH->interpret) {
            advanceTurtleColour();
        }
        return 1;
    }
    
      // "RAND"
    if(whatToken(pH->token) == randomColour) {
        if(pH->interpret) {
            setRandomTurtleColour();
        }
        return 1;
    }
    
      // colour
    if(checkForColour(pH->token, pH) ) {
        if(pH->interpret) {
            applyTurtleColour(pH->colour);
        }
        return 1;
    }
    
    return syntaxError(pH, "invalid token following CLR");
}




//  TOKEN CHECKING FUNCTIONS  ////////////////////////////////////////////////////////////////
/*..........................................................................................*/

// returns 1 if passed token is an assigned variable or a number
int checkForVarNum(char * token)
{
    if(whatToken(token) == assignedVar || whatToken(token) == num) {
        return 1;
    }
    return 0;
}

// returns 1 if the passed token is a variable, regardless of whether it has already been assigned
int checkForAnyVar(char * token)
{
    if(whatToken(token) == assignedVar || whatToken(token) == unassignedVar) {
        return 1;
    }
    return 0;
}

// returns 1 if the passed token is any one of the instruction commands
int checkForInstruction(char *chkToken)
{
    TokenType t = whatToken(chkToken);
    
    switch(t) {
        case fd :
        case rt :
        case lt :
        case set :
        case doToken :
        case whileToken :
        case bkStep :
        case penChange :
        case colour :
            return 1;
        default :
            return 0;
    }
}

// returns 1 if the passed token is a mathematical operator
int checkForValidOperator(char *c, ParseHandler pH)
{
      // check token has only one character - prevents "-5", for example, from being read as a minus operator
    if(strlen(c) > 1) {
        return 0;
    }
    switch(c[0]) {
        case '+' :
            pH->currentOperation = add;
            return 1;
        case '-' :
            pH->currentOperation = subtract;
            return 1;
        case '/' :
            pH->currentOperation = divide;
            return 1;
        case '*' :
            pH->currentOperation = multiply;
            return 1;
        default :
            return 0;
    }
}

// returns 1 if the passed token is any one of the valid colours and sets the ParseHandler's current colour
int checkForColour(char *token, ParseHandler pH)
{
    if(sameString(token, "WHTE")) {
        pH->colour = white;
        return 1;
    }
    if(sameString(token, "RED")) {
        pH->colour = red;
        return 1;
    }
    if(sameString(token, "BLUE")) {
        pH->colour = blue;
        return 1;
    }
    if(sameString(token, "GREEN")) {
        pH->colour = green;
        return 1;
    }
    if(sameString(token, "YLLW")) {
        pH->colour = yellow;
        return 1;
    }
    if(sameString(token, "PRPL")) {
        pH->colour = purple;
        return 1;
    }
    return 0;
}

// if token is a number, returns number. Else returns the number stored as that variable
double getCurrentTokenVal(ParseHandler pH)
{
    if(checkForNumber(pH->token, &pH->val) ) {
        return pH->val;
    }
    return getVariableVal(pH->token[0]);
}


//  WHITE BOX TESTING FUNCTIONS  /////////////////////////////////////////////////////////////
/*..........................................................................................*/

void runParserWhiteBoxTests()
{
	  sput_start_testing();
	  
	  sput_set_output_stream(NULL);
	  
	  sput_enter_suite("testHandlerInitialisation(): Checking Parse Handler structure is properly initialised");
    sput_run_test(testHandlerInitialisation);
    sput_leave_suite();
    
    sput_enter_suite("testSetAssignment(): Checking test scripts using the SET command");
    sput_run_test(testSetAssignment);
    sput_leave_suite();
    
    sput_finish_testing();

}


void testHandlerInitialisation()
{
    createParseHandler();
    initialiseParseHandler("testingFiles/parserTesting.txt", TESTING);
    ParseHandler pH = getParseHandlerPointer(NULL);
    
    sput_fail_unless(pH->tokenFP != NULL, "Parser Handler sets token file pointer on initialisation");
    freeParseHandler();
}

void testSetAssignment()
{
    sput_fail_unless(interpret("testingFiles/SET_Testing/test_selfSET.txt", TESTING) == 1 && getVariableVal('C') == 10, "Interpreted variable setting itself with correct value (e.g. C += C ;)");
    shutDownParsing();
}



//  BLACK BOX TESTING FUNCTIONS  /////////////////////////////////////////////////////////////
/*..........................................................................................*/


void runInterpreterBlackBoxTests()
{
    sput_start_testing();
	  
	  sput_set_output_stream(NULL);
	  
	  sput_enter_suite("testDoMaths(): testing the doing of the maths");
    sput_run_test(testDoMaths);
    sput_leave_suite();
    
	  sput_enter_suite("testVariableHandling(): testing valid/invalid variables, variable assignment and returning variable contents");
    sput_run_test(testVariableHandling);
    sput_leave_suite();
    
	  sput_enter_suite("testNumberChecking(): testing the checkForNumber() function");
    sput_run_test(testNumberChecking);
    sput_leave_suite();
    
    sput_finish_testing();

}

void testDoMaths()
{
    sput_fail_unless(doMaths(1, 4, add) == 5, "Adding correctly");
    sput_fail_unless(doMaths(25, -13, subtract) == 38, "Subtracting correctly");
    sput_fail_unless(doMaths(12, 4, divide) == 3, "Dividing correctly");
    sput_fail_unless(doMaths(20, 17, multiply) == 340, "Multiplying correctly");
}

void testVariableHandling()
{
    setUpForInterpreting(TESTING, INTERPRET);
    sput_fail_unless(checkValidVariable('X') == 1, "Confirms variable X exists");
    sput_fail_unless(checkValidVariable('a') == 0, "Confirms variable a doesn't exist");
    sput_fail_unless(checkValidVariable('%') == 0, "Can handle percent symbol");
    sput_fail_unless(checkValidVariable('+') == 0, "Can handle mathematical operators");
    
    double checkVal = 0;
    sput_fail_unless(checkVariableAssigned('X', INTERPRET, &checkVal) == 0 && checkVal == 0, "Variable X is initially unassigned when interpreting");
    assignValToVariable('X', 34, INTERPRET);
    sput_fail_unless(checkVariableAssigned('X', INTERPRET, &checkVal) == 1 && checkVal == 34, "Variable X is assigned and set ok when interpreting");
    
    sput_fail_unless(checkVariableAssigned('Q', INTERPRET, &checkVal) == 0, "Variable Q is initially unassigned when not interpreting");
    checkVal = 0;
    assignValToVariable('Q', 15, DONT_INTERPRET);
    sput_fail_unless(checkVariableAssigned('Q', DONT_INTERPRET, &checkVal) == 1 && checkVal == 0, "Variable Q is assigned ok and value is not set when not interpreting");
    
    shutDownInterpreting();
}

void testNumberChecking()
{
    setUpForInterpreting(TESTING, INTERPRET);
    
    double checkVal = 12;
    sput_fail_unless(checkForNumber("X", &checkVal) == 0 && checkVal == 12, "Confirms X is not a number and doesn't alter contents of pointer passed to function");
    sput_fail_unless(checkForNumber("The West Midlands", &checkVal) == 0 && checkVal == 12, "Confirms The West Midlands is not a number and doesn't alter contents of pointer passed to function");
    sput_fail_unless(checkForNumber("25", &checkVal) == 1 && checkVal == 25, "Confirms 25 is a number and sets pointer contents correctly");
    sput_fail_unless(checkForNumber("-25", &checkVal) == 1 && checkVal == -25, "Confirms -25 is a number and sets pointer contents correctly");
    sput_fail_unless(checkForNumber("38547.1698", &checkVal) == 1 && checkVal == 38547.1698, "Confirms 38547.1698 is a number and sets pointer contents correctly");
    
    shutDownInterpreting();
}





// independent main function - used in testing
// command line compile code: gcc `sdl2-config --cflags` -O4 -Wall -pedantic -std=c99 -o parseTest parser.c interpreter.c display.c -lm `sdl2-config --libs`
/*
int main(void)
{
    runParserWhiteBoxTests();
    
    runInterpreterBlackBoxTests();
}

*/


















