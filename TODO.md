* SELF COMPILE
  - [x] `ast.c`
  - [ ] `gen_x86.c`
  - [x] `lexer.c`
  - [ ] `main.c`
  - [x] `message.c`
  - [ ] `parse.c`
  - [x] `preprocessor.c`
  - [x] `semantics.c`
  - [x] `string_table.c`
  - [x] `symbol.c`
  - [x] `type.c`

* TODO

* OPTIONAL
  - init global var with const expr that has address of global variables;
    - `int *p = ptr + 1 // legal if global`
    - `int i = j + 1 // illegal even if global`
    - `char s[] = "abc"` is not supported
    - `char *s = "abc"` is not supported
    - `struct Point p = q` is not supported
  - simplify ast
    - remove `NOD_SPEC_*` from tree
    - remove `NOD_DECL_*` from tree
    - add `struct declaration` to manage decl context and use it as parse param
  - `data_tag_()` needs to be based on size of type
  - `code[123]__()` needs to take `data_type` or tag instead of `ast_node`
  - unversal type string for better messages
  - set `is_used` in parser => remove `check_symbol_usage()` from semantics
  - use registers r10 r11
  - bitwise op |, &, ~, ^
  - union
  - control flow check
  - lexer error

* IMPLEMENTATION

* BUG
  - //typedef struct node Node;
    struct node {
        int id;
        Node *next;
    };
    -> infinite loop
  - { ...  } } -> infinite loop
  - int foo(); { ...  } -> infinite loop
  - `return g_a[2]; int i = g_a[0];` -> infinite loop

* DONE
  - ++, --
  - for
  - do while
  - += ...
  - break, continue (need error)
  - logical op &&, ||, !
  - define symbol() takes type args
  - remove const cast
  - return check
  - void
  - switch case
  - goto
  - duplicate label for string
  - sizeof unary expression
  - sizeof (typename)
  - conditional op (?:)
  - find struct() - improve '.'
  - -> op
  - incomplete struct
  - struct usage
  - error with pos
  - typedef
  - pointer ++, --
  - push symbol()
  - use label() and enum scope
  - enum, struct with no tag
  - char literal
  - static, extern
  - add func name to static int (added symbol id)
  - const
  - new number() (not needed)
  - array initializer
  - struct initializer
  - not an error for 'sizeof typename'
  - void `printf(char *s, ind n)` -> infinite loop
  - static int -> quad
  - include
  - function prototype and definition in the same source
  - function prototype
  - define
  - unsigned
  - short, long
  - cast
  - % operator
  - check args
  - initializer index, enum index, invisible nodes in parser
  - symbol functions for struct 
  - return type check
  - char <-> int, negative number literal
  - improve `is_compatible` e.g. ptr = 0, ptr = array, ...
  - std stuff FILE, stderro, NULL ...
  - pp needs to adjust pos x when skipping block comment
  - char[] from string literal
  - convert from pointer to int e.g. if (node)
  - void pointer, void parameter
  - typedef'ed type produces actual type => actual type holds link to typedef sym
  - variable arguments
  - struct assign
  - struct arg
  - struct return
  - bit shift <<, >>
