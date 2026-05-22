# Bash Scripts

## How the shell parses strings

### About quote

| Quoting Style | Variable Expansion | Word Splitting | Pathname Expansion(Globbing) |
|---------------|--------------------|----------------|------------------------------|
| no quote      | ✅                 | ✅             | ✅                           |
| double quote  | ✅                 | ❌             | ❌                           |
| single quote  | ❌                 | ❌             | ❌                           |


### Unquoted: The most dangerous form

```bash
n="hello world"
printf '%s\n' $n
```

Many beginners think this becomes `printf '%s\n' "hello world"`, but it actually becomes
`printf '%s\n' hello world`. This happens because the shell performs variable expansion
followed by word splitting.

``` bash
x="*"
printf '%s\n' $x
```

If the current directory contains:
```text
a.txt
b.txt
main.c
```
then `*` is expanded by pathname expansion (globbing), so the command becomes:

```bash
printf '%s\n' a.txt b.txt main.c
```

### Double quote: Preserve the value as one argument

```bash
x="hello world"
printf '%s\n' "$x"
```

Variable expansion still happens, but no word splitting and no globbing, so the output is `hello world`
Likewise:
```bash
x="*"
printf '%s\n' "$x"
```
Output is `*`

### Single quote: completely literal
```bash
x="hello world"
printf '%s\n' '$x'
```
Output is `$x`

### Arithmetic expansion

```bash
$(( expression ))
```

#### Why `$` can be omitted

Because `$(( ... ))` already creates an arithmetic context.
Bash automatically interprets tokens as variables. For example,

```bash
x=10
y=20
echo $(( x + y ))
```

is equivalent to

```bash
echo $(( $x + $y ))
```

but the first form is cleaner

#### Assignment

```bash
x=5
echo $(( x = x + 1 ))
echo "$x"
```

#### Increment/Decrement

```bash
((x++))
((x--))
((++x))
((--x))
```
Example:

```bash
x=5
echo $((x++))
echo "$x"
```

Output:

```text
5
6
```

Because:
- `x++` is post-incremnt
- It returns the old value first
- Then increments by 1

#### `(( ... ))` vs `$(( ... ))`

These two are easy to confuse

`$(( ... ))` is **arithmetic expansion**, which is used to produce a value

```bash
result=$((1 + 2))
echo "$result"
```

`(( ... ))` is **arithmetic command**, which is used for arithmetic operations or conditions

```bash
((x = 5 + 3))
echo "$x"
```

#### Exit Status

``(( ... )) behaves like a shell command and has an exit status
- result ≠ 0 → success
- result = 0 → failure

#### Integer Only

```bash
echo $((3 / 2))
```

Output:

```text
1
```

not `1.5`

## Test expressions

Bash has three forms of test expressions. They look similar but have important differences.

| Form    | Type               | Use for                        |
|---------|--------------------|--------------------------------|
| `[ ]`   | POSIX command      | portable string/file tests     |
| `[[ ]]` | bash keyword       | string tests, pattern matching |
| `(( ))` | arithmetic command | numeric comparisons            |

### `[ ]` — POSIX test command

`[ ]` is actually the `test` command. It is portable (works in `sh`) but has gotchas.

**String tests:**

| Expression         | True when             |
|--------------------|-----------------------|
| `[ -z "$s" ]`      | `$s` is empty         |
| `[ -n "$s" ]`      | `$s` is non-empty     |
| `[ "$a" = "$b" ]`  | strings are equal     |
| `[ "$a" != "$b" ]` | strings are not equal |

**Numeric tests:**

| Expression          | Meaning               |
|---------------------|-----------------------|
| `[ "$a" -eq "$b" ]` | equal                 |
| `[ "$a" -ne "$b" ]` | not equal             |
| `[ "$a" -lt "$b" ]` | less than             |
| `[ "$a" -le "$b" ]` | less than or equal    |
| `[ "$a" -gt "$b" ]` | greater than          |
| `[ "$a" -ge "$b" ]` | greater than or equal |

**File tests:**

| Expression    | True when           |
|---------------|---------------------|
| `[ -e "$f" ]` | file exists         |
| `[ -f "$f" ]` | regular file exists |
| `[ -d "$f" ]` | directory exists    |
| `[ -r "$f" ]` | file is readable    |
| `[ -w "$f" ]` | file is writable    |
| `[ -x "$f" ]` | file is executable  |

**The quoting trap:**

Always quote variables inside `[ ]`. Without quotes, word splitting can break the test:

```bash
name=""
[ $name = "" ]   # becomes: [ = "" ] — syntax error!
[ "$name" = "" ] # correct
```

**Logical operators:** use `-a` (and) and `-o` (or):

```bash
[ "$a" -gt 0 -a "$a" -lt 10 ]
```

But `-a` and `-o` are deprecated and error-prone. Prefer combining two `[ ]` with `&&` and `||`:

```bash
[ "$a" -gt 0 ] && [ "$a" -lt 10 ]
```

### `[[ ]]` — bash keyword

`[[ ]]` is a bash built-in keyword, not a command. It avoids most of the quoting traps of `[ ]` and adds pattern matching.

**No word splitting inside `[[ ]]`:**

```bash
name=""
[[ $name == "" ]] # safe even without quotes
```

**Logical operators:** use `&&` and `||` directly inside:

```bash
[[ "$a" -gt 0 && "$a" -lt 10 ]]
```

**Pattern matching with `==`:**

```bash
file="report_2024.txt"
[[ "$file" == *.txt ]]  # true: glob pattern on right side
```

Note: the right side of `==` is a glob pattern, not a string. Do not quote it if you want pattern matching — quoting makes it a literal string.

**Regex matching with `=~`:**

```bash
input="abc123"
[[ "$input" =~ ^[a-z]+[0-9]+$ ]]  # true
```

The regex is unquoted on the right side. Capture groups are available in `BASH_REMATCH`:

```bash
date="2024-05-19"
[[ "$date" =~ ^([0-9]{4})-([0-9]{2})-([0-9]{2})$ ]]
echo "${BASH_REMATCH[1]}"  # 2024
echo "${BASH_REMATCH[2]}"  # 05
echo "${BASH_REMATCH[3]}"  # 19
```

### `(( ))` — arithmetic command for comparisons

For numeric comparisons, `(( ))` is cleaner than `[ ]` because it uses familiar math operators:

```bash
a=5
if (( a > 3 )); then
    echo "greater"
fi
```

| `[ ]` / `[[ ]]` | `(( ))` |
|------------------|---------|
| `-eq` | `==` |
| `-ne` | `!=` |
| `-lt` | `<` |
| `-le` | `<=` |
| `-gt` | `>` |
| `-ge` | `>=` |

### Choosing the right form

- Use `[[ ]]` for string and file tests in bash scripts — it is safer and more expressive than `[ ]`
- Use `(( ))` for numeric comparisons
- Use `[ ]` only when you need `sh` portability

