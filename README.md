# parseutils

Utility for parse strings.

```py:
import parseutils as pu 

src = '[section]'
i, section_name = pu.parse_section(0, src, len(src))
print(section_name)
# section

src = 'key = value'
i, key, val = pu.parse_key_value(0, src, len(src))
print(key, val)
# key value

src = '{ "abc": 123, "def": 3.14 }'
i, d = pu.parse_dict(0, src, len(src))
print(d)
# {'abc': 123, 'def': 3.14}

src = '[111, "222", 3.14]'
i, lis = pu.parse_list(0, src, len(src))
print(lis) 
# [111, '222', 3.14]

src = '123,"abc def",323'
i, row = pu.parse_csv_line(0, src, len(src), ',')
print(row)
# [123, 'abc def', 323]

```

## Install

```
python setup.py build
python setup.py install
```

## License

MIT
