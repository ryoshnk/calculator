//  Copyright 2016 Ryo Shinoki. Released under the MIT license.

#include "calculator.h"

const char *FORMULA      = "1000+5-1+5+1";
const char *FILE_NAME    = "formula.txt";
const int  MAX_CODE_SIZE = 100;

int main() {
  write_text(FORMULA, FILE_NAME);
  FILE *file = get_file_ptr(FILE_NAME);
  
  CurrentToken *currentToken = (CurrentToken*)malloc(sizeof(CurrentToken));
  currentToken->type = UNDIFINED;
  
  //Get Tokens from file stream.
  Node *ast= read_expression(file, currentToken);
  if (currentToken->type == UNEXPECTED_TOKEN)
    return -1;
  
  //Calculate like an interpreter
  printf("[Interpreter Result]\n %d\n\n", interpret_expression(ast));
  
  //Calculate like a compiler
  int codeIndex=0;
  Code codes[MAX_CODE_SIZE];
  compile_expression(&codeIndex, ast, codes);
  codes[codeIndex++].command = PRINT;
  generate_c(&codeIndex, codes);
  generate_assembler(&codeIndex, codes);
  
  return 0;
}

/*---------------------------------------------------------------------------------------
 If calculate  "5-1+4", 
   first loop create below Node
 
     -
    5 1
 
   second loop create below Node
 
      +
     -  4
    5 1
 
  if regard "5" as leftNode, "-" as topNode, "1" as rightNode in the first loop,
  "5-1" is regarded as leftNode in the next loop.
---------------------------------------------------------------------------------------*/
Node *read_expression(FILE *fp, CurrentToken *token) {
  Node *leftNode, *topNode;
  proceed_token(fp, token);
  leftNode = get_operand_node(token);
  proceed_token(fp, token);
  while (token->type == OPERATOR_PLUS || token->type == OPERATOR_MINUS) {
    topNode = (Node*)malloc(sizeof(struct _Node));
    topNode->nodeType = token->type;
    topNode->leftNode = leftNode;
    proceed_token(fp, token);
    topNode->rightNode = get_operand_node(token);
    //Proceed *token for next loop.
    proceed_token(fp, token);
    //Current topNode will be a left child Node if *token is operator.
    leftNode = topNode;
  }
  return leftNode;
}

void proceed_token(FILE *fp, CurrentToken *token) {
  if (token->type == OPERATOR_MINUS || token->type == OPERATOR_PLUS) {
    read_operand(fp, token);
  } else if (token->type == OPERAND) {
    read_operator(fp, token);
  } else if (token->type == UNDIFINED) {
    read_signed_operand(fp, token);
  }
  return;
}

Node *get_operand_node(CurrentToken *currentToken) {
  Node *node;
  if (currentToken->type == OPERAND) {
    node = (Node *)malloc(sizeof(Node));
    node->nodeType    = OPERAND;
    node->nodeOperand = currentToken->number;
  } else {
    fprintf(stderr, "Bad expression: Number expected\n");
  }
  return node;
}

void read_operand(FILE *fp, CurrentToken *token) {
  while (1) {
    int asciiChar = getc(fp);
    if (!isspace(asciiChar)) {
      if (isdigit(asciiChar)) {
        int integerChar = 0;
        do {
          integerChar = integerChar * 10 + asciiChar - '0';
          asciiChar = getc(fp);
        } while (isdigit(asciiChar));
        token->number = integerChar;
        token->type   = OPERAND;
        ungetc(asciiChar, fp);                   /* push back a token */
        break;                                  /* break for while */
      } else {
        token->type = UNEXPECTED_TOKEN;
        fprintf(stderr, "Unexpectred charcter '%c' \n", asciiChar);
        break;                                  /* break for while */
      }
    }
  }
}

void read_signed_operand(FILE *fp, CurrentToken *token) {
  while (1) {
    int asciiChar = getc(fp);
    if (!isspace(asciiChar)) {
      if (asciiChar=='-') {
        read_operand(fp, token);
        token->number *= -1;
      } else if (isdigit(asciiChar)) {
        ungetc(asciiChar, fp);
        read_operand(fp, token);
      }
      break;                                  /* break for while */
    }
  }
}

void read_operator(FILE *fp, CurrentToken *token) {
  while (1) {
    int asciiChar = getc(fp);
    if (!isspace(asciiChar)) {
      switch (asciiChar) {
        case '+':
          token->type = OPERATOR_PLUS;
          break;
        case '-':
          token->type = OPERATOR_MINUS;
          break;
        case EOF:
          token->type = TEXT_EOF;
          break;
        default:
          token->type = UNEXPECTED_TOKEN;
          fprintf(stderr, "Unexpectred charcter '%c' \n", asciiChar);
          break;
      }
      break;                                    /* break for while */
    }
  }
}

int interpret_expression(Node *tree) {
  switch(tree->nodeType) {
    case OPERAND:
      return tree->nodeOperand;
    case OPERATOR_PLUS:
      return interpret_expression(tree->leftNode)+interpret_expression(tree->rightNode);
    case OPERATOR_MINUS:
      return interpret_expression(tree->leftNode)-interpret_expression(tree->rightNode);
    default:
      fprintf(stderr, "interpret_expression: bad expression\n");
      return -1;
  }
}


void compile_expression(int *nCode, Node *tree, Code *codes)
{
  switch(tree->nodeType) {
    case OPERAND:
      codes[*nCode].command = PUSH;
      codes[*nCode].operand = tree->nodeOperand;
      break;
    case OPERATOR_PLUS:
      compile_expression(nCode, tree->leftNode, codes);
      compile_expression(nCode, tree->rightNode, codes);
      codes[*nCode].command = ADD;
      break;
    case OPERATOR_MINUS:
      compile_expression(nCode, tree->leftNode, codes);
      compile_expression(nCode, tree->rightNode, codes);
      codes[*nCode].command = SUB;
      break;
  }
  ++ *nCode;
}


void generate_c(int *nCode, Code *codes) {
  printf("[C Code]\nint stack[100]; \nmain() { \n int sp = 0; \n");
  for (int i = 0; i < *nCode; i++) {
    switch(codes[i].command) {
      case PUSH:
        printf(" stack[sp++] = %d;\n", codes[i].operand);
        break;
      case ADD:
        printf(" sp--;\n stack[sp-1] += stack[sp];\n");
        break;
      case SUB:
        printf(" sp--;\n stack[sp-1] -= stack[sp];\n");
        break;
      case PRINT:
        printf(" printf(\"%%d\\n\", stack[--sp]);\n");
        break;
    }
  }
  printf("}\n\n");
}


void generate_assembler(int *nCode, Code *codes) {
  printf("[Assembler Code] \n");
  for (int i = 0; i < *nCode; i++) {
    switch(codes[i].command) {
      case PUSH:
        printf(" PUSH %d;\n", codes[i].operand);
        break;
      case ADD:
        printf(" ADD \n");
        break;
      case SUB:
        printf(" SUB \n");
        break;
      case PRINT:
        break;
    }
  }
}
