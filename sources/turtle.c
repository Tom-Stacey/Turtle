
#include "../includes/turtle.h"

#define TEST_STR_LEN 100

int main(int argc, char *argv[])

{
    srand(time(NULL));

    checkInput(argc, argv, NO_TESTING);

    int startMode = getStartMode(argc, argv);
    
    switch (startMode) {
        case NO_TESTING :
            runFullProgram(argv[1]);
            break;
        case TEST_WHITEBOX :
            runWhiteBoxTesting();
            break;
        case TEST_BLACKBOX :
            runBlackBoxTesting();
            break;
        case TEST_SYSTEM :
            runSystemTesting();
            break;
        case TEST_ALL :
            runWhiteBoxTesting();
            runBlackBoxTesting();
            runSystemTesting();
            break;
        default :
            fprintf(stderr, "ERROR - invalid output from getStartMode()\n");
    }
    return 0;
}


//// SETUP/SHUTDOWN FUNCTIONS ////////////////////////////////////////////////
/*..........................................................................*/

  // set up everything and either parse or interpret the file depending on user input
void runFullProgram(char *filePath)
{
    printf("Enter 0 to parse, 1 to interpret: \n");
    int interpretFile = 0;
    
    if(scanf("%d", &interpretFile) != 1) {
        fprintf(stderr, "Incorrect value entered, program has exited\n");
        exit(1);
    }
    
    int processedOK = 0;
    
    if(interpretFile) {
        processedOK = interpret(filePath, NO_TESTING);
        holdScreenUntilUserInput(); // if interpretting, SDL is initialised so hold the window open after completion
    } else {   
        processedOK = parse(filePath, NO_TESTING);
    }
    
    shutDownParsing();
    
    if(processedOK) {
        printf("Success\n");
    } else {
        printf("Failure\n");
    }
}

// checks for valid input from argc and argv. If not run in testing mode, exits program on invalid input
int checkInput(int argc, char *argv[], int testMode)
{
    // check for correct number of arguments (should be between 1 and 3)
    if(argc == 1 || argc > 3) {
        if(testMode == NO_TESTING) {
            exitWithCommandLineError();
        }
        return 0;
    }
    
    // if 2 arguments, check for file located at filepath of argv[1]
    if(argc == 2) {
        FILE *fp = fopen(argv[1], "r");
        if(fp == NULL) {
            if(testMode == NO_TESTING) {
                fprintf(stderr, "ERROR: Unable to locate file at '%s'\n", argv[1]);
                exitWithCommandLineError();
                return 0;
            }
        }
        fclose(fp);
    }
    
    // if 3 arguments, check for valid testing input
    if(argc == 3) {
        if(strcmp(argv[1],"test") != 0) {
            if(testMode == NO_TESTING) {
                exitWithCommandLineError();
            }
            return 0;
        }
        if(strcmp(argv[2],"all") != 0 && strcmp(argv[2],"white") != 0 && strcmp(argv[2],"black") != 0 && strcmp(argv[2],"sys") != 0) {
            if(testMode == NO_TESTING) {
                exitWithCommandLineError();
            }
            return 0;
        }
    }
    return 1;
}


void exitWithCommandLineError()
{
    fprintf(stderr,"please run the turtle program with one of the command line arguments as follows:\n\nTo parse a .txt file and draw a shape:\n./turtle <FILENAME>.txt\n\nFor testing enter one of the below:\n./turtle test all\n./turtle test white\n./turtle test black\n./turtle test sys\n");
    exit(1);

}

// processes the user input and returns the type of mode to start the program in
int getStartMode(int argc, char *argv[])
{
    // if two arguments, set up the parse heper using filepath from argv[1]
    if(argc == 2) {
        return NO_TESTING;
    }
    
    // if three arguments, choose testing type based on argv[2]
    if(argc == 3) {
        if(strcmp(argv[2],"white") == 0) {
            return TEST_WHITEBOX;
        }
        if(strcmp(argv[2],"black") == 0) {
            return TEST_BLACKBOX;
        }
        if(strcmp(argv[2],"sys") == 0) {
            return TEST_SYSTEM;
        }
        if(strcmp(argv[2],"all") == 0) {
            return TEST_ALL;
        }
    }
    
    fprintf(stderr, "ERROR - incorrect input allowed into getStartMode()\n");
    exit(1);
    
}
    

//// WHITE BOX TESTING FUNCTIONS /////////////////////////////////////////////
/*..........................................................................*/


