#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAFEALLOC(var, Type) \
    if (((var) = (Type *)malloc(sizeof(Type))) == NULL) \
    err("not enough memory") // alocam memorie pentru un token și verifică dacă alocarea a reușit. Dacă nu, afisam eroare și oprim programul

enum { // definim codurile tokenilor pe care lexerul le poate recunoaste
    ID,
    BREAK,
    CHAR,
    DOUBLE,
    ELSE,
    FOR,
    IF,
    INT,
    RETURN,
    STRUCT,
    VOID,
    WHILE,
    CT_INT,
    CT_REAL,
    CT_CHAR,
    CT_STRING,
    COMMA,
    SEMICOLON,
    LPAR,
    RPAR,
    LBRACKET,
    RBRACKET,
    LACC,
    RACC,
    END,
    ADD,
    SUB,
    MUL,
    DIV,
    DOT,
    AND,
    OR,
    NOT,
    ASSIGN,
    EQUAL,
    NOTEQ,
    LESS,
    LESSEQ,
    GREATER,
    GREATEREQ,
    SPACE,
    LINECOMMENT
}; // token codes

typedef struct _Token { // definim cum arata un token în memorie
    int code;
    union {
        char *text;
        long int i;
        double r;
    };
    int line;
    struct _Token *next;
} Token;

static Token *tokens = NULL;
static Token *lastToken = NULL;
static Token *consumedToken= NULL;
static int line = 1;
static const char *pCrtCh = NULL;

static void err(const char *fmt, ...) // afisam o eroare generală și oprim programul
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

static Token *addTk(int code) // adaugam tokeni în listă
{
    Token *tk;
    SAFEALLOC(tk, Token);
    tk->code = code;
    tk->line = line;
    tk->next = NULL;
    if (lastToken) {
        lastToken->next = tk;
    } else {
        tokens = tk;
    }
    lastToken = tk;
    return tk;
}

int consume(int code) // pt. parser nu pt. lexer // verificam dacă tokenul curent are un anumit cod și avansam dacă se potriveste
{ 
    if(lastToken->code==code){ 
    consumedToken=lastToken; 
    lastToken=lastToken->next; 
    return 1; 
} 
return 0; 
} 

static void tkerr(const Token *tk, const char *fmt, ...) // este ca err, dar afisează si linia unde apare problema
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error in line %d: ", tk->line);
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
} 

static char *createString(const char *start, const char *end) // copiam o bucata din input într-un string nou
{
    size_t n = (size_t)(end - start);
    char *s = (char *)malloc(n + 1);
    if (s == NULL) {
        err("not enough memory");
    }
    memcpy(s, start, n);
    s[n] = '\0';
    return s;
}

