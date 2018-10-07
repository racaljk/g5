# notations
```ebnf
Production  = production_name "=" [ Expression ] "." .
Expression  = Alternative { "|" Alternative } .
Alternative = Term { Term } .
Term        = production_name | token [ "…" token ] | Group | Option | Repetition .
Group       = "(" Expression ")" .
Option      = "[" Expression "]" .
Repetition  = "{" Expression "}" .
Productions are expressions constructed from terms and the following operators, in increasing precedence:

|   alternation
()  grouping
[]  option (0 or 1 times)
{}  repetition (0 to n times)
```