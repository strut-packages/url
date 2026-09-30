include <url>;

function main() -> int : UrlError {
    Url base := url.parse("https://example.com/a/b?q=1#top");
    Url resolved := url.resolve(base, "../other");
    print(url.stringify(resolved));
    print(url.encode("a b"));
    print(url.decode("a%20b"));
    return 0;
}