static int getNextToken(void) // implementam FSM-ul. In funcție de state și de caracterul curent,
{                             // consumă caractere și ajunge într-o stare finală unde se creează tokenul
    int state = 0; // aici lexerul decide ce fel de token începe
    char ch;
    const char *pStartCh = NULL;
    Token *tk;
    long int charVal = 0;

    while (1) {
        ch = *pCrtCh;
        switch (state) {
        case 0: // initial state
            if (isalpha((unsigned char)ch) || ch == '_') { // daca primul caracter e literă sau _, poate fi ID sau keyword
                pStartCh = pCrtCh;
                pCrtCh++;
                state = 21; // identifier
            } else if (ch == '0') { // daca numărul începe cu 0, merg într-o stare separată fiindcă poate fi zero simplu, hex, real sau exponent
                pStartCh = pCrtCh;
                pCrtCh++;
                state = 1; // number starting with 0
            } else if (ch >= '1' && ch <= '9') { // daca începe cu 1-9, este început de constantă numerica
                pStartCh = pCrtCh;
                pCrtCh++;
                state = 3; // decimal number
            } else if (ch == '\'') {
                pCrtCh++;
                state = 14; // CT_CHAR
            } else if (ch == '"') {
                pStartCh = pCrtCh + 1;
                pCrtCh++;
                state = 19; // CT_STRING body
            } else if (ch == ',') {       // state 23: COMMA
                pCrtCh++;
                addTk(COMMA);
                return COMMA;
            } else if (ch == ';') {       // state 24: SEMICOLON
                pCrtCh++;
                addTk(SEMICOLON);
                return SEMICOLON;
            } else if (ch == '(') {       // state 25: LPAR
                pCrtCh++;
                addTk(LPAR);
                return LPAR;
            } else if (ch == ')') {       // state 26: RPAR
                pCrtCh++;
                addTk(RPAR);
                return RPAR;
            } else if (ch == '[') {       // state 27: LBRACKET
                pCrtCh++;
                addTk(LBRACKET);
                return LBRACKET;
            } else if (ch == ']') {       // state 28: RBRACKET
                pCrtCh++;
                addTk(RBRACKET);
                return RBRACKET;
            } else if (ch == '{') {       // state 29: LACC
                pCrtCh++;
                addTk(LACC);
                return LACC;
            } else if (ch == '}') {       // state 30: RACC
                pCrtCh++;
                addTk(RACC);
                return RACC;
            } else if (ch == '+') {       // state 57: ADD
                pCrtCh++;
                addTk(ADD);
                return ADD;
            } else if (ch == '-') {       // state 56: SUB
                pCrtCh++;
                addTk(SUB);
                return SUB;
            } else if (ch == '*') {       // state 55: MUL
                pCrtCh++;
                addTk(MUL);
                return MUL;
            } else if (ch == '/') {
                pCrtCh++;
                state = 36; // DIV or LINECOMMENT
            } else if (ch == '.') {
                pCrtCh++;
                addTk(DOT);
                return DOT;
            } else if (ch == '&') {
                pCrtCh++;
                state = 53; // AND
            } else if (ch == '|') {
                pCrtCh++;
                state = 51; // OR
            } else if (ch == '!') {
                pCrtCh++;
                state = 46; // NOT or NOTEQ
            } else if (ch == '=') {
                pCrtCh++;
                state = 42; // ASSIGN or EQUAL
            } else if (ch == '<') {
                pCrtCh++;
                state = 45; // LESS or LESSEQ
            } else if (ch == '>') {
                pCrtCh++;
                state = 39; // GREATER or GREATEREQ
            } else if (ch == ' ' || ch == '\r' || ch == '\t' || ch == '\n') {
                if (ch == '\n') line++;
                pCrtCh++;
                state = 35; // SPACE
            } else if (ch == '\0') {      // state 33: END
                addTk(END);
                return END;
            } else {
                tkerr(addTk(END), "invalid character `%c`", ch);
            }
            break;

        // Number starting with 0 (state 1) 
        case 1:
            if (ch == 'x' || ch == 'X') {
                pCrtCh++;
                state = 2; // hex digits
            } else if (ch >= '0' && ch <= '7') {
                pCrtCh++;
                state = 3; // octal/decimal digits
            } else if (ch == '.') {
                pCrtCh++;
                state = 5; // fractional part
            } else if (ch == 'e' || ch == 'E') {
                pCrtCh++;
                state = 10; // exponent from integer
            } else { // state 4: CT_INT
                tk = addTk(CT_INT);
                tk->i = 0;
                return CT_INT;
            }
            break;

        // Hex digits after 0x (state 2) 
        case 2:
            if (isdigit((unsigned char)ch) || (ch >= 'a' && ch <= 'f') ||
                (ch >= 'A' && ch <= 'F')) {
                pCrtCh++;
            } else { // state 4: CT_INT
                tk = addTk(CT_INT);
                tk->i = strtol(pStartCh, NULL, 16); // ramura pentru hexazecimal - baza 16 (transforma text din ex. 0xC in val. numerica)
                return CT_INT;
            }
            break;

        // Decimal/octal digits (state 3) 
        case 3: // starea 3 citește numere întregi zecimale. Dacă apare punct sau exponent, trece pe ramura de CT_REAL
            if (isdigit((unsigned char)ch)) {
                pCrtCh++;
            } else if (ch == '.') {
                pCrtCh++;
                state = 5; // fractional part
            } else if (ch == 'e' || ch == 'E') {
                pCrtCh++;
                state = 10; // exponent from integer
            } else { // state 4: CT_INT
                tk = addTk(CT_INT);
                tk->i = strtol(pStartCh, NULL, 10);
                return CT_INT;
            }
            break;

        // After '.' in number (state 5) 
        case 5: // după punct, regula cere cel puțin o cifră. Dacă nu apare cifră, dau eroare
            if (isdigit((unsigned char)ch)) {
                pCrtCh++;
                state = 6; // fractional digits
            } else {
                tkerr(addTk(END), "invalid real constant");
            }
            break;

        // Fractional digits (state 6) 
        case 6: // starea 6 citește cifrele de după punct. Dacă apare exponent, continuă; altfel creează CT_REAL
            if (isdigit((unsigned char)ch)) {
                pCrtCh++;
            } else if (ch == 'e' || ch == 'E') {
                pCrtCh++;
                state = 7; // exponent from fractional
            } else { // state 13: CT_REAL
                tk = addTk(CT_REAL);
                tk->r = strtod(pStartCh, NULL); //strtod transforma textul de ex. 2e0 in val. numerica 2.0
                return CT_REAL;
            }
            break;

        // After 'e'/'E' from fractional (state 7)
		// starile 7, 8 și 9 tratează exponentul pentru un real cu punct. După e poate exista semn, dar apoi trebuie obligatoriu cifre 
        case 7:
            if (ch == '+' || ch == '-') {
                pCrtCh++;
                state = 8;
            } else if (isdigit((unsigned char)ch)) {
                pCrtCh++;
                state = 9;
            } else {
                tkerr(addTk(END), "invalid real constant (exponent)");
            }
            break;

        // After sign in exponent from fractional (state 8) 
        case 8:
            if (isdigit((unsigned char)ch)) {
                pCrtCh++;
                state = 9;
            } else {
                tkerr(addTk(END), "invalid real constant (exponent)");
            }
            break;

        // Exponent digits from fractional (state 9) 
        case 9:
            if (isdigit((unsigned char)ch)) {
                pCrtCh++;
            } else { // state 13: CT_REAL
                tk = addTk(CT_REAL);
                tk->r = strtod(pStartCh, NULL);
                return CT_REAL;
            }
            break;

        // After 'e'/'E' from integer (state 10)
		// starile 10, 11 și 12 tratează realii fără punct, dar cu exponent, de forma 12e3 
        case 10:
            if (ch == '+' || ch == '-') {
                pCrtCh++;
                state = 11;
            } else if (isdigit((unsigned char)ch)) {
                pCrtCh++;
                state = 12;
            } else {
                tkerr(addTk(END), "invalid real constant (exponent)");
            }
            break;

        // After sign in exponent from integer (state 11) 
        case 11:
            if (isdigit((unsigned char)ch)) {
                pCrtCh++;
                state = 12;
            } else {
                tkerr(addTk(END), "invalid real constant (exponent)");
            }
            break;

        // Exponent digits from integer (state 12) 
        case 12:
            if (isdigit((unsigned char)ch)) {
                pCrtCh++;
            } else { // state 13: CT_REAL
                tk = addTk(CT_REAL);
                tk->r = strtod(pStartCh, NULL);
                return CT_REAL;
            }
            break;

        // CT_CHAR: after opening quote (state 14)
		// starea 14 citește conținutul constantei caracter. Dacă vede backslash, merge pe ramura de escape 
        case 14:
            if (ch == '\\') {
                pCrtCh++;
                state = 16; // escape sequence
            } else if (ch != '\'' && ch != '\0') {
                charVal = (unsigned char)ch;
                pCrtCh++;
                state = 15; // got char value
            } else {
                tkerr(addTk(END), "invalid char constant");
            }
            break;

        // CT_CHAR: got char value (state 15)
		// starea 15 verifică apostroful de închidere și apoi creează tokenul CT_CHAR cu valoarea caracterului.
        case 15:
            if (ch == '\'') { // state 17: CT_CHAR
                pCrtCh++;
                tk = addTk(CT_CHAR);
                tk->i = charVal;
                return CT_CHAR;
            } else {
                tkerr(addTk(END), "missing closing quote for char constant");
            }
            break;

        // CT_CHAR: escape sequence (state 16)
		// starea 16 tratează escape sequences pentru caractere
        case 16:
            if (ch != '\0') {
                switch (ch) {
                case 'n': charVal = '\n'; break;
                case 't': charVal = '\t'; break;
                case '\\': charVal = '\\'; break;
                case '\'': charVal = '\''; break;
                case '0': charVal = '\0'; break;
                case 'r': charVal = '\r'; break;
                case 'a': charVal = '\a'; break;
                case 'b': charVal = '\b'; break;
                default: charVal = (unsigned char)ch; break;
                }
                pCrtCh++;
                state = 15; // got escaped char value
            } else {
                tkerr(addTk(END), "invalid escape in char constant");
            }
            break;

        // CT_STRING: escape in string (state 18)
		// starea 18 tratează escape-ul în string, ca ghilimelele escapate să nu închidă stringul
        case 18:
            if (ch != '\0') {
                pCrtCh++;
                state = 19; // back to string body
            } else {
                tkerr(addTk(END), "unterminated string constant");
            }
            break;

        // CT_STRING: string body (state 19)
		// starea 19 citește conținutul stringului până la ghilimeaua de închidere
        case 19:
            if (ch == '\\') {
                pCrtCh++;
                state = 18; // escape in string
            } else if (ch == '"') { // state 20: CT_STRING
                tk = addTk(CT_STRING);
                tk->text = createString(pStartCh, pCrtCh);
                pCrtCh++;
                return CT_STRING;
            } else if (ch == '\0') {
                tkerr(addTk(END), "unterminated string constant");
            } else {
                pCrtCh++;
            }
            break;

        // Identifier (state 21)
		// Starea 21 citește identificatori. După ce lexemul se termină, îl compar cu lista de keyword-uri. 
		// Dacă nu este keyword, creez tokenul ID și salvez textul
        case 21:
            if (isalnum((unsigned char)ch) || ch == '_') {
                pCrtCh++;
            } else { // state 22: emit ID or keyword
                int nCh = (int)(pCrtCh - pStartCh);
                if (nCh == 5 && !memcmp(pStartCh, "break", 5)) tk = addTk(BREAK);
                else if (nCh == 4 && !memcmp(pStartCh, "char", 4)) tk = addTk(CHAR);
                else if (nCh == 6 && !memcmp(pStartCh, "double", 6)) tk = addTk(DOUBLE);
                else if (nCh == 4 && !memcmp(pStartCh, "else", 4)) tk = addTk(ELSE);
                else if (nCh == 3 && !memcmp(pStartCh, "for", 3)) tk = addTk(FOR);
                else if (nCh == 2 && !memcmp(pStartCh, "if", 2)) tk = addTk(IF);
                else if (nCh == 3 && !memcmp(pStartCh, "int", 3)) tk = addTk(INT);
                else if (nCh == 6 && !memcmp(pStartCh, "return", 6)) tk = addTk(RETURN);
                else if (nCh == 6 && !memcmp(pStartCh, "struct", 6)) tk = addTk(STRUCT);
                else if (nCh == 4 && !memcmp(pStartCh, "void", 4)) tk = addTk(VOID);
                else if (nCh == 5 && !memcmp(pStartCh, "while", 5)) tk = addTk(WHILE);
                else {
                    tk = addTk(ID);
                    tk->text = createString(pStartCh, pCrtCh);
                }
                return tk->code;
            }
            break;

        // SPACE (state 35) - consume whitespace, no token emitted
		// starea 35 consumă whitespace-ul. Nu se generează token, doar se actualizează linia pentru newline
        case 35:
            if (ch == ' ' || ch == '\r' || ch == '\t' || ch == '\n') {
                if (ch == '\n') line++;
                pCrtCh++;
            } else {
                state = 0; // back to initial
            }
            break;

        // DIV or LINECOMMENT (state 36)
		// starea 36 decide între operatorul / și comentariul //
        case 36:
            if (ch == '/') {
                pCrtCh++;
                state = 37; // line comment body
            } else { // state 36: DIV
                addTk(DIV);
                return DIV;
            }
            break;

        // Line comment body (state 37) 
		// starea 37 consumă tot comentariul de linie și apoi revine la starea inițiala
        case 37:
            if (ch == '\n' || ch == '\r' || ch == '\0') {
                state = 0; // state 38: done with comment
            } else {
                pCrtCh++;
            }
            break;

        // After '>' (state 39) 
		// daca după > vine =, produce GREATEREQ. Altfel produce GREATER
        case 39: 
            if (ch == '=') { // state 41: GREATEREQ
                pCrtCh++;
                addTk(GREATEREQ);
                return GREATEREQ;
            } else { // state 40: GREATER
                addTk(GREATER);
                return GREATER;
            }

        // After '=' (state 42) 
		// dacă după = vine =, produce EQUAL. Altfel produce ASSIGN
        case 42:
            if (ch == '=') { // state 48: EQUAL
                pCrtCh++;
                addTk(EQUAL);
                return EQUAL;
            } else { // state 49: ASSIGN
                addTk(ASSIGN);
                return ASSIGN;
            }

        // After '<' (state 45) 
        case 45:
            if (ch == '=') { // state 44: LESSEQ
                pCrtCh++;
                addTk(LESSEQ);
                return LESSEQ;
            } else { // state 43: LESS
                addTk(LESS);
                return LESS;
            }

        // After '!' (state 46)
        case 46:
            if (ch == '=') { // state 50: NOTEQ
                pCrtCh++;
                addTk(NOTEQ);
                return NOTEQ;
            } else { // state 47: NOT
                addTk(NOT);
                return NOT;
            }

        // After '|' (state 51) 
		// dupa | trebuie să mai fie un |
        case 51:
            if (ch == '|') { // state 52: OR
                pCrtCh++;
                addTk(OR);
                return OR;
            } else {
                tkerr(addTk(END), "expected '||'");
            }
            break;

        // After '&' (state 53) 
		// dupa & trebuie să mai fie un &
		// aceste stări sunt pentru operatori care au prefix comun. După primul caracter trebuie să verific dacă urmează = 
        // sau al doilea caracter logic
		case 53:
            if (ch == '&') { // state 54: AND
                pCrtCh++;
                addTk(AND);
                return AND;
            } else {
                tkerr(addTk(END), "expected '&&'");
            }
            break;
        }
    }
}

