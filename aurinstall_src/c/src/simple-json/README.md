# simple-json
An incredibly simple JSON parser written in C.

To use the parser, include the "simple_json.h" header file.<br>
There are two functions, `json_parse_dict` and `json_parse_arr`.<br>
`json_parse_dict` takes in a JSON string, and the key of the item to return.<br>
`json_parse_arr` takes in a JSON string and the index of the array item to return.

These functions do not return a JSON object, but rather a string of the retreived item.<br>
If you wish to parse that string further, just call the function(s) again on that string.

the `test/` folder includes some examples and a `test.c` file which will
show memory usage, time taken, and will parse two example JSON files.
