#include <isa.h>
#include <common.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <string.h>
#include <assert.h>

#define EVAL_IS_OPERAND(x) (x == TK_DEC || x == TK_HEX || x == TK_REG)

enum {
  TK_NOTYPE = 256, 
  TK_EQ, 
  TK_NEQ, 
  TK_DEC,
  TK_HEX,
  TK_NEG,
  TK_DEREF,
  TK_REG,
  TK_AND,
  TK_OR,
  TK_LE,
  TK_LT,
  TK_GE,
  TK_GT,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"-", '-'},           // substract
  {"\\*", '*'},         // multiply, \\ is for \ in C, not regex
  {"/", '/'},           // divide
  {"0[xX][[:xdigit:]]+", TK_HEX}, // hexdecimal
  {"[0-9]+", TK_DEC},     // decimal, this must be overriden by hex, so no problem
  {"\\(", '('},
  {"\\)", ')'},
  {"\\$[[:lower:][:digit:]]+", TK_REG}, // to make it isa independant, allow all kinds of combination
  {"&&", TK_AND},
  {"!=", TK_NEQ},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char *str;
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE: break;
          case TK_DEC: case TK_HEX:
          case TK_REG:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str = (char*)malloc(substr_len + 1);
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token ++;
            break;
          case '+': case '-': case '*': case '/': case '(': case ')':
          case TK_AND: case TK_EQ: case TK_NEQ:
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str = NULL;
            nr_token ++;
            break;          
          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

/*Parse Hex String into word_t
* Assume the format is 0x.....
* We are not doing sanity check, cause we have done regex before
*/
static word_t parse_hex(const char * str) {
  int i = 2;// we can assume the first two chars to be '0x', 'cause using regex
  word_t result = 0;
  while (str[i] != '\0') {
    if (str[i] >= '0' && str[i] <= '9')
      result = (result << 4) + str[i] - '0';
    else
      result = (result << 4) + str[i] - 'a' + 10;
    i += 1;
  }
  return result;
}

/*Parse Decimal String into word_t
* We are not doing sanity check, cause we have done regex before
*/
static word_t parse_dec(const char * str) {
  int i = 0;
  word_t result = 0;
  while (str[i] != '\0') {
    result = result * 10 + str[i] - '0';
    i += 1;
  }
  return result;
}

static int precedence(int op) {
  switch (op)
  {
  
  case '(': case ')':
    return 1;
  case TK_NEG: case TK_DEREF:
    return 2;
  case '*': case '/':
    return 3;
  case '+': case '-':
    return 4;
  case TK_LE: case TK_GE: case TK_LT: case TK_GT:
    return 6;
  case TK_EQ: case TK_NEQ:
    return 7;
  case TK_AND:
    return 11;
  case TK_OR:
    return 12;
  default:
    printf("UnKnown OP No.: %d\n", op);
    assert(0); // N't have to handle exception, if unknown operator occurs here, must be bugs
    break;
  }
}

