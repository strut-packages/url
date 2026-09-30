include <url>;

function main() -> int : UrlError {
    Url parsed := url_parse("https://user:pass@example.com:8443/a/b?q=1#top");
    print(parsed.scheme ?? "absent");
    print(parsed.authority ?? "absent");
    print(parsed.userinfo ?? "absent");
    print(parsed.hostname ?? "absent");
    print(parsed.port ?? "absent");
    print(parsed.path);
    print(parsed.query ?? "absent");
    print(parsed.fragment ?? "absent");
    print(url_stringify(parsed));

    Url ipv6 := url_parse("http://[2001:db8::1]:8080/");
    print(ipv6.hostname ?? "absent");
    print(ipv6.host_is_ip_literal);
    print(ipv6.port ?? "absent");
    Url ipv4 := url_parse("http://192.0.2.1/");
    print(ipv4.hostname ?? "absent");

    Url relative := url_parse("../other?");
    print(relative.scheme == null);
    print(relative.authority == null);
    print(relative.path);
    print((relative.query ?? "missing") == "");
    print(relative.fragment == null);

    print(url_resolve_string("http://a/b/c/d;p?q", "g"));
    print(url_resolve_string("http://a/b/c/d;p?q", "../g"));
    print(url_resolve_string("http://a/b/c/d;p?q", "?y"));
    print(url_resolve_string("http://a/b/c/d;p?q", "#s"));
    print(url_resolve_string("http://a/b/c/d;p?q", "./g/."));

    print(url_encode("a b/+"));
    print(url_decode("a%20b%2F%2B"));
    try {
        url_decode("bad%2");
        print("missing error");
    } catch (UrlError err) {
        print(err.code);
    }

    UrlQueryParam[] parameters := url_parse_query("a=1&b=&flag&plus=a+b&a=2");
    print(parameters.length());
    for (parameter : parameters) {
        print(parameter.name);
        print(parameter.value ?? "absent");
    }
    print(url_build_query(parameters));

    try {
        url_parse("http://[2001:db8::1/path");
        print("missing error");
    } catch (UrlError err) {
        print(err.code);
    }
    try {
        url_parse("http://example.com/a path");
        print("missing error");
    } catch (UrlError err) {
        print(err.code);
    }
    try {
        url_parse("http://user@name@example.com/p");
        print("missing error");
    } catch (UrlError err) {
        print(err.code);
    }
    try {
        url_parse("http://example[.com/p");
        print("missing error");
    } catch (UrlError err) {
        print(err.code);
    }

    string resolution_base := "http://a/b/c/d;p?q";
    print(url_resolve_string(resolution_base, "g:h") == "g:h");
    print(url_resolve_string(resolution_base, "./g") == "http://a/b/c/g");
    print(url_resolve_string(resolution_base, "g/") == "http://a/b/c/g/");
    print(url_resolve_string(resolution_base, "/g") == "http://a/g");
    print(url_resolve_string(resolution_base, "//g") == "http://g");
    print(url_resolve_string(resolution_base, "g?y") == "http://a/b/c/g?y");
    print(url_resolve_string(resolution_base, "g#s") == "http://a/b/c/g#s");
    print(url_resolve_string(resolution_base, "g?y#s") == "http://a/b/c/g?y#s");
    print(url_resolve_string(resolution_base, ";x") == "http://a/b/c/;x");
    print(url_resolve_string(resolution_base, "g;x") == "http://a/b/c/g;x");
    print(url_resolve_string(resolution_base, "g;x?y#s") == "http://a/b/c/g;x?y#s");
    print(url_resolve_string(resolution_base, "") == "http://a/b/c/d;p?q");
    print(url_resolve_string(resolution_base, ".") == "http://a/b/c/");
    print(url_resolve_string(resolution_base, "./") == "http://a/b/c/");
    print(url_resolve_string(resolution_base, "..") == "http://a/b/");
    print(url_resolve_string(resolution_base, "../") == "http://a/b/");
    print(url_resolve_string(resolution_base, "../g") == "http://a/b/g");
    print(url_resolve_string(resolution_base, "../..") == "http://a/");
    print(url_resolve_string(resolution_base, "../../") == "http://a/");
    print(url_resolve_string(resolution_base, "../../g") == "http://a/g");
    print(url_resolve_string(resolution_base, "../../../g") == "http://a/g");
    print(url_resolve_string(resolution_base, "../../../../g") == "http://a/g");
    print(url_resolve_string(resolution_base, "/./g") == "http://a/g");
    print(url_resolve_string(resolution_base, "/../g") == "http://a/g");
    print(url_resolve_string(resolution_base, "g.") == "http://a/b/c/g.");
    print(url_resolve_string(resolution_base, ".g") == "http://a/b/c/.g");
    print(url_resolve_string(resolution_base, "g..") == "http://a/b/c/g..");
    print(url_resolve_string(resolution_base, "..g") == "http://a/b/c/..g");
    print(url_resolve_string(resolution_base, "./../g") == "http://a/b/g");
    print(url_resolve_string(resolution_base, "g/./h") == "http://a/b/c/g/h");
    print(url_resolve_string(resolution_base, "g/../h") == "http://a/b/c/h");
    print(url_resolve_string(resolution_base, "g;x=1/./y") == "http://a/b/c/g;x=1/y");
    print(url_resolve_string(resolution_base, "g;x=1/../y") == "http://a/b/c/y");
    print(url_resolve_string(resolution_base, "g?y/./x") == "http://a/b/c/g?y/./x");
    print(url_resolve_string(resolution_base, "g?y/../x") == "http://a/b/c/g?y/../x");
    print(url_resolve_string(resolution_base, "g#s/./x") == "http://a/b/c/g#s/./x");
    print(url_resolve_string(resolution_base, "g#s/../x") == "http://a/b/c/g#s/../x");
    print(url_resolve_string(resolution_base, "http:g") == "http:g");
    print(url_resolve_string(resolution_base, "%2e/g") == "http://a/b/c/%2e/g");

    string large := url_encode("__LARGE_INPUT__");
    print(bytes.from_string(large).length());
    return 0;
}
