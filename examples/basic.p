include <url>;

function main() -> int : UrlError {
    Url base := url_parse("https://example.com/a/b?q=1#top");
    Url resolved := url_resolve(base, "../other");
    print(url_stringify(resolved));
    print(url_encode("a b"));
    print(url_decode("a%20b"));
    return 0;
}
