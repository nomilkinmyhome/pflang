#include "../include/ast.h"

static AstNode* create_node(NodeType type) {
    AstNode* node = (AstNode*)malloc(sizeof(AstNode));
    if (node == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for AST node\n");
        exit(1);
    }
    node->type = type;
    return node;
}

AstNode* create_function_node(char* name, AstNode** parameters, int param_count, 
                             AstNode* body, DataType* return_types, int return_type_count) {
    AstNode* node = create_node(NODE_FUNCTION);
    node->value.function.name = name;
    node->value.function.parameters = parameters;
    node->value.function.param_count = param_count;
    node->value.function.body = body;
    node->value.function.return_types = return_types;
    node->value.function.return_type_count = return_type_count;
    return node;
}

AstNode* create_variable_node(char* name, AstNode* init_value) {
    AstNode* node = create_node(NODE_VARIABLE);
    node->value.variable.name = name;
    node->value.variable.init_value = init_value;
    return node;
}

AstNode* create_binary_op_node(AstNode* left, AstNode* right, TokenType operator) {
    AstNode* node = create_node(NODE_BINARY_OP);
    node->value.binary_op.left = left;
    node->value.binary_op.right = right;
    node->value.binary_op.operator = operator;
    return node;
}

AstNode* create_return_node(AstNode* return_value) {
    AstNode* node = create_node(NODE_RETURN);
    node->value.return_stmt.return_value = return_value;
    return node;
}

AstNode* create_tuple_node(AstNode** values, int value_count) {
    AstNode* node = create_node(NODE_TUPLE);
    node->value.tuple.values = values;
    node->value.tuple.value_count = value_count;
    return node;
}

AstNode* create_literal_node(char* value, DataType type) {
    AstNode* node = create_node(NODE_LITERAL);
    node->value.literal.value = value;
    node->value.literal.type = type;
    return node;
}

AstNode* create_parameter_node(char* name, DataType type) {
    AstNode* node = create_node(NODE_PARAMETER);
    node->value.parameter.name = name;
    node->value.parameter.type = type;
    return node;
}

AstNode* create_unary_op_node(TokenType operator, AstNode* operand) {
    AstNode* node = create_node(NODE_UNARY_OP);
    node->value.unary_op.operator = operator;
    node->value.unary_op.operand = operand;
    return node;
}

void free_ast(AstNode* node) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_FUNCTION:
            free(node->value.function.name);
            for (int i = 0; i < node->value.function.param_count; i++) {
                free_ast(node->value.function.parameters[i]);
            }
            free(node->value.function.parameters);
            free(node->value.function.body);
            free(node->value.function.return_types);
            break;
        case NODE_VARIABLE:
            free(node->value.variable.name);
            free_ast(node->value.variable.init_value);
            break;
        case NODE_BINARY_OP:
            free_ast(node->value.binary_op.left);
            free_ast(node->value.binary_op.right);
            break;
        case NODE_RETURN:
            free_ast(node->value.return_stmt.return_value);
            break;
        case NODE_TUPLE:
            for (int i = 0; i < node->value.tuple.value_count; i++) {
                free_ast(node->value.tuple.values[i]);
            }
            free(node->value.tuple.values);
            break;
        case NODE_LITERAL:
            free(node->value.literal.value);
            break;
        case NODE_PARAMETER:
            free(node->value.parameter.name);
            break;
        case NODE_UNARY_OP:
            free_ast(node->value.unary_op.operand);
            break;
        case NODE_IF:
            free_ast(node->value.if_stmt.condition);
            for (int i = 0; i < node->value.if_stmt.then_branches_count; i++) {
                free_ast(node->value.if_stmt.then_branches[i]);
            }
            free_ast(node->value.if_stmt.else_branch);
            break;
    }

    free(node);
}

static void print_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
}

static const char* data_type_to_string(DataType type) {
    switch (type) {
        case TYPE_I8: return "i8";
        case TYPE_I16: return "i16";
        case TYPE_I32: return "i32";
        case TYPE_I64: return "i64";
        case TYPE_U8: return "u8";
        case TYPE_U16: return "u16";
        case TYPE_U32: return "u32";
        case TYPE_U64: return "u64";
        case TYPE_F32: return "f32";
        case TYPE_F64: return "f64";
        case TYPE_BOOL: return "bool";
        case TYPE_STR: return "str";
        case TYPE_NULL: return "null";
        case TYPE_ERROR: return "error";
        default: return "unknown";
    }
}

