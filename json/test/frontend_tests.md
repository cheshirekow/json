# Frontend Tests

<style type="text/css">
body {
  background-color: #1e1e1e;
  color: #d4d4d4;
}
div.content {
  white-space: pre;
  font-family: 'Courier New', Courier, monospace;
}
span.COMMENT {
  color: darkgrey;
}
span.BOOLEAN_LITERAL, span.NULL_LITERAL {
  color: violet;
  font-weight: bold;
}
span.NUMERIC_LITERAL {
  color: lightblue;
  font-weight: bold;
}
span:not(.OBJECT_KEY) > span.STRING_LITERAL {
  color: lightgreen;
}
</style>


## Simple Test

### Test Case:
```json
{
  "foo": {
    "bar": 1.0,
    "baz": "hello world!"
  }
}
```

### Expect Lex:
```
  0: [   PUNCTUATION](0:0) '{'
  1: [    WHITESPACE](0:1) '
  '
  2: [STRING_LITERAL](1:2) '"foo"'
  3: [   PUNCTUATION](1:7) ':'
  4: [    WHITESPACE](1:8) ' '
  5: [   PUNCTUATION](1:9) '{'
  6: [    WHITESPACE](1:10) '
    '
  7: [STRING_LITERAL](2:4) '"bar"'
  8: [   PUNCTUATION](2:9) ':'
  9: [    WHITESPACE](2:10) ' '
 10: [NUMERIC_LITERAL](2:11) '1.0'
 11: [   PUNCTUATION](2:14) ','
 12: [    WHITESPACE](2:15) '
    '
 13: [STRING_LITERAL](3:4) '"baz"'
 14: [   PUNCTUATION](3:9) ':'
 15: [    WHITESPACE](3:10) ' '
 16: [STRING_LITERAL](3:11) '"hello world!"'
 17: [    WHITESPACE](3:25) '
  '
 18: [   PUNCTUATION](4:2) '}'
 19: [    WHITESPACE](4:3) '
'
 20: [   PUNCTUATION](5:0) '}'
 21: [    WHITESPACE](5:1) '
'
```

### Expect Parse:
```
  0: [ OBJECT_BEGIN] '{'
  1: [   OBJECT_KEY] '"foo"'
  2: [ OBJECT_BEGIN] '{'
  3: [   OBJECT_KEY] '"bar"'
  4: [VALUE_LITERAL] '1.0'
  5: [   OBJECT_KEY] '"baz"'
  6: [VALUE_LITERAL] '"hello world!"'
  7: [   OBJECT_END] '}'
  8: [   OBJECT_END] '}'
```

### Expect Markup:

<div class="expect_markup">
<span class="OBJECT_BEGIN"><span class="PUNCTUATION">{</span><span class="WHITESPACE">
  </span><span class="OBJECT_KEY"><span class="STRING_LITERAL">"foo"</span></span><span class="PUNCTUATION">:</span><span class="WHITESPACE"> </span><span class="OBJECT_BEGIN"><span class="PUNCTUATION">{</span><span class="WHITESPACE">
    </span><span class="OBJECT_KEY"><span class="STRING_LITERAL">"bar"</span></span><span class="PUNCTUATION">:</span><span class="WHITESPACE"> </span><span class="VALUE_LITERAL"><span class="NUMERIC_LITERAL">1.0</span></span><span class="PUNCTUATION">,</span><span class="WHITESPACE">
    </span><span class="OBJECT_KEY"><span class="STRING_LITERAL">"baz"</span></span><span class="PUNCTUATION">:</span><span class="WHITESPACE"> </span><span class="VALUE_LITERAL"><span class="STRING_LITERAL">"hello world!"</span></span><span class="WHITESPACE">
  </span><span class="PUNCTUATION">}</span></span><span class="WHITESPACE">
</span><span class="PUNCTUATION">}</span></span><span class="WHITESPACE">
</span>
</div>


## Test Token Types

### Test Case:

```json
["string", 1.0, true, null]
```

### Expect Lex:

```txt
  0: [   PUNCTUATION](0:0) '['
  1: [STRING_LITERAL](0:1) '"string"'
  2: [   PUNCTUATION](0:9) ','
  3: [    WHITESPACE](0:10) ' '
  4: [NUMERIC_LITERAL](0:11) '1.0'
  5: [   PUNCTUATION](0:14) ','
  6: [    WHITESPACE](0:15) ' '
  7: [BOOLEAN_LITERAL](0:16) 'true'
  8: [   PUNCTUATION](0:20) ','
  9: [    WHITESPACE](0:21) ' '
 10: [  NULL_LITERAL](0:22) 'null'
 11: [   PUNCTUATION](0:26) ']'
 12: [    WHITESPACE](0:27) '
'
```

### Expect Parse:

```txt
  0: [   LIST_BEGIN] '['
  1: [VALUE_LITERAL] '"string"'
  2: [VALUE_LITERAL] '1.0'
  3: [VALUE_LITERAL] 'true'
  4: [VALUE_LITERAL] 'null'
  5: [     LIST_END] ']'
```

### Expect Markup:

<div class="expect_markup">
<span class="LIST_BEGIN"><span class="PUNCTUATION">[</span><span class="VALUE_LITERAL"><span class="STRING_LITERAL">"string"</span></span><span class="PUNCTUATION">,</span><span class="WHITESPACE"> </span><span class="VALUE_LITERAL"><span class="NUMERIC_LITERAL">1.0</span></span><span class="PUNCTUATION">,</span><span class="WHITESPACE"> </span><span class="VALUE_LITERAL"><span class="BOOLEAN_LITERAL">true</span></span><span class="PUNCTUATION">,</span><span class="WHITESPACE"> </span><span class="VALUE_LITERAL"><span class="NULL_LITERAL">null</span></span><span class="PUNCTUATION">]</span></span><span class="WHITESPACE">
</span>
</div>
