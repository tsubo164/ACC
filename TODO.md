* TODO
  - include
  - define
  - struct arg
  - struct assign
  - check args
  - cast
  - char <-> int, negative number literal
  - char literal
  - static, extern
  - const
  - array initializer
  - std stuff FILE, stderro, NULL ...
  - variable arguments

* OPTIONAL
  - use registers r10 r11
  - function prototype
  - unsigned
  - short, long
  - bit shift <<, >>
  - bitwise op |, &, ~, ^
  - anonymous enum, struct, union
  - union
  - control flow check
  - void pointer, void parameter

* IMPLEMENTATION
  - new number()
  - push symbol()
  - use label() and enum scope

* BUG
  - void printf(char *s, ind n) -> infinite loop
  - //typedef struct node Node;
    struct node {
        int id;
        Node *next;
    };
    -> infinite loop

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