static const char *tokenName(int code) // folosim doar pentru afișare, ca să vedem numele tokenului, nu doar codul numeric
{
    switch (code) {
    case ID:        return "ID";
    case BREAK:     return "BREAK";
    case CHAR:      return "CHAR";
    case DOUBLE:    return "DOUBLE";
    case ELSE:      return "ELSE";
    case FOR:       return "FOR";
    case IF:        return "IF";
    case INT:       return "INT";
    case RETURN:    return "RETURN";
    case STRUCT:    return "STRUCT";
    case VOID:      return "VOID";
    case WHILE:     return "WHILE";
    case CT_INT:    return "CT_INT";
    case CT_REAL:   return "CT_REAL";
    case CT_CHAR:   return "CT_CHAR";
    case CT_STRING: return "CT_STRING";
    case COMMA:     return "COMMA";
    case SEMICOLON: return "SEMICOLON";
    case LPAR:      return "LPAR";
    case RPAR:      return "RPAR";
    case LBRACKET:  return "LBRACKET";
    case RBRACKET:  return "RBRACKET";
    case LACC:      return "LACC";
    case RACC:      return "RACC";
    case END:       return "END";
    case ADD:       return "ADD";
    case SUB:       return "SUB";
    case MUL:       return "MUL";
    case DIV:       return "DIV";
    case DOT:       return "DOT";
    case AND:       return "AND";
    case OR:        return "OR";
    case NOT:       return "NOT";
    case ASSIGN:    return "ASSIGN";
    case EQUAL:     return "EQUAL";
    case NOTEQ:     return "NOTEQ";
    case LESS:      return "LESS";
    case LESSEQ:    return "LESSEQ";
    case GREATER:   return "GREATER";
    case GREATEREQ: return "GREATEREQ";
    default:        return "UNKNOWN";
    }
}

