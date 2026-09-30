# url

Official RFC 3986 URI-reference parsing and resolution for Strut.

This package implements generic RFC 3986 semantics. It does not claim WHATWG URL compatibility.

## API

```strut
function url_parse(string text) -> Url : UrlError;
function url_stringify(Url value) -> string;
function url_resolve(Url base, string reference) -> Url : UrlError;
function url_resolve_string(string base, string reference) -> string : UrlError;
function url_percent_encode(bytes input) -> string;
function url_percent_decode(string text) -> bytes : UrlError;
function url_encode(string text) -> string;
function url_decode(string text) -> string : UrlError;
function url_parse_query(string query) -> UrlQueryParam[] : UrlError;
function url_build_query(UrlQueryParam[] parameters) -> string;
```

## Usage

```strut
include <url>;

function main() -> int : UrlError {
    Url base := url_parse("https://example.com/a/b?q=1#top");
    print(url_stringify(url_resolve(base, "../other")));
    print(url_encode("a b"));
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
