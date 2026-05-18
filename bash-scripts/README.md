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