static void showTokens(void) // afisam lista de tokeni și atributele lor
{
    Token *tk;
    for (tk = tokens; tk; tk = tk->next) {
        printf("line %d: %s", tk->line, tokenName(tk->code));
        if (tk->code == ID) {
            printf(":%s", tk->text);
        } else if (tk->code == CT_INT) {
            printf(":%ld", tk->i);  //arata val. numerica finala
        } else if (tk->code == CT_REAL) {
            printf(":%g", tk->r);
        } else if (tk->code == CT_CHAR) {
            printf(":%c", (char)tk->i);
        } else if (tk->code == CT_STRING) {
            printf(":%s", tk->text);
        }
        printf("\n");
    }
}


// ===================== SEMANTIC ANALYSIS: DOMAIN + TYPE =====================
// Added for LFTC-L5/LFTC-L6. Lexical code above is kept as it was.

// Type base codes
enum { TB_INT, TB_DOUBLE, TB_CHAR, TB_STRUCT, TB_VOID };
// Symbol kinds
enum { SK_VAR, SK_PARAM, SK_FN, SK_EXTFN, SK_STRUCT };

struct _Symbol;
typedef struct _Symbol Symbol;

typedef struct {
    int tb;          // TB_*
    Symbol *s;       // struct definition, only for TB_STRUCT
    int n;           // >0 array with size, 0 array without size, <0 scalar/non-array
} Type;

typedef struct {
    Symbol **begin;
    Symbol **end;
    Symbol **after;
} Symbols;

struct _Symbol {
    const char *name;
    int kind;          // SK_*
    Type type;
    int depth;         // 0 global, 1 function domain, 2+ nested blocks
    Symbol *owner;     // current function/struct owner for locals/members
    int varIdx;
    int paramIdx;
    Symbols params;        // for functions
    Symbols locals;        // for functions
    Symbols structMembers; // for structs
};

typedef union {
    long int i;
    double d;
    const char *str;
} CtVal;

typedef struct {
    Type type;
    int lval;
    int ct;
    CtVal ctVal;
} Ret;

static Symbols symTable;
static int crtDepth = 0;
static Symbol *owner = NULL;
static Symbol **domainStack = NULL; // saved symTable.end values
static int domainStackLen = 0;
static int domainStackCap = 0;

static void initSymbols(Symbols *s)
{
    s->begin = s->end = s->after = NULL;
}

static int symbolsLen(Symbols s)
{
    return s.begin ? (int)(s.end - s.begin) : 0;
}

static Symbol *addSymbolToList(Symbols *symbols, Symbol *sym)
{
    int count, n;
    if (symbols->end == symbols->after) {
        count = symbols->begin ? (int)(symbols->end - symbols->begin) : 0;
        n = count * 2;
        if (n == 0) n = 4;
        symbols->begin = (Symbol **)realloc(symbols->begin, (size_t)n * sizeof(Symbol *));
        if (!symbols->begin) err("not enough memory");
        symbols->end = symbols->begin + count;
        symbols->after = symbols->begin + n;
    }
    *symbols->end++ = sym;
    return sym;
}

static Symbol *newSymbol(const char *name, int kind)
{
    Symbol *s;
    SAFEALLOC(s, Symbol);
    s->name = name;
    s->kind = kind;
    s->type.tb = TB_INT;
    s->type.s = NULL;
    s->type.n = -1;
    s->depth = crtDepth;
    s->owner = NULL;
    s->varIdx = -1;
    s->paramIdx = -1;
    initSymbols(&s->params);
    initSymbols(&s->locals);
    initSymbols(&s->structMembers);
    return s;
}

static Symbol *dupSymbol(const Symbol *src)
{
    Symbol *s;
    SAFEALLOC(s, Symbol);
    *s = *src;
    initSymbols(&s->params);
    initSymbols(&s->locals);
    initSymbols(&s->structMembers);
    // For params/members we only need the shallow type/name/kind info.
    return s;
}

static Symbol *addSymbolToDomain(Symbols *symbols, Symbol *s)
{
    s->depth = crtDepth;
    return addSymbolToList(symbols, s);
}

static Symbol *findSymbolInList(Symbols symbols, const char *name)
{
    Symbol **p;
    if (!symbols.begin) return NULL;
    for (p = symbols.end; p != symbols.begin; ) {
        --p;
        if ((*p)->name && !strcmp((*p)->name, name)) return *p;
    }
    return NULL;
}

static Symbol *findSymbol(const char *name)
{
    return findSymbolInList(symTable, name);
}

static Symbol *findSymbolInDomain(Symbols symbols, const char *name)
{
    Symbol **p;
    if (!symbols.begin) return NULL;
    for (p = symbols.end; p != symbols.begin; ) {
        --p;
        if ((*p)->depth == crtDepth && (*p)->name && !strcmp((*p)->name, name)) return *p;
    }
    return NULL;
}

static void pushDomain(void)
{
    if (domainStackLen == domainStackCap) {
        domainStackCap = domainStackCap ? domainStackCap * 2 : 8;
        domainStack = (Symbol **)realloc(domainStack, (size_t)domainStackCap * sizeof(Symbol *));
        if (!domainStack) err("not enough memory");
    }
    domainStack[domainStackLen++] = (Symbol *)symTable.end; // saved end pointer
    crtDepth++;
}

