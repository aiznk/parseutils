# parseutils

Utility for parse strings.

```py:
import parseutils as pu 

src = '[111, "222", 3.14]'
i, lis = pu.parse_list(0, src, len(src))
print(lis)  # [111, '222', 3.14]
```

## License

MIT
