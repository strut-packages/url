# url

Official RFC 3986 URI-reference parsing and resolution for Strut.

This package implements generic RFC 3986 semantics. It does not claim WHATWG URL compatibility.

## API

```strut
value := url.parse(text);
text := url.stringify(value);
resolved := url.resolve(value, reference);
text := url.resolve_string(base, reference);
encoded := url.percent_encode(input_bytes);
decoded := url.percent_decode(text);
encoded := url.encode(text);
decoded := url.decode(text);
parameters := url.parse_query(query);
query := url.build_query(parameters);
```

## Usage

```strut
include <url>;

function main() -> int : UrlError {
    Url base := url.parse("https://example.com/a/b?q=1#top");
    print(url.stringify(url.resolve(base, "../other")));
    print(url.encode("a b"));
    return 0;
}
```

## v0.1 contract

- `Url` preserves raw encoded scheme, authority, path, query, and fragment components.
- Nullable fields distinguish an absent component from a present empty component.
- Authority parsing exposes userinfo, hostname, optional port, and bracketed IP-literal status while retaining the original authority.
- Resolution follows RFC 3986 section 5.2 and removes literal dot segments without decoding `%2e`.
- Percent escapes are strict and emit uppercase hexadecimal.
- Generic encoding leaves only RFC unreserved bytes unchanged.
- Query parsing preserves order, duplicates, missing `=`, empty values, and literal `+`.
- Raw non-ASCII URI bytes are rejected; percent encoding operates on exact string code units and does not perform Unicode normalization.

The package does not implement WHATWG parsing, form encoding, `+`-as-space conversion, IDNA, IRI conversion, DNS validation, scheme-specific defaults, or URI canonicalization. Bracketed IP literals are structurally recognized and preserved; v0.1 does not fully validate IPv6 or IPvFuture address grammar.

## Test

```sh
python3 tests/run.py --strut /path/to/strut
```