static void dropDomain(void)
{
    if (domainStackLen <= 0) err("internal error: empty domain stack");
    symTable.end = (Symbol **)domainStack[--domainStackLen];
    crtDepth--;
}

static Type createType(int tb, int n)
{
    Type t;
    t.tb = tb;
    t.s = NULL;
    t.n = n;
    return t;
}

static const char *typeBaseName(int tb)
{
    switch (tb) {
    case TB_INT: return "int";
    case TB_DOUBLE: return "double";
    case TB_CHAR: return "char";
    case TB_STRUCT: return "struct";
    case TB_VOID: return "void";
    default: return "?";
    }
}

static void printType(Type t)
{
    if (t.tb == TB_STRUCT && t.s) printf("struct %s", t.s->name);
    else printf("%s", typeBaseName(t.tb));
    if (t.n == 0) printf("[]");
    else if (t.n > 0) printf("[%d]", t.n);
}

static int typeSize(Type *t)
{
    int base = 1;
    switch (t->tb) {
    case TB_CHAR: base = 1; break;
    case TB_INT: base = 4; break;
    case TB_DOUBLE: base = 8; break;
    case TB_STRUCT: base = 1; break; // not used for real code generation here
    default: base = 0; break;
    }
    return t->n > 0 ? base * t->n : base;
}

static int canBeScalar(Ret *r)
{
    return r->type.n < 0 && r->type.tb != TB_STRUCT && r->type.tb != TB_VOID;
}

static int convTo(Type *src, Type *dst)
{
    if (src->n >= 0) {
        if (dst->n >= 0) {
            if (src->tb != dst->tb) return 0;
            if (src->tb == TB_STRUCT && src->s != dst->s) return 0;
            return 1;
        }
        return 0;
    }
    if (dst->n >= 0) return 0;

    if ((src->tb == TB_CHAR || src->tb == TB_INT || src->tb == TB_DOUBLE) &&
        (dst->tb == TB_CHAR || dst->tb == TB_INT || dst->tb == TB_DOUBLE)) return 1;

    if (src->tb == TB_STRUCT && dst->tb == TB_STRUCT && src->s == dst->s) return 1;
    return 0;
}

static int arithTypeTo(Type *a, Type *b, Type *dst)
{
    if (a->n >= 0 || b->n >= 0) return 0;
    if (a->tb == TB_STRUCT || b->tb == TB_STRUCT) return 0;
    if (a->tb == TB_VOID || b->tb == TB_VOID) return 0;
    if (a->tb == TB_DOUBLE || b->tb == TB_DOUBLE) *dst = createType(TB_DOUBLE, -1);
    else if (a->tb == TB_INT || b->tb == TB_INT) *dst = createType(TB_INT, -1);
    else *dst = createType(TB_CHAR, -1);
    return 1;
}

static Symbol *addExtFunc(const char *name, Type type)
{
    Symbol *s = newSymbol(name, SK_EXTFN);
    s->type = type;
    addSymbolToDomain(&symTable, s);
    return s;
}

static void addFuncArg(Symbol *func, const char *name, Type type)
{
    Symbol *a = newSymbol(name, SK_PARAM);
    a->type = type;
    a->paramIdx = symbolsLen(func->params);
    addSymbolToList(&func->params, a);
}

static void addExtFuncs(void)
{
    Symbol *s;
    s = addExtFunc("put_s", createType(TB_VOID, -1)); addFuncArg(s, "s", createType(TB_CHAR, 0));
    s = addExtFunc("get_s", createType(TB_VOID, -1)); addFuncArg(s, "s", createType(TB_CHAR, 0));
    s = addExtFunc("put_i", createType(TB_VOID, -1)); addFuncArg(s, "i", createType(TB_INT, -1));
    addExtFunc("get_i", createType(TB_INT, -1));
    s = addExtFunc("put_d", createType(TB_VOID, -1)); addFuncArg(s, "d", createType(TB_DOUBLE, -1));
    addExtFunc("get_d", createType(TB_DOUBLE, -1));
    s = addExtFunc("put_c", createType(TB_VOID, -1)); addFuncArg(s, "c", createType(TB_CHAR, -1));
    addExtFunc("get_c", createType(TB_CHAR, -1));
    addExtFunc("seconds", createType(TB_DOUBLE, -1));
}

static void showSymbols(void)
{
    Symbol **p;
    printf("\n--- symbols ---\n");
    for (p = symTable.begin; p && p != symTable.end; ++p) {
        Symbol *s = *p;
        printf("depth %d: %s  kind=", s->depth, s->name);
        switch (s->kind) {
        case SK_VAR: printf("var"); break;
        case SK_PARAM: printf("param"); break;
        case SK_FN: printf("fn"); break;
        case SK_EXTFN: printf("extfn"); break;
        case SK_STRUCT: printf("struct"); break;
        }
        printf(" type="); printType(s->type); printf("\n");
    }
}

// Parser prototypes with semantic attributes
int expr(Ret *r);
int exprAssign(Ret *r);
int exprOr(Ret *r);       int exprOr1(Ret *r);
int exprAnd(Ret *r);      int exprAnd1(Ret *r);
int exprEq(Ret *r);       int exprEq1(Ret *r);
int exprRel(Ret *r);      int exprRel1(Ret *r);
int exprAdd(Ret *r);      int exprAdd1(Ret *r);
int exprMul(Ret *r);      int exprMul1(Ret *r);
int exprCast(Ret *r);
int exprUnary(Ret *r);
int exprPostfix(Ret *r);  int exprPostfix1(Ret *r);
int exprPrimary(Ret *r);
int stm(void);
int stmCompound(int newDomain);
int typeBase(Type *t);
int arrayDecl(Type *t);
int varDef(void);
int fnParam(void);
int fnDef(void);
int structDef(void);
int unit(void);

int typeBase(Type *t)
{
    Token *startTk = lastToken;
    t->n = -1;
    t->s = NULL;
    if (consume(INT)) { t->tb = TB_INT; return 1; }
    if (consume(DOUBLE)) { t->tb = TB_DOUBLE; return 1; }
    if (consume(CHAR)) { t->tb = TB_CHAR; return 1; }
    if (consume(STRUCT)) {
        if (consume(ID)) {
            Token *tkName = consumedToken;
            Symbol *s = findSymbol(tkName->text);
            if (!s || s->kind != SK_STRUCT) tkerr(tkName, "undefined struct: %s", tkName->text);
            t->tb = TB_STRUCT;
            t->s = s;
            t->n = -1;
            return 1;
        }
        tkerr(lastToken, "missing identifier after struct");
    }
    lastToken = startTk;
    return 0;
}