word_t eval(int l, int r, bool *success) {
  int main_op_pos = -1;
  int lowest_prio = 0; // 0 --- the highest level
  int bracket_count = 0;
  int current_prio;
  int i;
  word_t result;
  word_t eval1, eval2;
  if (*success == false) return 0;
  if (l > r) {
    *success = false;
    printf("Eval failed @ token %d\n", l);
    return 0;
  }

  // Process leaf nodes, constants and regs
  if (l == r) {
    if (tokens[l].type == TK_HEX) {
      *success = true;
      // printf("parsing hex: %016lX\n", parse_hex(tokens[l].str));
      return parse_hex(tokens[l].str);
    } 
    else if (tokens[l].type == TK_DEC) {
      *success = true;
      // printf("parsing dec: %ld\n", parse_dec(tokens[l].str));
      return parse_dec(tokens[l].str);
    } 
    else if (tokens[l].type == TK_REG) {
      result = isa_reg_str2val(&(tokens[l].str[1]), success);
      if (*success == false) 
        printf("No such register: %s\n", tokens[l].str);
      return result;
    }
    else {
      printf("Leaf should be a number or register\n");
      *success = false;
      return 0;
    }
  }
  // Shake off a pair of parentheses
  else if (tokens[l].type == '(' && tokens[r].type == ')') {
    return eval(l + 1, r - 1, success);
  } 
  else {
    // process operator precedence, find main operator(LTR combination)
    for (i = l; i <= r; ++ i) {
      switch (tokens[i].type)
      {
      case '(':
        bracket_count ++;
      break;
      case ')':
        bracket_count --;
      break;
      case TK_DEC: case TK_HEX: case TK_REG:
      break; // Ignore Leaf
      case TK_DEREF: case TK_NEG:
      break; // Not process RTL operator this time
      default:
        current_prio = precedence(tokens[i].type);
        if (bracket_count == 0 && current_prio >= lowest_prio) {
          lowest_prio = current_prio;
          main_op_pos = i;
        }
        break;// all kinds of operators
      }
    }
    // handle exception
    if (bracket_count) {
      printf("unbalanced parentheses\n");
      *success = false;
      return 0;
    }
    // different process for RTL and LTR
    if (main_op_pos == -1) { 
      // no main operator found, but not a leaf, so try RTL operators
      switch (tokens[l].type)
      {
      case TK_NEG: return -eval(l+1, r, success); // all operations here are regarded as unsigned, so, neg is no recommended
      case TK_DEREF: TODO();
      default:
        printf("Unknown Single Operator: %d\n", tokens[i].type);
        *success = false;
        return 0;
      }
    }
    else {
      // normal binary operator
      // to simulate shortcut in || and &&, we have to dicide whether to eval the other side by operator
      switch (tokens[main_op_pos].type)
      {
      case '+':
        eval1 = eval(l, main_op_pos - 1, success); 
        eval2 = eval(main_op_pos + 1, r, success);
        if (*success == false) return 0; // Stop evalutation process at the first error
        result = eval1 + eval2;
        break;
      case '-':
        eval1 = eval(l, main_op_pos - 1, success); 
        eval2 = eval(main_op_pos + 1, r, success);
        if (*success == false) return 0;
        result = eval1 - eval2;
        break;
      case '*':
        eval1 = eval(l, main_op_pos - 1, success); 
        eval2 = eval(main_op_pos + 1, r, success);
        if (*success == false) return 0;
        result = eval1 * eval2;
        break;
      case '/':
        eval1 = eval(l, main_op_pos - 1, success); 
        eval2 = eval(main_op_pos + 1, r, success);
        if (*success == false) return 0;
        if (eval2 == 0) {
          printf("Dividing zero\n");
          *success = false;
          return 0;
        } else {
          result = eval1 / eval2;
        }
        break;
      case TK_AND:
        eval1 = eval(l, main_op_pos - 1, success);
        if (*success == false) return 0;
        if (eval1 == 0) {
          result = 0;
        }
        else {
          eval2 = eval(main_op_pos + 1, r, success);
          if (*success == false) return 0;
          result = eval2 != 0;
        }
        break;
      case TK_EQ:
        eval1 = eval(l, main_op_pos - 1, success); 
        eval2 = eval(main_op_pos + 1, r, success);
        if (*success == false) return 0;
        result = eval1 == eval2;
        break;
      case TK_NEQ:
        eval1 = eval(l, main_op_pos - 1, success); 
        eval2 = eval(main_op_pos + 1, r, success);
        if (*success == false) return 0;
        result = eval1 != eval2;
        break;
      default:
        assert(0);
        break;
      }
      printf("%ld %c %ld \n", eval1, (char)tokens[main_op_pos].type, eval2);
      *success = true;
      return result;
    }
  }
}

static void release_token() {
  int i;
  for (i = 0; i < nr_token; ++ i) {
    if (tokens[i].str != NULL) {
      free(tokens[i].str);
    }
  }
}

static void print_token() {
  int i;
  printf("Token Stream:\n");
  for (i = 0; i < nr_token; ++ i) {
    printf("(%d,%s),", tokens[i].type, tokens[i].str == NULL ? "null" : tokens[i].str);
  }
  puts("");
}



word_t expr(char *e, bool *success) {
  word_t result = 0;
  bool _success = true;
  int i;

  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  print_token();
  for (i = 0; i < nr_token; ++ i) {
    // no problem with that i-1, using shortcut
    if (tokens[i].type == '-' && (i == 0 || tokens[i-1].type == '(' || !EVAL_IS_OPERAND(tokens[i-1].type))) {
      tokens[i].type = TK_NEG;
    }
    if (tokens[i].type == '*' && (i == 0 || tokens[i-1].type == '(' || !EVAL_IS_OPERAND(tokens[i-1].type))) {
      tokens[i].type = TK_DEREF;
    }
  }
  print_token();
  result = eval(0, nr_token - 1, &_success);
  release_token(); // 'cause these strings are allocated dynamically, have to free them avoid leak
  *success = _success;
  return result;
}
