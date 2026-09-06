# STYLE GUIDE!

This project loosely (emphasis on loosely) follows the DOOM 3 code styling guidelines, which are as such:

## ENUMS!

- Stuff defined in enums must be UPPERCASE, using snake_case.
- Use explicitly defined numbers only when necessary, like defining negatives.
- Enums used as types must be in lower pascalCase prefixed by e and suffixed by _t, and use typedef enum.
- Enums defined using the enum keyword must start with an uppercase E and be in PascalCase.
- Always prefix all definitions with something identifiable and unique, for example R_, NET_ or SYS_.

### Correct examples:
typedef:
```
typedef enum {
    HUH_NUH_UH = -1,
    HUH_YUH_UH,
    HUH_GUH_UH,
} eHuhTypes_t;
```
(starts at -1, common with result enums)

normal enum:
```
enum EHuhTypes {
    HUH_NUH_UH,
    HUH_YUH_UH,
    HUH_GUH_UH,
};
```
(notice the lack of = -1, this starts at 0)


## VARIABLES!

- All variables should use camelCase.
- ALWAYS INITIALIZE THEM!
- Please use small type variables when applicable, like uint8_t.
- Use atomics when variables are shared across threads.
- Pointer asterisk goes after the TYPE, *not* before the name.
- Prefix global variables with g_ if you use them, but... don't use them.

### Correct examples:
```
uint8_t variableThatOnlyNeedsToGoUnder255 = 0;
uint8_t* pointerToVariableThatOnlyNeedsToGoUnder255 = &variableThatOnlyNeedsToGoUnder255;
```

```
struct Stuff thingsBro = {
    .stuff = 2,
    .things = 0,
};
```

## FUNCTIONS!

- Use snake_case please.
- Use static if they don't need to be shared.
- Use func_result_t return type from misc.h for errors and stuff.

### Correct examples:
```
func_result_t check_if_over() 
{
    // it's always over
    if( true )
        return R_A_OK;
        
    // if it's not over, something is seriously wrong, so error!
    return R_ERROR;
}
```

## PARENTHESES N' BRACES!

- Parentheses must always have spaces before and after! Like the Valve if( x ) or if ( x ).
- Newline before curly brace only in functions, otherwise same line.
- Don't use braces on single line if statements.

### Correct examples:
```
void destroy_cpu()
{
    while( 1 ) {
        for( int i = 0; i < 2147483648; ++i ) {
            cos(3837277282);
            cos(3837277282);
            cos(3837277282);
        }
    }
}
```

## STRUCTS!

- Similar to enums, but use S instead of E as the prefix.

### Correct egg samples:
```
struct SCoolData {
    int scaryMonsters;
    int niceSprites;
};
```

```
typedef struct {
    int scaryMonsters;
    int niceSprites;
} sCoolData_t;
```