int arrayDecl(Type *t)
{
    Token *startTk = lastToken;
    if (consume(LBRACKET)) {
        if (consume(CT_INT)) t->n = (int)consumedToken->i;
        else t->n = 0;
        if (consume(RBRACKET)) return 1;
        tkerr(lastToken, "missing ] in array declaration");
    }
    lastToken = startTk;
    return 0;
}

int varDef(void)
{
    Token *startTk = lastToken;
    Type t;
    if (typeBase(&t)) {
        if (consume(ID)) {
            Token *tkName = consumedToken;
            int hasArray = arrayDecl(&t);
            if (hasArray && t.n == 0) tkerr(tkName, "a vector variable must have a specified dimension");
            if (consume(SEMICOLON)) {
                Symbol *var = findSymbolInDomain(symTable, tkName->text);
                if (var) tkerr(tkName, "symbol redefinition: %s", tkName->text);
                var = newSymbol(tkName->text, SK_VAR);
                var->type = t;
                var->owner = owner;
                addSymbolToDomain(&symTable, var);
                if (owner) {
                    if (owner->kind == SK_FN) {
                        var->varIdx = symbolsLen(owner->locals);
                        addSymbolToList(&owner->locals, dupSymbol(var));
                    } else if (owner->kind == SK_STRUCT) {
                        var->varIdx = typeSize(&t);
                        addSymbolToList(&owner->structMembers, dupSymbol(var));
                    }
                }
                return 1;
            }
        }
        lastToken = startTk;
    }
    return 0;
}

int fnParam(void)
{
    Token *startTk = lastToken;
    Type t;
    if (typeBase(&t)) {
        if (consume(ID)) {
            Token *tkName = consumedToken;
            if (arrayDecl(&t)) t.n = 0; // parameters declared as vectors lose their explicit dimension
            Symbol *param = findSymbolInDomain(symTable, tkName->text);
            if (param) tkerr(tkName, "symbol redefinition: %s", tkName->text);
            param = newSymbol(tkName->text, SK_PARAM);
            param->type = t;
            param->owner = owner;
            param->paramIdx = symbolsLen(owner->params);
            addSymbolToDomain(&symTable, param);
            addSymbolToList(&owner->params, dupSymbol(param));
            return 1;
        }
        lastToken = startTk;
    }
    return 0;
}

int structDef(void)
{
    Token *startTk = lastToken;
    if (consume(STRUCT)) {
        if (consume(ID)) {
            Token *tkName = consumedToken;
            if (consume(LACC)) {
                Symbol *s = findSymbolInDomain(symTable, tkName->text);
                if (s) tkerr(tkName, "symbol redefinition: %s", tkName->text);
                s = newSymbol(tkName->text, SK_STRUCT);
                s->type.tb = TB_STRUCT;
                s->type.s = s;
                s->type.n = -1;
                addSymbolToDomain(&symTable, s);
                pushDomain();
                owner = s;
                while (varDef()) {}
                if (consume(RACC)) {
                    if (consume(SEMICOLON)) {
                        owner = NULL;
                        dropDomain();
                        return 1;
                    }
                    tkerr(lastToken, "missing ; after struct definition");
                }
                tkerr(lastToken, "missing } in struct definition");
            }
        }
        lastToken = startTk;
    }
    return 0;
}

int fnDef(void)
{
    Token *startTk = lastToken;
    Type t;
    if (typeBase(&t) || (consume(VOID) && (t = createType(TB_VOID, -1), 1))) {
        if (consume(ID)) {
            Token *tkName = consumedToken;
            if (consume(LPAR)) {
                Symbol *fn = findSymbolInDomain(symTable, tkName->text);
                if (fn) tkerr(tkName, "symbol redefinition: %s", tkName->text);
                fn = newSymbol(tkName->text, SK_FN);
                fn->type = t;
                addSymbolToDomain(&symTable, fn);
                owner = fn;
                pushDomain();
                if (fnParam()) {
                    while (consume(COMMA)) {
                        if (!fnParam()) tkerr(lastToken, "missing parameter after ,");
                    }
                }
                if (consume(RPAR)) {
                    if (stmCompound(0)) {
                        dropDomain();
                        owner = NULL;
                        return 1;
                    }
                    tkerr(lastToken, "missing function body");
                }
                tkerr(lastToken, "missing ) in function definition");
            }
        }
        lastToken = startTk;
    }
    return 0;
}

int stmCompound(int newDomain)
{
    Token *startTk = lastToken;
    if (consume(LACC)) {
        if (newDomain) pushDomain();
        while (1) {
            if (varDef()) {}
            else if (stm()) {}
            else break;
        }
        if (consume(RACC)) {
            if (newDomain) dropDomain();
            return 1;
        }
        tkerr(lastToken, "missing } or syntax error");
    }
    lastToken = startTk;
    return 0;
}

