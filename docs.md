### Language syntax

**Each program should have a main function**, which is the entry point of the program.

```
f main() -> null:
    do_something()
    return null
```

Function example:

```
f some_func(a: u8, b: str) -> null:
    print(b)
    u8 second_operand = u8(2)
    print(a * second_operand)
    return null
```


Fibonacci example:

```
f fib(n: int) -> null:
    int a = 0
    int b = 1
    while a < n:
        print("%d\n" % a)
        a = b
        b = a + b
    return null

fib(1000)
```

### Types

#### Primitive types

| Type | Description |
| --- | --- |
| u8 | Unsigned 8-bit integer |
| u16 | Unsigned 16-bit integer |
| u32 | Unsigned 32-bit integer |
| u64 | Unsigned 64-bit integer |
| i8 | Signed 8-bit integer |
| i16 | Signed 16-bit integer |
| i32 | Signed 32-bit integer |
| i64 | Signed 64-bit integer |
| f32 | 32-bit floating point number |
| f64 | 64-bit floating point number |
| str | String |
| bool | Boolean |
| null | Null value |
| error | Error value |

#### Compound types

| Type | Description |
| --- | --- |
| array | Array |
| list | List |
| map | Map |
| tuple | Tuple |

#### Type aliases

| Type | Description   |
| --- |---------------|
| int | Alias for i32 |
| float | Alias for f32 |
| double | Alias for f64 |

### Operators

| Operator | Description |
| --- | --- |
| `+` | Addition |
| `-` | Subtraction |
| `*` | Multiplication |
| `/` | Division |
| `%` | Modulo |
| `++` | Increment |
| `--` | Decrement |
| `==` | Equal |
| `!=` | Not equal |
| `>` | Greater than |
| `<` | Less than |
| `>=` | Greater than or equal |
| `<=` | Less than or equal |
| `&&` | Logical and |
| `||` | Logical or |
| `!` | Logical not |
| `&` | Bitwise and |
| `\|` | Bitwise or |
| `^` | Bitwise xor |
| `<<` | Bitwise shift left |
| `>>` | Bitwise shift right |
| `~` | Bitwise not |

### Functions

Functions are defined using the `f` keyword.

```
f function_name(parameter1: type1, parameter2: type2) -> return_type:
    // function body
    return null
```

The return type can be `null` if the function does not return a value.

Functions also can return multiple values:

```
f function_name(parameter1: type1, parameter2: type2) -> (return_type1, return_type2):
    // function body
    return (value1, value2)
```

### Control flow

#### If statement

```
if condition:
    // code to execute if condition is true
elsif condition2:
    // code to execute if condition2 is true
else:
    // code to execute if condition is false
```

#### While loop

```
while condition:
    // code to execute while condition is true
```

#### For loop

```
for i = range(start, end, step):
    // code to execute for each value of i
```

#### Break and continue

```
while condition:
    if condition:
        break
    else:
        continue
```

#### Return

```
return value
```

#### Error handling

```
error("Error message, with %d and %s" % 1, "string")
```

Example of a function that returns an error:

```
f div(a: int, b: int) -> (int, error):
    if b == 0:
        return (0, error("Division by zero"))
    return (a / b, null)
```
