#include <isa.h>
#include <common.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <string.h>
#include <assert.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
  TK_DEC, TK_HEX,
  /* TODO: Add more token types */

};

enum {
  EVAL_NOERROR = 0,
  EVAL_LEAF,
  EVAL_BRACKET,
  EVAL_OP,
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

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
          case TK_DEC:
          case TK_HEX: 
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str = (char*)malloc(substr_len + 1);
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token ++;
            break;
          case '+': case '-': case '*': case '/': case '(': case ')':
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str = NULL;
            nr_token ++;
            break;
          case TK_EQ: TODO();
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

word_t eval(int l, int r, bool *success) {
  int main_op_pos = l;
  int lowest_prio = 2; // +,- : 0; *,/ : 1; 
  int bracket_count = 0;
  int i;
  word_t result;
  word_t eval1, eval2;
  if (*success == false) return 0;
  if (l > r) {
    *success = false;
    printf("Eval failed @ token %d\n", l);
    return 0;
  }
  if (l == r) {
    if (tokens[l].type == TK_HEX) {
      *success = true;
      // printf("parsing hex: %016lX\n", parse_hex(tokens[l].str));
      return parse_hex(tokens[l].str);
    } else if (tokens[r].type == TK_DEC) {
      *success = true;
      // printf("parsing dec: %ld\n", parse_dec(tokens[l].str));
      return parse_dec(tokens[l].str);
    } else {
      printf("Leaf should be a number or register\n");
      *success = false;
      return 0;
    }
  } else if (tokens[l].type == '(' && tokens[r].type == ')') {
    return eval(l + 1, r - 1, success);
  } else {
    // TODO: negative operator support
    for (i = l; i <= r; ++ i) {
      switch (tokens[i].type)
      {
      case '(':
        bracket_count ++;
      break;
      case ')':
        bracket_count --;
      break;
      case '+': case '-':
        if (bracket_count == 0) {
          lowest_prio = 0;
          main_op_pos = i;
        }
      break;
      case '*': case '/':
        if (bracket_count == 0 && lowest_prio >= 1) {
          lowest_prio = 1;
          main_op_pos = i;
        }
      break;
      default:
        break;
      }
    }

    if (bracket_count) {
      printf("unbalanced parentheses\n");
      *success = false;
      return 0;
    }
    eval1 = eval(l, main_op_pos - 1, success); 
    eval2 = eval(main_op_pos + 1, r, success);
    if (*success == false) return 0; // Stop evalutation process at the first error
    printf("%ld %c %ld \n", eval1, (char)tokens[main_op_pos].type, eval2);
    switch (tokens[main_op_pos].type)
    {
    case '+':
      result = eval1 + eval2;
      break;
    case '-':
      result = eval1 - eval2;
      break;
    case '*':
      result = eval1 * eval2;
      break;
    case '/':
      if (eval2 == 0) {
        printf("Dividing zero\n");
        *success = false;
        return 0;
      } else {
        result = eval1 / eval2;
      }
      break;
    default:
      assert(0);
      break;
    }
    *success = true;
    return result;
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

word_t expr(char *e, bool *success) {
  word_t result = 0;
  bool _success = true;
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  result = eval(0, nr_token - 1, &_success);
  release_token(); // 'cause these strings are allocated dynamically, have to free them avoid leak
  *success = _success;
  return result;
}
