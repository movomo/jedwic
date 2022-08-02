# jedwic

JSON Encoder/Decoder Written In C (for study purposes)


## Description

In search of answer to the great mystery that is compiler,
I decided to learn how they actually work.
This is the first step.

I also realized that writing a decoder, even if it's just a humple JSON,
can be harder than writing some simple cli programs.
So I decided to write a JSON decoder in C,
because it's the language I'm trying to familiarize myself with.


## Data structure

Currently, the `json.h` file contains public headers and declarations,
as well as things that shouldn't really be there, such as individual
data containers' structs.

I kind of regret it but also think it's kinda nice to have direct access to
struct members because,
having to call a function just to know an array's length feels sucky.

```c
typedef enum JsonValueType {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonValueType;

typedef struct JsonValue JsonValue;
struct JsonValue {
    JsonValueType type;
    union _JsonValue {
        bool as_bool;
        JsonNumber as_num;
        char *as_str;
        JsonArray *as_arr;
        JsonObject *as_obj;
    } value;
};
```

For encoding/decoding, arraylist(named `JsonArray`)
and hashtable(named `JsonObject`) are available.

All numbers are converted to `double` because integer or float doesn't make
difference in Javascript.

`JSON_NULL` is mentioned in the enum but is not present in the `JsonValue`
struct, because it's supposed to mean absence of value.

They all need to be wrapped in `JsonValue`.
The abovementioned containers don't accept void pointers.

### JsonArray

This is a self-resizing array list with growing factor of 1.5.
It also shrinks somewhat timidly.

### JsonObject

It's a simple hash table with [FNV-1a][1] hash and linked list buckets.

The initial capacity is 7 and gradually grows when 2/3 of capacity is used.
The next capacity is chosen among the hard-coded prime table.

Liks `JsonArray`, this also can shrink conservatively.

[1]: http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-source
