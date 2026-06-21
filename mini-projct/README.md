# Canonical LR(1) Parser — Compiler Mini-Project (COMP 409)

A canonical LR(1) parser, in C++, for the augmented grammar

```
0.  S' -> S
1.  S  -> L = R
2.  S  -> R
3.  L  -> * R
4.  L  -> id
5.  R  -> L
```

The program builds the FIRST sets, the canonical collection of sets of LR(1)
items (14 states), the goto automaton, and the ACTION/GOTO parsing table; checks
the table for conflicts (there are none); and traces input strings step by step.

## Layout

```
mini-projct/
├── src/
│   └── canonical_lr_parser.cpp   # full implementation
└── report/
    ├── report.tex                # LaTeX source
    └── report.pdf                # compiled report
```

## Build & run the parser

```
 g++ -std=c++17 -O2 src/canonical_lr_parser.cpp -o clr
./clr
```

It prints the grammar, FIRST sets, all 14 LR(1) states, the parsing table, the
conflict check, and the parse traces for `id = *id` (accepted) and
`id = id * id` (rejected).


## Note on the test string

In this grammar `*` is a **prefix** dereference operator (`L -> * R`, modelling
an l-value such as `*p`), so the well-formed reading of the requested string
`id = id * id` is `id = *id`. The parser accepts `id = *id` and correctly
rejects the literal `id = id * id`, which also demonstrates its error detection.
The interesting result is that this grammar is **not SLR(1)** (it has a
shift/reduce conflict in state I2) yet is **conflict-free under canonical
LR(1)** — see Section 2.7 / 4.5 of the report.
