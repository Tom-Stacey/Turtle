#include "interpreter.h"

#define TOKEN_LENGTH 100 // maximum token length

#define TEST_WITH_SYNTAX_ERRORS 0 //set to 1 to display syntax errors during testing

typedef struct parseHandler *ParseHandler;

// SETUP/SHUTDOWN FUNCTIONS
void         setUpForParsing(char *filePath, int testMode, int interpretMode);
void         createParseHandler();
ParseHandler getParseHandlerPointer(ParseHandler newHandler);
void         initialiseParseHandler(char *filePath, int testMode);
void         shutDownParsing();

// PARSE HANDLER FUNCTIONS
void resizeTokenArray(ParseHandler pH);
void freeParseHandler();

// PARSING FUNCTIONS
int       parse(char * filePath, int testMode);
int       interpret(char *filePath, int testMode);
int       getToken(ParseHandler pH);
TokenType whatToken(char *token);
int       sameString(char *a, char *b);
int       checkForEndOfCode(ParseHandler pH);
int       syntaxError(ParseHandler pH, char *message);

// RECURSIVE DESCENT FUNCTIONS
int  processMain(ParseHandler pH);
int  processInstructionList(ParseHandler pH);
int  processInstruction(ParseHandler pH);
int  processAction(ParseHandler pH);
int  processSet(ParseHandler pH);
int  processPolish(ParseHandler pH);
int  processOperator(ParseHandler pH);
int  finishPolish(ParseHandler pH);
int  processDo(ParseHandler pH);
int  executeUpwardsDoLoop(ParseHandler pH, int doLoopStartIndex, char loopVariable, int loopVal, int loopTargetVal);
int  executeDownwardsDoLoop(ParseHandler pH, int doLoopStartIndex, char loopVariable, int loopVal, int loopTargetVal);
int  processWhile(ParseHandler pH);
int  executeWhileLoop(ParseHandler pH, TokenType loopType, char loopVariable, double loopTargetVal, int loopStartIndex);
int  skipLoop(ParseHandler pH);
int  processColour(ParseHandler pH);

// TOKEN CHECKING FUNCTIONS
int    checkForVarNum(char * token);
int    checkForAnyVar(char * token);
int    checkForInstruction(char *chkToken);
int    checkForValidOperator(char *c, ParseHandler pH);
int    checkForColour(char *token, ParseHandler pH);
double getCurrentTokenVal(ParseHandler pH);

// TESTING FUNCTIONS
// WHITE
void runParserWhiteBoxTests();
void testHandlerInitialisation();
void testSetAssignment();

// BLACK
void runInterpreterBlackBoxTests();
void testDoMaths();
void testVariableHandling();
void testNumberChecking();












