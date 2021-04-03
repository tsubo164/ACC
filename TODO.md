* SELF COMPILE
  - [ ] `ast.c`
  - [ ] `gen_x86.c`
  - [ ] `lexer.c`
  - [ ] `main.c`
  - [ ] `message.c`
  - [ ] `parse.c`
  - [ ] `preprocessor.c`
  - [ ] `semantics.c`
  - [x] `string_table.c`
  - [ ] `symbol.c`
  - [ ] `type.c`

* TODO
  - struct arg
  - struct assign
  - variable arguments

* OPTIONAL
  - char[] from string literal
  - simplify ast
    - remove `NOD_SPEC_*` from tree
    - remove `NOD_DECL_*` from tree
    - add `struct declaration` to manage decl context and use it as parse param
  - unversal type string for better messages
  - set `is_used` in parser => remove `check_symbol_usage()` from semantics
  - use registers r10 r11
  - bit shift <<, >>
  - bitwise op |, &, ~, ^
  - union
  - control flow check
  - void pointer, void parameter
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
  - void printf(char *s, ind n) -> infinite loop
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