int stm(void)
{
    Token *startTk = lastToken;
    Ret r;
    if (stmCompound(1)) return 1;

    if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr(&r)) {
                if (!canBeScalar(&r)) tkerr(consumedToken, "the if condition must be a scalar value");
                if (consume(RPAR)) {
                    if (stm()) {
                        if (consume(ELSE)) {
                            if (!stm()) tkerr(lastToken, "missing statement after else");
                        }
                        return 1;
                    }
                    tkerr(lastToken, "missing if body");
                }
                tkerr(lastToken, "missing ) after if condition");
            }
            tkerr(lastToken, "invalid expression after (");
        }
        tkerr(lastToken, "missing ( after if");
    }

    if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr(&r)) {
                if (!canBeScalar(&r)) tkerr(consumedToken, "the while condition must be a scalar value");
                if (consume(RPAR)) {
                    if (stm()) return 1;
                    tkerr(lastToken, "missing while statement");
                }
                tkerr(lastToken, "missing )");
            }
            tkerr(lastToken, "invalid expression after (");
        }
        tkerr(lastToken, "missing ( after while");
    }

    if (consume(FOR)) {
        if (consume(LPAR)) {
            expr(&r);
            if (consume(SEMICOLON)) {
                if (expr(&r)) {
                    if (!canBeScalar(&r)) tkerr(consumedToken, "the for condition must be a scalar value");
                }
                if (consume(SEMICOLON)) {
                    expr(&r);
                    if (consume(RPAR)) {
                        if (stm()) return 1;
                        tkerr(lastToken, "missing for body");
                    }
                    tkerr(lastToken, "missing ) after for");
                }
                tkerr(lastToken, "missing ; in for");
            }
            tkerr(lastToken, "missing ; in for");
        }
        tkerr(lastToken, "missing ( after for");
    }

    if (consume(BREAK)) {
        if (consume(SEMICOLON)) return 1;
        tkerr(lastToken, "missing ; after break");
    }

    if (consume(RETURN)) {
        if (expr(&r)) {
            if (!owner || owner->type.tb == TB_VOID) tkerr(consumedToken, "a void function cannot return a value");
            if (!canBeScalar(&r)) tkerr(consumedToken, "the return value must be a scalar value");
            if (!convTo(&r.type, &owner->type)) tkerr(consumedToken, "cannot convert the return expression type to the function return type");
        } else {
            if (owner && owner->type.tb != TB_VOID) tkerr(lastToken, "a non-void function must return a value");
        }
        if (consume(SEMICOLON)) return 1;
        tkerr(lastToken, "missing ; after return");
    }

    if (expr(&r)) {
        if (consume(SEMICOLON)) return 1;
        tkerr(lastToken, "missing ; after expression");
    }
    if (consume(SEMICOLON)) return 1;

    lastToken = startTk;
    return 0;
}

int expr(Ret *r) { return exprAssign(r); }

int exprAssign(Ret *r)
{
    Token *startTk = lastToken;
    Ret rDst;
    if (exprUnary(&rDst)) {
        if (consume(ASSIGN)) {
            if (exprAssign(r)) {
                if (!rDst.lval) tkerr(consumedToken, "the assign destination must be a left-value");
                if (rDst.ct) tkerr(consumedToken, "the assign destination cannot be constant");
                if (!canBeScalar(&rDst)) tkerr(consumedToken, "the assign destination must be scalar");
                if (!canBeScalar(r)) tkerr(consumedToken, "the assign source must be scalar");
                if (!convTo(&r->type, &rDst.type)) tkerr(consumedToken, "the assign source cannot be converted to destination");
                r->lval = 0;
                r->ct = 1;
                return 1;
            }
            tkerr(lastToken, "missing assignment expression");
        }
        lastToken = startTk;
    }
    return exprOr(r);
}

int exprOr(Ret *r) { return exprAnd(r) ? exprOr1(r) : 0; }
int exprOr1(Ret *r)
{
    if (consume(OR)) {
        Ret right; Type tDst;
        if (exprAnd(&right)) {
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(consumedToken, "invalid operand type for ||");
            *r = (Ret){createType(TB_INT, -1), 0, 1, {0}};
            return exprOr1(r);
        }
        tkerr(lastToken, "invalid expression after ||");
    }
    return 1;
}

int exprAnd(Ret *r) { return exprEq(r) ? exprAnd1(r) : 0; }
int exprAnd1(Ret *r)
{
    if (consume(AND)) {
        Ret right; Type tDst;
        if (exprEq(&right)) {
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(consumedToken, "invalid operand type for &&");
            *r = (Ret){createType(TB_INT, -1), 0, 1, {0}};
            return exprAnd1(r);
        }
        tkerr(lastToken, "invalid expression after &&");
    }
    return 1;
}

int exprEq(Ret *r) { return exprRel(r) ? exprEq1(r) : 0; }
int exprEq1(Ret *r)
{
    if (consume(EQUAL) || consume(NOTEQ)) {
        Ret right; Type tDst;
        if (exprRel(&right)) {
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(consumedToken, "invalid operand type for == or !=");
            *r = (Ret){createType(TB_INT, -1), 0, 1, {0}};
            return exprEq1(r);
        }
        tkerr(lastToken, "invalid expression after == or !=");
    }
    return 1;
}

int exprRel(Ret *r) { return exprAdd(r) ? exprRel1(r) : 0; }
int exprRel1(Ret *r)
{
    if (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)) {
        Ret right; Type tDst;
        if (exprAdd(&right)) {
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(consumedToken, "invalid operand type for relational operator");
            *r = (Ret){createType(TB_INT, -1), 0, 1, {0}};
            return exprRel1(r);
        }
        tkerr(lastToken, "invalid expression after relational operator");
    }
    return 1;
}

int exprAdd(Ret *r) { return exprMul(r) ? exprAdd1(r) : 0; }
int exprAdd1(Ret *r)
{
    if (consume(ADD) || consume(SUB)) {
        Ret right; Type tDst;
        if (exprMul(&right)) {
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(consumedToken, "invalid operand type for + or -");
            *r = (Ret){tDst, 0, 1, {0}};
            return exprAdd1(r);
        }
        tkerr(lastToken, "invalid expression after + or -");
    }
    return 1;
}

int exprMul(Ret *r) { return exprCast(r) ? exprMul1(r) : 0; }
int exprMul1(Ret *r)
{
    if (consume(MUL) || consume(DIV)) {
        Ret right; Type tDst;
        if (exprCast(&right)) {
            if (!arithTypeTo(&r->type, &right.type, &tDst)) tkerr(consumedToken, "invalid operand type for * or /");
            *r = (Ret){tDst, 0, 1, {0}};
            return exprMul1(r);
        }
        tkerr(lastToken, "invalid expression after * or /");
    }
    return 1;
}

int exprCast(Ret *r)
{
    Token *startTk = lastToken;
    Type t;
    Ret op;
    if (consume(LPAR)) {
        if (typeBase(&t)) {
            arrayDecl(&t);
            if (consume(RPAR)) {
                if (exprCast(&op)) {
                    if (t.tb == TB_STRUCT) tkerr(consumedToken, "cannot convert to a struct type");
                    if (op.type.tb == TB_STRUCT) tkerr(consumedToken, "cannot convert a struct");
                    if (op.type.n >= 0 && t.n < 0) tkerr(consumedToken, "an array can be converted only to another array");
                    if (op.type.n < 0 && t.n >= 0) tkerr(consumedToken, "a scalar can be converted only to another scalar");
                    *r = (Ret){t, 0, 1, {0}};
                    return 1;
                }
                tkerr(lastToken, "invalid expression after cast");
            }
            tkerr(lastToken, "missing ) after cast type");
        }
        lastToken = startTk;
    }
    return exprUnary(r);
}