void runWhiteBoxTesting()
{
    runCommandLineTests();
    runParserWhiteBoxTests();
    runInterpreterWhiteBoxTests();
}

void runBlackBoxTesting()
{
    runParserBlackBoxTests();
    runInterpreterBlackBoxTests();
}

void runSystemTesting()
{
    sput_start_testing();
	  sput_set_output_stream(NULL);
	  
    sput_enter_suite("systemTesting(): Checking full system implementation");
    sput_run_test(systemTesting);
    sput_leave_suite();
    
	  sput_finish_testing();
}

void runCommandLineTests()
{  
	  sput_start_testing();
	  sput_set_output_stream(NULL);	
    
	  sput_enter_suite("testCommandLine(): Checking command line input is processed properly");
    sput_run_test(testCommandLine);
    sput_leave_suite();
    
	  sput_finish_testing();
}

void testCommandLine()
{
    int argc = 1;
    char *argv[3];
    for(int i = 0; i < 3; i++) {
        argv[i] = calloc(TEST_STR_LEN, sizeof(char));
        if(argv[i] == NULL) {
            fprintf(stderr, "ERROR: Unable to calloc space for strings in testCommandLine()\n");
        }
    }
    
    sput_fail_unless(checkInput(argc, argv, TESTING) == 0, "Input check detects error when only one argument passed");
    
    argc = 4;
    sput_fail_unless(checkInput(argc, argv, TESTING) == 0, "Input check detects error when more than 3 arguments passed");
    
    argc = 3;
    strcpy(argv[1], "testwrong");
    sput_fail_unless(checkInput(argc, argv, TESTING) == 0, "Input check detects error when 2nd of 3 arguments is not 'test' passed");
    
    strcpy(argv[1], "test");
    strcpy(argv[2], "all");
    sput_fail_unless(checkInput(argc, argv, TESTING) == 1, "Input check OK when 3 arguments and third is 'all'");
    strcpy(argv[2], "white");
    sput_fail_unless(checkInput(argc, argv, TESTING) == 1, "Input check OK when 3 arguments and third is 'white'");
    strcpy(argv[2], "black");
    sput_fail_unless(checkInput(argc, argv, TESTING) == 1, "Input check OK when 3 arguments and third is 'black'");
    strcpy(argv[2], "testwrong");
    sput_fail_unless(checkInput(argc, argv, TESTING) == 0, "Input check OK when 3 arguments and third is neither 'all', 'black' or 'white'");
    
    for(int i = 0; i < 3; i++) {
        free(argv[i]);
    }

}


void runParserBlackBoxTests()
{
	  sput_start_testing();
	  
    sput_enter_suite("testSyntaxErrors(): Checking test scripts with missing brackets etc");
    sput_run_test(testSyntaxErrors);
    sput_leave_suite();
    
    sput_enter_suite("testVarNum(): Checking test scripts with variables and numbers");
    sput_run_test(testVarNum);
    sput_leave_suite();
    
    sput_enter_suite("testSetCommand(): Checking test scripts using the SET command");
    sput_run_test(testSetCommand);
    sput_leave_suite();
    
    sput_enter_suite("testDOloops(): Testing syntax parsing of DO loops");
    sput_run_test(testDOloops);
    sput_leave_suite();
    
    sput_enter_suite("testBackstepCommand(): Checking test scripts using the BKSTP command");
    sput_run_test(testBackstepCommand);
    sput_leave_suite();
    
    sput_enter_suite("testWhileCommand(): Checking test scripts using the WHILE command");
    sput_run_test(testWhileCommand);
    sput_leave_suite();
    
    sput_enter_suite("testPenSwitchCommand(): Checking test scripts using the PN command");
    sput_run_test(testPenSwitchCommand);
    sput_leave_suite();
    
    sput_enter_suite("testColourCommand(): Checking test scripts using the CLR command");
    sput_run_test(testColourCommand);
    sput_leave_suite();
    
	  sput_finish_testing();
}


void testSyntaxErrors()
{
    sput_fail_unless(parse("testingFiles/test_simpleParse.txt", TESTING) == 1, "Parsed simple RT, LT and FD commands ok");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/test_noClosingBrace.txt", TESTING) == 0, "Will not parse text with no closing brace");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/test_noOpeningBrace.txt", TESTING) == 0, "Will not parse text with no opening brace");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/test_textAfterClosingBrace.txt", TESTING) == 0, "Will not parse text when there is remaining text after last brace");
    shutDownParsing();
}

