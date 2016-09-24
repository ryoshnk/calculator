//  Copyright 2016 Ryo Shinoki. Released under the MIT license.

#ifndef calculator_h
#define calculator_h

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

//CommandType means assembler command + print in this code.
enum CommandType{
  PUSH,
  ADD,
  SUB,
  PRINT,
};

enum NodeType {
  UNDIFINED,
  OPERAND,
  OPERATOR_PLUS,
  OPERATOR_MINUS,
  TEXT_EOF,
  UNEXPECTED_TOKEN,
};

typedef struct _Node {
  int nodeType;
  int nodeOperand;
  struct _Node *leftNode;
  struct _Node *rightNode;
} Node;

typedef struct _Code {
  enum CommandType command;
  int operand;
} Code;

typedef struct _CurrentToken{
  int  number;
  enum NodeType  type;
} CurrentToken;

//calculator.c
void proceed_token(FILE *fp, CurrentToken *token);
void read_operand (FILE *fp, CurrentToken *token);
void read_signed_operand(FILE *fp, CurrentToken *token);
void read_operator(FILE *fp, CurrentToken *token);
Node *read_expression(FILE *fp, CurrentToken *token);
Node *get_operand_node(CurrentToken *currentToken);
int  interpret_expression(Node *tree);
void compile_expression(int *nCode, Node *tree, Code *codes);
void generate_c(int *nCode, Code *codes);
void generate_assembler(int *nCode, Code *codes);

//textController.c
int write_text(const char *formula, const char *name);
FILE* get_file_ptr(const char *fileName);

#endif