int exprUnary(Ret *r)
{
    if (consume(SUB)) {
        if (exprUnary(r)) {
            if (!canBeScalar(r)) tkerr(consumedToken, "unary - must have a scalar operand");
            r->lval = 0; r->ct = 1;
            return 1;
        }
        tkerr(lastToken, "invalid unary expression");
    }
    if (consume(NOT)) {
        if (exprUnary(r)) {
            if (!canBeScalar(r)) tkerr(consumedToken, "! must have a scalar operand");
            *r = (Ret){createType(TB_INT, -1), 0, 1, {0}};
            return 1;
        }
        tkerr(lastToken, "invalid unary expression");
    }
    return exprPostfix(r);
}

int exprPostfix(Ret *r) { return exprPrimary(r) ? exprPostfix1(r) : 0; }
int exprPostfix1(Ret *r)
{
    if (consume(LBRACKET)) {
        Ret idx; Type tInt = createType(TB_INT, -1);
        if (expr(&idx)) {
            if (consume(RBRACKET)) {
                if (r->type.n < 0) tkerr(consumedToken, "only an array can be indexed");
                if (!convTo(&idx.type, &tInt)) tkerr(consumedToken, "the index is not convertible to int");
                r->type.n = -1;
                r->lval = 1;
                r->ct = 0;
                return exprPostfix1(r);
            }
            tkerr(lastToken, "missing ] after array index");
        }
        tkerr(lastToken, "invalid expression after [");
    }
    if (consume(DOT)) {
        if (consume(ID)) {
            Token *tkName = consumedToken;
            Symbol *field;
            if (r->type.tb != TB_STRUCT) tkerr(tkName, "a field can only be selected from a struct");
            field = findSymbolInList(r->type.s->structMembers, tkName->text);
            if (!field) tkerr(tkName, "the structure %s does not have a field %s", r->type.s->name, tkName->text);
            *r = (Ret){field->type, 1, field->type.n >= 0, {0}};
            return exprPostfix1(r);
        }
        tkerr(lastToken, "missing identifier after .");
    }
    return 1;
}

int exprPrimary(Ret *r)
{
    Token *startTk = lastToken;
    if (consume(ID)) {
        Token *tkName = consumedToken;
        Symbol *s = findSymbol(tkName->text);
        if (!s) tkerr(tkName, "undefined id: %s", tkName->text);
        if (consume(LPAR)) {
            Symbol **param = s->params.begin;
            Ret rArg;
            if (s->kind != SK_FN && s->kind != SK_EXTFN) tkerr(tkName, "only a function can be called");
            if (expr(&rArg)) {
                if (param == s->params.end) tkerr(tkName, "too many arguments in function call");
                if (!convTo(&rArg.type, &(*param)->type)) tkerr(tkName, "in call, cannot convert the argument type to the parameter type");
                param++;
                while (consume(COMMA)) {
                    if (!expr(&rArg)) tkerr(lastToken, "missing expression after ,");
                    if (param == s->params.end) tkerr(tkName, "too many arguments in function call");
                    if (!convTo(&rArg.type, &(*param)->type)) tkerr(tkName, "in call, cannot convert the argument type to the parameter type");
                    param++;
                }
            }
            if (!consume(RPAR)) tkerr(lastToken, "missing ) in function call");
            if (param != s->params.end) tkerr(tkName, "too few arguments in function call");
            *r = (Ret){s->type, 0, 1, {0}};
            return 1;
        }
        if (s->kind == SK_FN || s->kind == SK_EXTFN) tkerr(tkName, "a function can only be called");
        *r = (Ret){s->type, 1, s->type.n >= 0, {0}};
        return 1;
    }
    if (consume(CT_INT)) { *r = (Ret){createType(TB_INT, -1), 0, 1, {.i = consumedToken->i}}; return 1; }
    if (consume(CT_REAL)) { *r = (Ret){createType(TB_DOUBLE, -1), 0, 1, {.d = consumedToken->r}}; return 1; }
    if (consume(CT_CHAR)) { *r = (Ret){createType(TB_CHAR, -1), 0, 1, {.i = consumedToken->i}}; return 1; }
    if (consume(CT_STRING)) { *r = (Ret){createType(TB_CHAR, 0), 0, 1, {.str = consumedToken->text}}; return 1; }
    if (consume(LPAR)) {
        if (expr(r)) {
            if (consume(RPAR)) return 1;
            tkerr(lastToken, "missing ) in expression");
        }
        lastToken = startTk;
    }
    return 0;
}

int unit(void)
{
    while (1) {
        if (structDef()) {}
        else if (fnDef()) {}
        else if (varDef()) {}
        else break;
    }
    if (!consume(END)) tkerr(lastToken, "expected struct/function/variable definition or end of file");
    return 1;
}

static void freeTokens(Token *head)
{
    Token *tk = head;
    while (tk) {
        Token *next = tk->next;
        if ((tk->code == ID || tk->code == CT_STRING) && tk->text) free(tk->text);
        free(tk);
        tk = next;
    }
}

static char *loadFileText(const char *path)
{
    FILE *f = fopen(path, "rb");
    long size;
    size_t nRead;
    char *buf;
    if (f == NULL) err("cannot open `%s`", path);
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); err("cannot seek `%s`", path); }
    size = ftell(f);
    if (size < 0) { fclose(f); err("cannot get size for `%s`", path); }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); err("cannot seek `%s`", path); }
    buf = (char *)malloc((size_t)size + 1);
    if (buf == NULL) { fclose(f); err("not enough memory"); }
    nRead = fread(buf, 1, (size_t)size, f);
    if (ferror(f)) { free(buf); fclose(f); err("cannot read `%s`", path); }
    buf[nRead] = '\0';
    fclose(f);
    return buf;
}

int main(int argc, char **argv)
{
    char *ownedInput = NULL;
    if (argc > 1) {
        ownedInput = loadFileText(argv[1]);
        pCrtCh = ownedInput;
    } else {
        err("usage: AtomC <input_file>");
    }

    tokens = NULL;
    lastToken = NULL;
    line = 1;
    while (getNextToken() != END) {}

    showTokens();

    initSymbols(&symTable);
    addExtFuncs();
    lastToken = tokens;
    unit();
    printf("\nsyntax + domain + type OK\n");
    showSymbols();

    freeTokens(tokens);
    free(ownedInput);
    return 0;
}