void testVarNum()
{
    sput_fail_unless(parse("testingFiles/VarNum_Testing/test_simpleVarNum.txt", TESTING) == 1, "Parsed simple access of set variable OK");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/VarNum_Testing/test_uninitialisedVarNum.txt", TESTING) == 0, "Will not parse text when variable is used uninitialised");
    shutDownParsing();
}

void testSetCommand()
{
    sput_fail_unless(parse("testingFiles/SET_Testing/test_simpleSET.txt", TESTING) == 1, "Parsed simple SET commands ok");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/SET_Testing/test_SETmultiple.txt", TESTING) == 1, "Parsed multiple SET commands to same variable ok");
    shutDownParsing();

    sput_fail_unless(parse("testingFiles/SET_Testing/test_SETpolish.txt", TESTING) == 1, "Parsed SET commands with reverse polish maths ok");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/SET_Testing/test_SETpolishUndefined.txt", TESTING) == 0, "Will not parse command performing reverse polish on undefined variable");
    
    sput_fail_unless(parse("testingFiles/SET_Testing/test_SETlongPolish.txt", TESTING) == 1, "Parsed SET commands using long Polish expression ok");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/SET_Testing/test_SETpolishTooManyVariables.txt", TESTING) == 0, "Will not parse SET command with unbalanced (too many variables) reverse polish equation");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/SET_Testing/test_SETpolishTooManyOperators.txt", TESTING) == 0, "Will not parse SET command with unbalanced (too many operators) reverse polish equation");
    shutDownParsing();
}
   
   
void testDOloops()
{
    sput_fail_unless(parse("testingFiles/DO_Testing/test_simpleDO.txt", TESTING) == 1, "Parsed simple DO loop ok");
    shutDownParsing();

    sput_fail_unless(parse("testingFiles/DO_Testing/test_DOwithoutOpeningBrace.txt", TESTING) == 0, "Will not parse DO without opening brace");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/DO_Testing/test_DOwithoutClosingBrace.txt", TESTING) == 0, "Will not parse DO without closing brace");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/DO_Testing/test_noFROM_DO.txt", TESTING) == 0, "Will not parse DO without FROM keyword");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/DO_Testing/test_noTO_DO.txt", TESTING) == 0, "Will not parse DO without TO keyword");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/DO_Testing/test_nestedDO.txt", TESTING) == 1, "Parsed nested DO loop ok");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/DO_Testing/test_polishDO.txt", TESTING) == 1, "Parsed DO loop containing reverse polish ok");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/DO_Testing/test_complexDO.txt", TESTING) == 1, "Parsed complex nested DO loop containing SET & reverse polish ok");
    shutDownParsing();
    
    sput_fail_unless(interpret("testingFiles/DO_Testing/test_simpleDOvalues.txt", TESTING) == 1 && (int) getVariableVal('B') == 20, "Interpreted simple DO loop with correct value at end");
    shutDownParsing();
    
    sput_fail_unless(interpret("testingFiles/DO_Testing/test_nestedDOvalues.txt", TESTING) == 1 && (int) getVariableVal('D') == 31, "Interpreted nested DO loop with correct value at end");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/DO_Testing/test_descendingDO.txt", TESTING) == 1, "Parsed descending DO loop ok");
    shutDownParsing();
    
    sput_fail_unless(interpret("testingFiles/DO_Testing/test_descendingDO.txt", TESTING) == 1, "Interpreted descending DO loop ok");
    shutDownParsing();
    
}

void testBackstepCommand()
{
    sput_fail_unless(parse("testingFiles/BKSTP_Testing/test_simpleBKSTP.txt", TESTING) == 1, "Parsed simple BKSTP command ok");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/BKSTP_Testing/test_overflowBKSTP.txt", TESTING) == 1, "Parsed BKSTP ok when there are more BKSTP commands than actions");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/BKSTP_Testing/test_skipSET.txt", TESTING) == 1, "Parsed BKSTP when separated from action by SET commands ok");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/BKSTP_Testing/test_multipleForked.txt", TESTING) == 1, "Parsed multiple forked BKSTP commands ok");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/BKSTP_Testing/test_unassignedVarBKSTP.txt", TESTING) == 0, "Will not parse BKSTP when used with unassigned variable");
    shutDownParsing();
}