static const char* token_type_to_operator_string(TokenType type) {
    switch (type) {
        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_MULTIPLY: return "*";
        case TOKEN_DIVIDE: return "/";
        case TOKEN_EQUALS: return "==";
        case TOKEN_NOT_EQUAL: return "!=";
        case TOKEN_LESS: return "<";
        case TOKEN_LESS_EQUAL: return "<=";
        case TOKEN_GREATER: return ">";
        case TOKEN_GREATER_EQUAL: return ">=";
        case TOKEN_AND: return "&&";
        case TOKEN_OR: return "||";
        default: return "unknown";
    }
}

void print_ast(AstNode* node, int indent_level) {
    if (node == NULL) {
        print_indent(indent_level);
        printf("NULL\n");
        return;
    }

    switch (node->type) {
        case NODE_FUNCTION:
            print_indent(indent_level);
            printf("FUNCTION: %s\n", node->value.function.name);
            
            print_indent(indent_level + 1);
            printf("PARAMETERS (%d):\n", node->value.function.param_count);
            for (int i = 0; i < node->value.function.param_count; i++) {
                print_ast(node->value.function.parameters[i], indent_level + 2);
            }
            
            print_indent(indent_level + 1);
            printf("RETURN TYPES (%d):\n", node->value.function.return_type_count);
            for (int i = 0; i < node->value.function.return_type_count; i++) {
                print_indent(indent_level + 2);
                printf("%s\n", data_type_to_string(node->value.function.return_types[i]));
            }
            
            print_indent(indent_level + 1);
            printf("BODY:\n");
            print_ast(node->value.function.body, indent_level + 2);
            break;
            
        case NODE_VARIABLE:
            print_indent(indent_level);
            printf("VARIABLE: %s\n", node->value.variable.name);
            if (node->value.variable.init_value) {
                print_indent(indent_level + 1);
                printf("INIT VALUE:\n");
                print_ast(node->value.variable.init_value, indent_level + 2);
            }
            break;
            
        case NODE_BINARY_OP:
            print_indent(indent_level);
            printf("BINARY_OP: %s\n", token_type_to_operator_string(node->value.binary_op.operator));
            print_indent(indent_level + 1);
            printf("LEFT:\n");
            print_ast(node->value.binary_op.left, indent_level + 2);
            print_indent(indent_level + 1);
            printf("RIGHT:\n");
            print_ast(node->value.binary_op.right, indent_level + 2);
            break;
            
        case NODE_RETURN:
            print_indent(indent_level);
            printf("RETURN:\n");
            if (node->value.return_stmt.return_value) {
                print_ast(node->value.return_stmt.return_value, indent_level + 1);
            } else {
                print_indent(indent_level + 1);
                printf("NULL\n");
            }
            break;
            
        case NODE_TUPLE:
            print_indent(indent_level);
            printf("TUPLE (%d values):\n", node->value.tuple.value_count);
            for (int i = 0; i < node->value.tuple.value_count; i++) {
                print_indent(indent_level + 1);
                printf("VALUE %d:\n", i);
                print_ast(node->value.tuple.values[i], indent_level + 2);
            }
            break;
            
        case NODE_LITERAL:
            print_indent(indent_level);
            printf("LITERAL: %s (type: %s)\n", 
                   node->value.literal.value, 
                   data_type_to_string(node->value.literal.type));
            break;
            
        case NODE_PARAMETER:
            print_indent(indent_level);
            printf("PARAMETER: %s (type: %s)\n", 
                   node->value.parameter.name, 
                   data_type_to_string(node->value.parameter.type));
            break;
            
        case NODE_UNARY_OP:
            print_indent(indent_level);
            printf("UNARY_OP: %s\n", token_type_to_operator_string(node->value.unary_op.operator));
            print_indent(indent_level + 1);
            printf("OPERAND:\n");
            print_ast(node->value.unary_op.operand, indent_level + 2);
            break;

        case NODE_IF:
            print_indent(indent_level);
            printf("IF:\n");
            print_indent(indent_level + 1);
            printf("CONDITION:\n");
            print_ast(node->value.if_stmt.condition, indent_level + 2);

            for (int i = 0; i < node->value.if_stmt.then_branches_count; i++) {
                print_indent(indent_level + 1);
                printf(i == 0 ? "THEN:\n" : "ELSIF:\n");
                print_ast(node->value.if_stmt.then_branches[i], indent_level + 2);
            }

            if (node->value.if_stmt.else_branch) {
                print_indent(indent_level + 1);
                printf("ELSE:\n");
                print_ast(node->value.if_stmt.else_branch, indent_level + 2);
            }
            break;
            
        default:
            print_indent(indent_level);
            printf("UNKNOWN NODE TYPE: %d\n", node->type);
            break;
    }
}