void testWhileCommand()
{
    sput_fail_unless(parse("testingFiles/WHILE_Testing/test_simpleWHILE.txt", TESTING) == 1, "Parsed simple WHILE loop ok");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/WHILE_Testing/test_nestedWHILE.txt", TESTING) == 1, "Parsed nested WHILE loop ok");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/WHILE_Testing/test_unassignedWHILE.txt", TESTING) == 0, "Will not parse WHILE loop set up with unassigned variable");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/WHILE_Testing/test_noBraceWHILE.txt", TESTING) == 0, "Will not parse WHILE loop with no opening brace");
    shutDownParsing();
}

void testPenSwitchCommand()
{
    sput_fail_unless(parse("testingFiles/PN_Testing/test_simplePEN.txt", TESTING) == 1, "Parsed pen switch ok");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/PN_Testing/test_multiplePEN.txt", TESTING) == 1, "Parsed multiple pen switches ok");
    shutDownParsing();
}

void testColourCommand()
{
    sput_fail_unless(parse("testingFiles/CLR_Testing/test_setColour.txt", TESTING) == 1, "Parsed setting draw colour ok");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/CLR_Testing/test_randomColour.txt", TESTING) == 1, "Parsed setting random draw colour ok");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/CLR_Testing/test_advanceColour.txt", TESTING) == 1, "Parsed advancing draw colour ok");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/CLR_Testing/test_incorrectColour.txt", TESTING) == 0, "Will not parse if CLR followed by incorrect symbol");
    shutDownParsing();
}

void systemTesting()
{
    sput_fail_unless(interpret("testingFiles/system_Testing/test_simple_move.txt", TEST_SYSTEM) == 1, "Interpreted simple file ok");
    sput_fail_unless(getTurtleX() == SCREEN_WIDTH/2, "Turtle in correct x position after simple moves");
    sput_fail_unless(getTurtleY() == SCREEN_HEIGHT/2, "Turtle in correct y position after simple moves");
    sput_fail_unless(getTurtleAngle() == 45, "Turtle at correct angle after simple moves");
    shutDownParsing();
    
    sput_fail_unless(interpret("testingFiles/system_Testing/test_while.txt", TEST_SYSTEM) == 1, "interpret while loop file ok");
    sput_fail_unless(getTurtleX() == SCREEN_WIDTH/2, "Turtle in correct x position after while loop moves");
    sput_fail_unless(getTurtleY() == SCREEN_HEIGHT/2, "Turtle in correct y position after while loop moves");
    sput_fail_unless(getTurtleAngle() == 90, "Turtle at correct angle after while loop moves");
    shutDownParsing();
    
    sput_fail_unless(interpret("testingFiles/system_Testing/test_polish.txt", TEST_SYSTEM) == 1, "Interpreted reverse polish file ok");
    sput_fail_unless(getTurtleX() == SCREEN_WIDTH/2, "Turtle in correct x position after long polish expression determining moves");
    sput_fail_unless(getTurtleY() == SCREEN_HEIGHT/2, "Turtle in correct y position after long polish expression determining moves");
    sput_fail_unless(getTurtleAngle() == 270, "Turtle at correct angle after long polish expression determining moves");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/system_Testing/test_neverTrueWHILE.txt", TEST_SYSTEM) == 1, "parsed never true while loop ok");
    shutDownParsing();
    sput_fail_unless(interpret("testingFiles/system_Testing/test_neverTrueWHILE.txt", TEST_SYSTEM) == 1, "interpreted never true while loop ok");
    sput_fail_unless(getTurtleX() == SCREEN_WIDTH/2, "Turtle in correct x position after interpretting never true while loop");
    sput_fail_unless(getTurtleY() == (SCREEN_HEIGHT/2) - 45, "Turtle in correct y position after interpretting never true while loop");
    sput_fail_unless(getTurtleAngle() == 320, "Turtle at correct angle after after interpretting never true while loop");
    shutDownParsing();
    
    sput_fail_unless(parse("testingFiles/system_Testing/test_neverTrueWHILEsyntax.txt", TEST_SYSTEM) == 0, "Will not parse file with syntax error within never true while loop");
    shutDownParsing();
    sput_fail_unless(interpret("testingFiles/system_Testing/test_neverTrueWHILEsyntax.txt", TEST_SYSTEM) == 1, "Interprets file with syntax error within never true while loop ok");
    shutDownParsing();
    
    
}




